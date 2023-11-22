const express = require('express');
const app = express();
const server = require('http').createServer(app);
const io = require('socket.io')(server);
const mqtt = require('mqtt')

var valve0Status = 0;
var valve1Status = 0;

// ----------------------------------------------------------------------
// Usage:
// ------
// node server.js
// http://localhost:3000
// http://coldbox01.psi.ch:3000
// ----------------------------------------------------------------------

app.use(express.static('public'));

// mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "tec 6 cmd Power_Off "

const protocol = 'mqtt'
const host = 'coldbox01.psi.ch'
const port = '1883'
const clientId = `mqtt_${Math.random().toString(16).slice(3)}`

const connectUrl = `${protocol}://${host}:${port}`

const client = mqtt.connect(connectUrl, {
  clientId,
  clean: true,
  connectTimeout: 4000,
  reconnectPeriod: 1000
})

client.on('connect', () => {
  console.log('Connected')
})

const topic = 'ctrlTessie'

client.on('connect', () => {
  console.log('Connected')
  client.subscribe([topic], () => {
    console.log(`Subscribe to topic '${topic}'`)
  })
})

client.on('message', (topic, payload) => {
  console.log('Received Message:', topic, payload.toString())
})


io.on('connection', (socket) => {
    console.log('User connected');
    
    socket.on('disconnect', () => {
        console.log('User disconnected');
    });

    socket.on('valve0', (msg) => {
        if (valve0Status == 0) {
            valve0Status = 1;
            client.publish(topic, 'set valve0 on', {qos: 0, retain: false }, (error) => {
                if (error) {
                    console.error(error)
                }
            })
        } else {
            valve0Status = 0;
            client.publish(topic, 'set valve0 off', {qos: 0, retain: false }, (error) => {
                if (error) {
                    console.error(error)
                }
            })
        }
        console.log('valve0 clicked, valve0Status = ' + valve0Status);
        if (valve0Status == 0) {
            socket.emit('valve0white');
        } else {
            socket.emit('valve0blue');
        }
    });

    socket.on('valve1', (msg) => {
        if (valve1Status == 0) {
            valve1Status = 1;
        } else {
            valve1Status = 0;
        }
        console.log('valve1 clicked, valve1Status = ' + valve1Status);
        if (valve1Status == 0) {
            socket.emit('valve1white');
        } else {
            socket.emit('valve1blue');
        }
    });
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
});
