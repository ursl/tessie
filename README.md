# tessie

Etymology: tessie sounds better than TC (box), for temperature cycling (box)

## Documentation and installation
>[!IMPORTANT]
>Please start with the [user guide](https://github.com/ursl/tessie/blob/master/main.pdf).
>The installation procedure for `tessie` is described in (gory) detail.

You could also look at [Bane's scripts](https://github.com/BranislavRistic/tessie/tree/dev) for quasi-automatic installation (it may or may not work out of the box, depending which OS version you start with).



## Hints for operating tessie (and the coldbox) from a remote computer
Remote operation of `tessie` (and hence the coldbox) is done exclusively through MQTT. 

The following assumes that your `coldbox01` has `tessie` up and running, and that your computer `laptop` has mosquitto installed.

In a window on your computer `laptop` run the mosquittto_pub commands, e.g.,
```shell
laptop>mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve0 on"
laptop>mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve1 on"
laptop>mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set ControlVoltage_Set 4.5"
laptop>mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "cmd Power_On"

mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "cmd Power_Off"
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set ControlVoltage_Set 0.0"

mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve0 off"
mosquitto_pub -h coldbox01 -t "ctrlTessie" -m "set valve1 off"
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
