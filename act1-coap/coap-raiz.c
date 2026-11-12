#include "contiki.h"
#include "net/routing/routing.h"
#include "coap-engine.h"
#include <stdio.h>
#include <string.h>

static int total_recibidos = 0;

static void res_post_handler(coap_message_t *request, coap_message_t *response,
                             uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_temperatura, "title=\"temperatura\";rt=\"Text\"", NULL, res_post_handler, NULL, NULL);

static void
res_post_handler(coap_message_t *request, coap_message_t *response,
                 uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *payload;
  char texto[32];
  int largo;
  unsigned int id;
  int temp;

  largo = coap_get_payload(request, &payload);
  if(largo >= sizeof(texto)) {
    largo = sizeof(texto) - 1;
  }
  memcpy(texto, payload, largo);
  texto[largo] = '\0';

  if(sscanf(texto, "id=%u temp=%d", &id, &temp) != 2) {
    printf("payload raro: %s\n", texto);
    coap_set_status_code(response, BAD_REQUEST_4_00);
    return;
  }

  total_recibidos++;
  printf("coap temperatura nodo %u: %d C (total %d)\n", id, temp, total_recibidos);
  coap_set_status_code(response, CHANGED_2_04);
}

PROCESS(coap_raiz_process, "raiz coap");
AUTOSTART_PROCESSES(&coap_raiz_process);

PROCESS_THREAD(coap_raiz_process, ev, data)
{
  PROCESS_BEGIN();

  NETSTACK_ROUTING.root_start();
  coap_activate_resource(&res_temperatura, "temperatura");
  printf("raiz coap lista, recurso /temperatura\n");

  PROCESS_END();
}
