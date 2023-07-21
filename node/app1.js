const express = require('express');
const app = express();

var client = null;

app.get('/subscribe', (req, res) => {
    // send headers to keep connection alive
    const headers = {
        'Content-Type': 'text/event-stream',
        'Connection': 'keep-alive',
        'Cache-Control': 'no-cache'
    };
    res.writeHead(200, headers);
    
    // send client a simple response
    res.write('you are subscribed');
    
    // store `res` of client to let us send events at will
    client = res;
    
    // listen for client 'close' requests
    req.on('close', () => { client = null; })
});

// send refresh event (must start with 'data: ')
function sendRefresh() {
    client.write('data: refresh');
}
