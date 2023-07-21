const http = require('http');
const fs = require('fs').promises;

const mqtt = require('mqtt');

const client = mqtt.connect('mqtt://coldbox01.psi.ch');

const hostname = '127.0.0.1';
const port = 3000;

mqttMessage = "";

client.subscribe('monTessie');
client.on('message', (topic, message) => {
    console.log(`Received message on topic ${topic}: ${message}`);
    mqttMessage = message;
    //  res.send(`Received message on topic ${topic}: ${message}`);
});



const requestListener = function (req, res) {
    // res.setHeader("Content-Type", "application/json");
    // res.writeHead(200);
    // res.end(`{"message": "This is a JSON response"}`);

    //fs.readFile(__dirname + "/index.html")

    fs.readFile(__dirname + "/index.html")
        .then(contents => {
            res.setHeader("Content-Type", "text/html");
            res.writeHead(200);
            res.end(contents);
        })
};

const server = http.createServer(requestListener);
server.listen(port, hostname, () => {
    console.log(`Server is running on http://${hostname}:${port}`);
});


// const server = http.createServer((req, res) => {
//     res.statusCode = 200;
//     res.setHeader('Content-Type', 'text/plain');
//     res.end(`${mqttMessage}`);
   
// });

server.listen(port, hostname, () => {
    console.log(`Server running at http://${hostname}:${port}/`);
});


