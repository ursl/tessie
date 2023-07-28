const fs = require('fs').promises;
const express = require('express');
const app = express();
const port = 80

const mqtt = require('mqtt');
const client = mqtt.connect('mqtt://coldbox01.psi.ch');

mqttMessage = "";

client.subscribe('monTessie');
client.on('message', (topic, message) => {
    console.log(`Received message on topic ${topic}: ${message}`);
    mqttMessage = message;
    //  res.send(`Received message on topic ${topic}: ${message}`);
});



app.get('/data', (req, res) => {
    console.log(`app1 received data request`)
//    mqttMessageJson = `\{\"MQTT\" : \"title\": \"Titel\", \"data\": \{${mqttMessage}\}\}`;
//    data=> res.json(mqttMessageJson);
})


app.listen(port, () => {
  console.log(`app1 listening on port ${port}`)
})


app.get('/tessie/', (req, res) => {
    //    res.send("tessie!");
    fs.readFile(__dirname + "/index2.html")
        .then(contents => {
            res.setHeader("Content-Type", "text/html");
            res.writeHead(200);
            res.end(contents);
        })
})


