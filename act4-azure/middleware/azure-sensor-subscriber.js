'use strict';
const { EventHubConsumerClient, latestEventPosition } = require('@azure/event-hubs');

// connection string del "Event Hub-compatible endpoint" (IoT Hub > Built-in endpoints)
const connection_string = process.env.AZURE_EVENTHUB_CONN;
const consumer_group = '$Default';

if (!connection_string) {
    console.log('falta la variable AZURE_EVENTHUB_CONN');
    process.exit(1);
}

const client = new EventHubConsumerClient(consumer_group, connection_string);

client.subscribe({
    processEvents: async (events, context) => {
        for (const event of events) {
            let cuerpo = event.body;
            if (Buffer.isBuffer(cuerpo)) {
                cuerpo = cuerpo.toString();
            }
            if (typeof cuerpo !== 'string') {
                cuerpo = JSON.stringify(cuerpo);
            }
            console.log('Message received (particion ' + context.partitionId + '):');
            console.log(cuerpo);
            console.log('propiedades: ' + JSON.stringify(event.properties));
            console.log('');
        }
    },
    processError: async (err) => {
        console.log(err.message);
    }
}, { startPosition: latestEventPosition });

console.log('suscriptor iniciado, esperando mensajes de azure iot hub...');
