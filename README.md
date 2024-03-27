# tessie

Etymology: tessie sounds better than TC (box), for temperature cycling (box)

## Hints on required software
Installatation of various components
```shell
sudo apt install nodejs
sudo apt install npm

sudo apt-get install pigpio

sudo apt-get install libmosquitto-dev libmosquittopp-dev
sudo apt install -y mosquitto mosquitto-clients

sudo apt install libqt5charts5 libqt5charts5-dev

```

Add the following two lines to `/etc/mosquito/mosquito.conf`
```
listener 1883
allow_anonymous true
```

Enable various components at startup
```shell
sudo systemctl enable pigpiod
sudo systemctl enable mosquitto.service
```


## Hints for compilation and running locally on a coldbox

```shell
git clone git@github.com:ursl/tessie
(if you have issues with keys, use: git clone https://github.com/ursl/tessie.git)
cd tessie/test1
qmake -o Makefile  "CONFIG+=PI" test1.pro
make

./tessie
```


## Hints for installing/running hardware components

### Installation of the power button: 
Based on [how-to-add-a-power-button-to-your-raspberry-pi](https://howchoo.com/pi/how-to-add-a-power-button-to-your-raspberry-pi/)

The most important 2 lines for auto installation:
```
git clone https://github.com/Howchoo/pi-power-button.git
./pi-power-button/script/install
```
There is a backup ([powerbutton](https://github.com/ursl/tessie/tree/master/powerbutton)) of this contents within the tessie repository.

### Enable CAN bus and I2C
(sudo) Edit `/boot/config.txt` and add 
```
dtparam=spi=on
dtoverlay=mcp2515-can0,oscillator=12000000,interrupt=25
dtoverlay=spi-bcm2835-overlay
dtparam=i2c_vc=on
```
Afterwards, reboot for this to take effect.

The "script" [resetCAN.sh](https://github.com/ursl/tessie/tree/master/resetCAN.sh) must be run to configure the CAN bus to the proper bitrate (125kHz). If you enable the automatic startup of tessie (see below), this is done automatically before starting tessie. If you encounter many CAN bus errors, it might help to execute this script manually. 

### Adaption of the splash screen
(sudo) Edit `/boot/cmdline.txt` to contain
```
console=serial0,115200 console=tty1 root=PARTUUID=9cd7f13f-02 rootfstype=ext4 fsck.repair=yes rootwait quiet splash plymouth.ignore-serial-consoles logo.nologo vt.global_cursor_default=0
```
(sudo) Edit `/boot/config.txt` and add 
```
disable_splash=1
```
In a terminal do
```
cd /usr/share/plymouth/themes/pix/
sudo mv splash.png splash.png.bk
sudo cp /home/pi/tessie/splash.png ./
```


## Hints for operating tessie from a remote computer
In a shell on your computer `laptop`, do
```shell
laptop>ssh -Y "coldbox01" (or whatever hostname your Raspberry Pi has; assuming you have a login there)
coldbox01>cd tessie/test1
coldbox01>./tessie
```


In another window on your computer `laptop` run the mosquittto_pub commands, e.g.,
```shell
laptop>mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve0 on"
laptop>mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve1 on"
laptop>mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set ControlVoltage_Set 4.5"
laptop>mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "cmd Power_On"
```

See below for a help text on the MQTT/ctrlTessie commands.


If you want to see the response, you have to subscribe in another window, e.g., 
```shell
laptop>mosquitto_sub -h coldbox01 -t "ctrlTessie"
```

In another window on your computer `laptop` run the monitor, if desired
```shell
laptop>mosquitto_sub -h coldbox01 -t "monTessie"
```



## Hints for operating tessie with mosquitto
The following assumes that your `coldbox01` has `tessie` up and running, and that on your computer `laptop` has mosquitto installed
```shell
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve0 on"
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve1 on"
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set ControlVoltage_Set 4.5"
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "cmd Power_On"

mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "cmd Power_Off"
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set ControlVoltage_Set 0.0"

mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve0 off"
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve1 off"
```

## Help on MQTT/ctrlTessie commands
Issue `mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "help"` and then you will get the following printout in the window where you subscribed to "ctrlTessie" (see above).

```shell
===================
hostname: coldbox01
thread:  ctrlTessie
===================

Note: [tec {0|x}] can be before or after {get|set|cmd XXX}, e.g.
      cmd Power_On tec 7
      tec 7 cmd Power_Off

Note: tec numbering is from 1 .. 8. tec 0 refers to all TECs.

cmd messages:
-------------
cmd valve0
cmd valve1
cmd [tec {0|x}] Power_On
cmd [tec {0|x}] Power_Off
cmd [tec {0|x}] ClearError
cmd [tec {0|x}] GetSWVersion
cmd [tec {0|x}] SaveVariables
cmd [tec {0|x}] LoadVariables
cmd [tec {0|x}] Reboot

messages to write information:
------------------------------
[tec {0|x}] set Mode {0,1}
[tec {0|x}] set ControlVoltage_Set 1.1
[tec {0|x}] set PID_kp 1.1
[tec {0|x}] set PID_ki 1.1
[tec {0|x}] set PID_kd 1.1
[tec {0|x}] set Temp_Set 1.1
[tec {0|x}] set PID_Max 1.1
[tec {0|x}] set PID_Min 1.1
set valve0 {on|off}
set valve1 {on|off}

messages to obtain information:
-------------------------------
get Temp
get RH
get DP
get valve0
get valve1
get vprobe[1-8]

get [tec {0|x}] Mode
get [tec {0|x}] ControlVoltage_Set
get [tec {0|x}] PID_kp
get [tec {0|x}] PID_ki
get [tec {0|x}] PID_kd
get [tec {0|x}] Temp_Set
get [tec {0|x}] PID_Max
get [tec {0|x}] PID_Min
get [tec {0|x}] Temp_W
get [tec {0|x}] Temp_M
get [tec {0|x}] Temp_Diff
get [tec {0|x}] Peltier_U
get [tec {0|x}] Peltier_I
get [tec {0|x}] Peltier_R
get [tec {0|x}] Peltier_P
get [tec {0|x}] Supply_U
get [tec {0|x}] Supply_I
get [tec {0|x}] Supply_P
get [tec {0|x}] PowerState
get [tec {0|x}] Error
get [tec {0|x}] Ref_U

Tutorial for getting started:
-----------------------------
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve0 on" 
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve1 on" 
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set ControlVoltage_Set 4.5" 
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "cmd Power_On" 
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "cmd Power_Off" 
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set ControlVoltage_Set 0.0" 
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve0 off" 
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve1 off"
```

## Subscribing to monitoring information
```
mosquitto_sub -h coldbox01 -t "monTessie"
```
Only values (changes) outside of a window a published

## Configure node for the webserver
```
cd tessie/node/test1
npm install --save express socket.io mqtt
```

## How to setup the automatic startup of tessie and the webserver
Using `systemctl` create the following two files (`sudo`!):

```
pi@coldbox01:~ $ cat /lib/systemd/system/tessie.service
[Unit]
Description=tessie
#After=multi-user.target
After=network.target

[Service]
Type=idle
Environment="XAUTHORITY=/home/pi/.Xauthority"
Environment="DISPLAY=:0"
WorkingDirectory=/home/pi/tessie/test1
ExecStartPre=/home/pi/tessie/resetCAN.sh
ExecStart=/home/pi/tessie/test1/tessie 
StandardOutput=inherit
StandardError=inherit

[Install]
#WantedBy=multi-user.target
WantedBy=graphical.target
```

```
pi@coldbox01:~ $ cat /lib/systemd/system/tessieWeb.service
[Unit]
Description=tessie
After=multi-user.target

[Service]
Type=idle
WorkingDirectory=/home/pi/tessie/node/test1
ExecStart=/usr/bin/node /home/pi/tessie/node/test1/server3.js 
 
[Install]
WantedBy=multi-user.target
```

Turn these two services on with 
```
sudo systemctl enable tessie.service
sudo systemctl enable tessieWeb.service
```

Reboot. Control their status with 
```
systemctl status tessie
systemctl status tessieWeb
```

With this setup, you can connect to http://coldbox01:3000 (Note: http, not https!) For more information on the webserver, see https://github.com/ursl/tessie/tree/master/node/test1#readme

<img width="1250" alt="240201-tessie-web" src="https://github.com/ursl/tessie/assets/5073648/931cdd31-1165-4ca0-88da-b44ae3c5af6e">

## Configure tessie to serve port 80 instead of 3000
The purpose of this section is to provide hints how to setup `nginx` such that you can connect to `http://coldbox01` instead of `http://coldbox01:3000`. 
Follow the instructions [here](https://medium.com/@adarsh-d/node-js-on-port-80-or-443-7083336af3b0). 

The following nginx config file works at PSI
```
server {
    listen 80;
    server_name coldbox01.psi.ch;

    location / {
        proxy_pass http://localhost:3000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_cache_bypass $http_upgrade;
    }
}
```

Start/test/restart with `service`:
```
sudo service nginx start
sudo service nginx status
sudo service nginx restart
```

Depending on your system setup, you might need 
```
pi@coldbox01:~ $ cat /lib/systemd/system/nginx.service 
# Stop dance for nginx
# =======================
#
# ExecStop sends SIGQUIT (graceful stop) to the nginx process.
# If, after 5s (--retry QUIT/5) nginx is still running, systemd takes control
# and sends SIGTERM (fast shutdown) to the main process.
# After another 5s (TimeoutStopSec=5), and if nginx is alive, systemd sends
# SIGKILL to all the remaining processes in the process group (KillMode=mixed).
#
# nginx signals reference doc:
# http://nginx.org/en/docs/control.html
#
[Unit]
Description=A high performance web server and a reverse proxy server
Documentation=man:nginx(8)
After=network-online.target remote-fs.target nss-lookup.target
Wants=network-online.target

[Service]
Type=forking
PIDFile=/run/nginx.pid
ExecStartPre=/usr/sbin/nginx -t -q -g 'daemon on; master_process on;'
ExecStart=/usr/sbin/nginx -g 'daemon on; master_process on;'
ExecReload=/usr/sbin/nginx -g 'daemon on; master_process on;' -s reload
ExecStop=-/sbin/start-stop-daemon --quiet --stop --retry QUIT/5 --pidfile /run/nginx.pid
TimeoutStopSec=5
KillMode=mixed

[Install]
WantedBy=multi-user.target
``
