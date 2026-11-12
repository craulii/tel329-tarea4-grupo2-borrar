'use strict';
const { clientFromConnectionString } = require('azure-iot-device-mqtt');
const { Message } = require('azure-iot-device');

// la connection string del dispositivo va en una variable de entorno, no en el codigo (el repo es publico)
const connection_string = process.env.AZURE_DEVICE_CONN;
const sensor_url = process.env.SENSOR_URL || 'http://[fd00::202:2:2:2]/';
const device_id = 'contiki-ng-01';
const intervalo_ms = 3000;

if (!connection_string) {
    console.log('falta la variable AZURE_DEVICE_CONN');
    process.exit(1);
}

const client = clientFromConnectionString(connection_string);

function print_resultado(op) {
    return function (err, res) {
        if (err) console.log(op + ' error: ' + err.toString());
        if (res) console.log(op + ' status: ' + res.constructor.name);
    };
}

async function leer_sensor() {
    try {
        const res = await fetch(sensor_url);
        const body = await res.text();
        const obj = JSON.parse(body);
        console.log('sensor:', obj);

        const data = JSON.stringify({ deviceId: device_id, temperature: obj.temp, humidity: obj.hum });
        const message = new Message(data);
        message.contentType = 'application/json';
        message.contentEncoding = 'utf-8';
        message.properties.add('temperatureAlert', obj.temp > 30 ? 'true' : 'false');

        console.log('mandando mensaje: ' + message.getData());
        client.sendEvent(message, print_resultado('send'));
    } catch (err) {
        console.log('error leyendo el sensor: ' + err.message);
    }
}

client.open(function (err) {
    if (err) {
        console.log('no se pudo conectar: ' + err);
        return;
    }
    console.log('cliente conectado a azure iot hub');
    setInterval(leer_sensor, intervalo_ms);
});

console.log('middleware contiki-ng azure iniciado, sensor en ' + sensor_url);
