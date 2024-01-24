# tessie

Etymology: tessie sounds better than TC (box), for temperature cycling (box)

## Hints for compilation and running locally on a coldbox

```shell
git clone git@github.com:ursl/tessie
(git clone https://github.com/ursl/tessie.git)
cd tessie/test1
qmake -o Makefile test1.pro
make

./tessie
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
