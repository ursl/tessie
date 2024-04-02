#!/bin/bash

cd "$(dirname "$0")"

if [ $UID -ne 0 ]; then
   echo "Rerunning as root"
   exec sudo $0 $@
fi

apt -y install mosquitto

cp ./tessi.conf /etc/mosquitto/conf.d
systemctl stop mosquitto
systemctl enable --now mosquitto
