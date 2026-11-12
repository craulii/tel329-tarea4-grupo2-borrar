#include "contiki.h"
#include "net/routing/routing.h"
#include "mqtt.h"
#include "random.h"
#include "sys/node-id.h"
#include <stdio.h>
#include <string.h>

#define BROKER_IP "fd00::1"
#define BROKER_PORT 1883
#define TOPICO "usm/tel329/temperatura"
#define INTERVALO (10 * CLOCK_SECOND)

static struct mqtt_connection conn;
static char client_id[24];
static char msg[48];
static uint16_t mid_pub;
static clock_time_t t_envio;
static int enviados = 0;
static int confirmados = 0;
static int conectado = 0;
static int conectando = 0;

static void
evento_mqtt(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  unsigned long rtt;

  switch(event) {
  case MQTT_EVENT_CONNECTED:
    printf("conectado al broker\n");
    conectado = 1;
    conectando = 0;
    break;
  case MQTT_EVENT_DISCONNECTED:
    printf("desconectado del broker\n");
    conectado = 0;
    conectando = 0;
    break;
  case MQTT_EVENT_PUBACK:
    confirmados++;
    rtt = (clock_time() - t_envio) * 1000 / CLOCK_SECOND;
    printf("puback mid=%u rtt=%lu ms (enviados %d, confirmados %d)\n",
           *((uint16_t *)data), rtt, enviados, confirmados);
    break;
  default:
    printf("evento mqtt %d\n", event);
    break;
  }
}

PROCESS(mqtt_cliente_process, "cliente mqtt");
AUTOSTART_PROCESSES(&mqtt_cliente_process);

PROCESS_THREAD(mqtt_cliente_process, ev, data)
{
  static struct etimer et;
  static int temp;

  PROCESS_BEGIN();

  snprintf(client_id, sizeof(client_id), "tel329-nodo-%u", node_id);
  mqtt_register(&conn, &mqtt_cliente_process, client_id, evento_mqtt, 64);

  etimer_set(&et, INTERVALO + random_rand() % INTERVALO);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    if(!NETSTACK_ROUTING.node_is_reachable()) {
      printf("todavia no estoy en la red\n");
    } else if(!conectado) {
      if(conectando == 0 || conectando > 3) {
        printf("conectando a [%s]:%u como %s\n", BROKER_IP, BROKER_PORT, client_id);
        mqtt_connect(&conn, BROKER_IP, BROKER_PORT, 60, MQTT_CLEAN_SESSION_ON);
        conectando = 1;
      } else {
        conectando++;
      }
    } else if(mqtt_ready(&conn) && conn.out_buffer_sent) {
      temp = 15 + random_rand() % 20;
      snprintf(msg, sizeof(msg), "{\"id\":%u,\"temp\":%d}", node_id, temp);
      enviados++;
      t_envio = clock_time();
      mqtt_publish(&conn, &mid_pub, TOPICO, (uint8_t *)msg, strlen(msg),
                   MQTT_QOS_LEVEL_1, MQTT_RETAIN_OFF);
      printf("publico en %s: %s (mid %u)\n", TOPICO, msg, mid_pub);
    } else {
      printf("mqtt ocupado, espero\n");
    }

    etimer_set(&et, INTERVALO);
  }

  PROCESS_END();
}
