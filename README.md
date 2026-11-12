# TEL329 Tarea 4 - Grupo 2

Jose Vega, Sebastian Venegas, Ignacio Cubillos

Copiar esta carpeta en `contiki-ng/examples/tarea4`.

- `act1-coap`: raiz RPL Lite + servidor CoAP con el recurso `/temperatura`, y cliente CoAP que manda POST cada 10 s
- `act2-mqtt`: cliente MQTT que publica en `usm/tel329/temperatura` (QoS 1) hacia mosquitto en `fd00::1`, usar con `examples/rpl-border-router`
- `act3-captura`: script para sacar los tiempos de respuesta desde el csv exportado de wireshark, y los csv de las capturas (`captura_coap.csv`, `captura_mqtt.csv`)
- `act4-azure/websense-cloud`: sensor contiki-ng con servidor web que responde un json con temperatura y humedad
- `act4-azure/middleware`: middleware que consulta al sensor y manda a azure iot hub, y el suscriptor que lee los mensajes
- `sims`: simulaciones de Cooja (`act1.csc`, `act2.csc`, `act4.csc`, UDGM 50 m / 100 m). En `sims/logs` quedan los logs de cada corrida (mote output, mosquitto, tunslip6, middleware, suscriptor, monitor-events), la salida del script y `tiempos.csv`. Los pcap no se suben
- `evidencias`: capturas de pantalla que van en el notebook

Compilar: `make TARGET=cooja` dentro de cada carpeta (o agregarlo como mote en Cooja).

Las simulaciones se abren desde `contiki-ng/tools/cooja` (el script de cada .csc guarda el mote output en `sims/logs/<act>/mote_output.txt` con ruta relativa a esa carpeta):

```
cd contiki-ng/tools/cooja
./gradlew run --args="--gui --autostart --contiki=$(realpath ../..) $(realpath ../../examples/tarea4/sims/act1.csc)"
```

Middleware (Node 22 o mas nuevo, lo piden las librerias actuales de azure):

```
cd act4-azure/middleware
npm install
export AZURE_DEVICE_CONN="HostName=...;DeviceId=contiki-ng-01;SharedAccessKey=..."
npm run middleware
export AZURE_EVENTHUB_CONN="Endpoint=sb://...;SharedAccessKeyName=...;SharedAccessKey=...;EntityPath=..."
npm run suscriptor
```

Basado en Kurniawan (2018), Practical Contiki-NG, adaptado a Contiki-NG actual.
