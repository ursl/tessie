#!/bin/sh

cd "$(dirname "$0")"

sudo cp interfaces.d/can0 /etc/network/interfaces.d
sudo ifup can0
