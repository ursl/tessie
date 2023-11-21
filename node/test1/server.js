const express = require('express');
const app = express();
const server = require('http').createServer(app);
const io = require('socket.io')(server);

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

io.on('connection', (socket) => {
    console.log('User connected');
    
    socket.on('disconnect', () => {
        console.log('User disconnected');
    });

    socket.on('valve0', (msg) => {
        if (valve0Status == 0) {
            valve0Status = 1;
        } else {
            valve0Status = 0;
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
