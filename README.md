# tessie

Etymology: tessie sounds better than TC (box), for temperature cycling (box)

## hints for compilation and running locally on a coldbox

```shell
git clone git@github.com:ursl/tessie
(git clone https://github.com/ursl/tessie.git)
cd tessie/test1
qmake -o Makefile test1.pro
make

./tessie
```

## hints for operating tessie from a remote computer
In a shell on your computer `laptop`, do
```shell
laptop>ssh -Y "coldbox" (or whatever hostname your Raspberry Pi has; assuming you have a login there)
coldbox>cd tessie/test1
coldbox>./tessie
```


In another window on your computer `laptop` run the mosquittto_pub commands, e.g.,
```shell
laptop>mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve0 on"
laptop>mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve1 on"
laptop>mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set ControlVoltage_Set 4.5"
laptop>mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "cmd Power_On"
```

See below for a help text on the ctrlTessie commands.

In another window on your computer `laptop` run the monitor, if desired
```shell
laptop>mosquitto_sub -h coldbox01 -t "monTessie"
```



## hints for operating tessie with mosquitto
The following assumes that your `coldbox01` has `tessie` up and running.
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

## help on MQTT commands
```shell
===================
hostname: coldbox01
thread:  ctrlTessie
===================
 
Note: [tec {0|x}] can be before or after {get|set|cmd}

cmd messages:
-------------
cmd Power_On
cmd Power_Off
cmd valve0
cmd valve1
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
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m " set valve0 on" 
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
