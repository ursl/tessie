const express = require('express');
const app = express();
const server = require('http').createServer(app);
const io = require('socket.io')(server);
const mqtt = require('mqtt')
const os = require("os");
const fs = require('fs');
const path = require('path');

var valve0Status = 0;
var valve1Status = 0;

var checktec1Status = 0;

// ----------------------------------------------------------------------
// Usage:
// ------
// node server3.js
// http://localhost:3000
// http://coldbox01.psi.ch:3000
// ----------------------------------------------------------------------

app.use(express.static('public'));

// mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "tec 6 cmd Power_Off "

const protocol = 'mqtt'
//const host = os.hostname();
const host = 'localhost';
const port = '1883'
const clientId = `mqtt_${Math.random().toString(16).slice(3)}`

const connectUrl = `${protocol}://${host}:${port}`

let versionString = ''
let envString = ''
let PowerStateString = ''
let ControlVoltage_SetString = ''
let Temp_MString = ''
let PID_kpString = ''
let PID_kiString = ''
let PID_kdString = ''
let Temp_SetString = ''
let PID_MaxString = ''
let PID_MinString = ''
let Temp_DiffString = ''
let Peltier_UString = ''
let Peltier_IString = ''
let Peltier_RString = ''
let Peltier_PString = ''
let Supply_UString = ''
let Supply_IString = ''
let Supply_PString = ''
let ErrorString = ''
let Ref_UString = ''
let ModeString = ''
let WarningString = ''
let AlarmString = ''

// -- read version string
readVersion();


// ----------------------------------------------------------------------
// -- MQTT
// ----------------------------------------------------------------------
const clientMqtt = mqtt.connect(connectUrl, {
    clientId,
    clean: true,
    connectTimeout: 4000,
    reconnectPeriod: 1000
})

clientMqtt.on('connect', () => {
  console.log('Connected')
})

// -- ctrlTessie
const topCtrl = 'ctrlTessie';
clientMqtt.on('connect', () => {
  console.log('Connected')
  clientMqtt.subscribe([topCtrl], () => { console.log(`Subscribe to topCtrl '${topCtrl}'`)  })
})

clientMqtt.on('message', (topCtrl, payload) => {
//  console.log('Received Message:', topCtrl, payload.toString())
})

// -- monTessie
const topMon  = 'monTessie';
clientMqtt.on('connect', () => {
  console.log('Connected')
  clientMqtt.subscribe([topMon], () => { console.log(`Subscribe to topMon '${topMon}'`)  })
})

clientMqtt.on('message', (topMon, payload) => {
    // -- reset strings
    AlarmString = '';
    WarningString = '';

    if (payload.includes('Env = ')) {
        envString = payload.toString();
    }
    if (payload.includes('PowerState = ')) {
        PowerStateString = payload.toString();
    }
    if (payload.includes('ControlVoltage_Set = ')) {
        ControlVoltage_SetString = payload.toString();
    }
    if (payload.includes('Temp_M = ')) {
        Temp_MString = payload.toString();
    }
    if (payload.includes('PID_kp = ')) {
        PID_kpString = payload.toString();
    }
    if (payload.includes('PID_ki = ')) {
        PID_kiString = payload.toString();
    }
    if (payload.includes('PID_kd = ')) {
        PID_kdString = payload.toString();
    }
    if (payload.includes('Temp_Set = ')) {
        Temp_SetString = payload.toString();
    }
    if (payload.includes('PID_Max = ')) {
        PID_MaxString = payload.toString();
    }
    if (payload.includes('PID_Min = ')) {
        PID_MinString = payload.toString();
    }
    if (payload.includes('Temp_Diff = ')) {
        Temp_DiffString = payload.toString();
    }
    if (payload.includes('Peltier_U = ')) {
        Peltier_UString = payload.toString();
    }
    if (payload.includes('Peltier_I = ')) {
        Peltier_IString = payload.toString();
    }
    if (payload.includes('Peltier_R = ')) {
        Peltier_RString = payload.toString();
    }
    if (payload.includes('Peltier_P = ')) {
        Peltier_PString = payload.toString();
    }
    if (payload.includes('Supply_U = ')) {
        Supply_UString = payload.toString();
    }
    if (payload.includes('Supply_I = ')) {
        Supply_IString = payload.toString();
    }
    if (payload.includes('Supply_P = ')) {
        Supply_PString = payload.toString();
    }
    if (payload.includes('Error = ')) {
        ErrorString = payload.toString();
    }
    if (payload.includes('Ref_U = ')) {
        Ref_UString = payload.toString();
    }
    if (payload.includes('Mode = ')) {
        ModeString = payload.toString();
    }
    if (payload.includes('==WARNING== ')) {
        WarningString = payload.toString();
    }
    if (payload.includes('==ALARM== ')) {
        AlarmString = payload.toString();
    }
})


// ----------------------------------------------------------------------
// -- socket.io
// ----------------------------------------------------------------------
io.on('connection', (socket) => {
    console.log('User connected');
    console.log('versionString ->' + versionString + '<-');

    socket.on('disconnect', () => {
        console.log('User disconnected');
    });


    setInterval(() => {
        socket.emit('envString', envString);
        socket.emit('PowerStateString', PowerStateString);
        socket.emit('ControlVoltage_SetString', ControlVoltage_SetString);
        socket.emit('Temp_MString', Temp_MString);
        socket.emit('PID_kpString', PID_kpString);
        socket.emit('PID_kiString', PID_kiString);
        socket.emit('PID_kdString', PID_kdString);
        socket.emit('Temp_SetString', Temp_SetString);
        socket.emit('PID_MaxString', PID_MaxString);
        socket.emit('PID_MinString', PID_MinString);
        socket.emit('Temp_DiffString', Temp_DiffString);
        socket.emit('Peltier_UString', Peltier_UString);
        socket.emit('Peltier_IString', Peltier_IString);
        socket.emit('Peltier_RString', Peltier_RString);
        socket.emit('Peltier_PString', Peltier_PString);
        socket.emit('Supply_UString', Supply_UString);
        socket.emit('Supply_IString', Supply_IString);
        socket.emit('Supply_PString', Supply_PString);
        socket.emit('ErrorString', ErrorString);
        socket.emit('Ref_UString', Ref_UString);
        socket.emit('ModeString', ModeString);
        if (WarningString.length != 0) {
            socket.emit('WarningString', WarningString);
        }
        if (AlarmString.length != 0) {
            socket.emit('AlarmString', AlarmString);
        }
        if (valve0Status == 0) {
            socket.emit('valve0white');
        } else {
            socket.emit('valve0blue');
        }
        if (valve1Status == 0) {
            socket.emit('valve1white');
        } else {
            socket.emit('valve1blue');
        }

    }, 1000);

    socket.on('valve0', (msg) => {
        if (valve0Status == 0) {
            valve0Status = 1;
            clientMqtt.publish(topCtrl, 'set valve0 on', {qos: 0, retain: false }, (error) => {
                if (error) {
                    console.error(error)
                }
            })
        } else {
            valve0Status = 0;
            clientMqtt.publish(topCtrl, 'set valve0 off', {qos: 0, retain: false }, (error) => {
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
            clientMqtt.publish(topCtrl, 'set valve1 on', {qos: 0, retain: false }, (error) => {
                if (error) {
                    console.error(error)
                }
            })
        } else {
            valve1Status = 0;
            clientMqtt.publish(topCtrl, 'set valve1 off', {qos: 0, retain: false }, (error) => {
                if (error) {
                    console.error(error)
                }
            })
        }
        console.log('valve1 clicked, valve1Status = ' + valve1Status);
        if (valve1Status == 0) {
            socket.emit('valve1white');
        } else {
            socket.emit('valve1blue');
        }
    });

    socket.on('getcsv', (msg) => {
        console.log('getcsv input received ->' + msg + '<-');
        var filePath = path.join('/home/pi/tessie/test1', 'shorttessie.csv');
        var stat = fs.statSync(filePath);
        socket.emit('putcsv', filePath, (status) => {
            console.log(status);
        });
    });

    socket.on('controlvoltage_set', (msg) => {
        console.log('controlvoltage_set input received ->' + msg + '<-');
        clientMqtt.publish(topCtrl, msg, {qos: 0, retain: false }, (error) => {
            if (error) {
                console.error(error)
            }
        })
    });

    socket.on('temp_set', (msg) => {
        console.log('temp_set input received ->' + msg + '<-');
        clientMqtt.publish(topCtrl, msg, {qos: 0, retain: false }, (error) => {
            if (error) {
                console.error(error)
            }
        })
    });

    socket.on('mode_set', (msg) => {
        console.log('mode_set input received ->' + msg + '<-');
        clientMqtt.publish(topCtrl, msg, {qos: 0, retain: false }, (error) => {
            if (error) {
                console.error(error)
            }
        })
    });

    socket.on('pid_kp', (msg) => {
        console.log('pid_kp input received ->' + msg + '<-');
        clientMqtt.publish(topCtrl, msg, {qos: 0, retain: false }, (error) => {
            if (error) {
                console.error(error)
            }
        })
    });

    socket.on('pid_ki', (msg) => {
        console.log('pid_ki input received ->' + msg + '<-');
        clientMqtt.publish(topCtrl, msg, {qos: 0, retain: false }, (error) => {
            if (error) {
                console.error(error)
            }
        })
    });

    socket.on('pid_kd', (msg) => {
        console.log('pid_kd input received ->' + msg + '<-');
        clientMqtt.publish(topCtrl, msg, {qos: 0, retain: false }, (error) => {
            if (error) {
                console.error(error)
            }
        })
    });

    socket.on('checktec', (msg) => {
        console.log('checktec input received ->' + msg + '<-');
        clientMqtt.publish(topCtrl, msg, {qos: 0, retain: false }, (error) => {
            if (error) {
                console.error(error)
            }
        })
    });

    socket.on('getversionstring', (msg) => {
        console.log('getversionstring input received ->' + msg + '<-');
        socket.emit('versionString', versionString);
    });

});



const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
});

// ----------------------------------------------------------------------
function readVersion() {
    let filePath = "../../test1/version.txt";
    versionString = fs.readFileSync(filePath).toString().replace('\n', '');
}
