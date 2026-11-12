#include "contiki.h"
#include "contiki-net.h"
#include "httpd-simple.h"
#include "random.h"
#include <stdio.h>

static int consultas = 0;

static
PT_THREAD(generate_routes(struct httpd_state *s))
{
  static char buff[35];
  int temperatura;
  int humedad;

  PSOCK_BEGIN(&s->sout);

  temperatura = 15 + random_rand() % 25;
  humedad = 80 + random_rand() % 10;
  consultas++;

  sprintf(buff, "{\"temp\":%d,\"hum\":%d}", temperatura, humedad);
  printf("consulta %d, mando json: %s\n", consultas, buff);
  SEND_STRING(&s->sout, buff);

  PSOCK_END(&s->sout);
}

httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return generate_routes;
}

PROCESS(websense_cloud_process, "websense cloud");
AUTOSTART_PROCESSES(&websense_cloud_process);

PROCESS_THREAD(websense_cloud_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();
  printf("servidor web del sensor escuchando en el puerto 80\n");

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}
