#!/bin/sh

cd "$(dirname "$0")"

sudo cp interfaces.d/can0 /etc/network/interfaces.d
sudo ifup can0 || echo "!!! Interface can0 not found. It should show up after reboot and if the coldbox RPi hat is installed"
