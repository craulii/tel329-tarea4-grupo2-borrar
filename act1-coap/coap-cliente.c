#include "contiki.h"
#include "net/routing/routing.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "random.h"
#include "sys/node-id.h"
#include <stdio.h>
#include <string.h>

#define INTERVALO (10 * CLOCK_SECOND)

static clock_time_t t_envio;
static int enviados = 0;
static int respuestas = 0;

static void
manejar_respuesta(coap_message_t *response)
{
  unsigned long rtt;

  if(response == NULL) {
    printf("timeout, no llego respuesta\n");
    return;
  }

  respuestas++;
  rtt = (clock_time() - t_envio) * 1000 / CLOCK_SECOND;
  printf("respuesta %u.%02u rtt=%lu ms (enviados %d, respuestas %d)\n",
         response->code >> 5, response->code & 0x1f, rtt, enviados, respuestas);
}

PROCESS(coap_cliente_process, "cliente coap");
AUTOSTART_PROCESSES(&coap_cliente_process);

PROCESS_THREAD(coap_cliente_process, ev, data)
{
  static struct etimer et;
  static coap_endpoint_t raiz_ep;
  static coap_message_t request[1];
  static char msg[32];
  static int temp;

  PROCESS_BEGIN();

  etimer_set(&et, INTERVALO + random_rand() % INTERVALO);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    if(NETSTACK_ROUTING.node_is_reachable() &&
       NETSTACK_ROUTING.get_root_ipaddr(&raiz_ep.ipaddr)) {

      raiz_ep.port = UIP_HTONS(COAP_DEFAULT_PORT);
      raiz_ep.secure = 0;

      temp = 15 + random_rand() % 20;
      snprintf(msg, sizeof(msg), "id=%u temp=%d", node_id, temp);

      coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
      coap_set_header_uri_path(request, "temperatura");
      coap_set_payload(request, (uint8_t *)msg, strlen(msg));

      printf("mando POST /temperatura %s\n", msg);
      enviados++;
      t_envio = clock_time();
      COAP_BLOCKING_REQUEST(&raiz_ep, request, manejar_respuesta);
    } else {
      printf("todavia no estoy en la red\n");
    }

    etimer_set(&et, INTERVALO);
  }

  PROCESS_END();
}
