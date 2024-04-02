#!/bin/bash

set -e
cd "$(dirname "$0")"

banner(){
	echo -e "\n##########################################"
	echo  "$*"
	echo -e "\n##########################################"
}

[[ $UID -eq 0 ]] && echo "Don't run this script as sudo. ABORT" && exit 2 

echo -e "!!!!WARNING!!!!\nRun this on a fresh system only. You have 10s to stop by pressing CTRL+C"
sleep 10


banner "Installing packages"
sudo sh -c "apt-get update && xargs -a packages.list apt-get -y install"
cd ../node/test1; npm install --save express socket.io mqtt; cd -

banner "Setting up boot parameters"
boot/install-configtxt.sh
boot/fix_splash_screen.sh

banner "Configuring can interface"
network/install_can-config.sh

banner "Installing power button handling"
powerbutton/install

banner "Configuring MQTT server"
mosquitto/install-mqtt.sh

banner "Activating system services"
sudo systemctl enable --now mosquitto.service
sudo systemctl enable --now pigpiod

if [ -e ../test1/tessie ]; then
   systemd/install_units.sh now
else
   systemd/install_units.sh
   echo -e "\n!!! Please compile tessi before rebooting\n"
fi

banner "Done. Reboot system to activate changes."
