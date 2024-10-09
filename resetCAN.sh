#!/bin/bash
sudo ip link set can0 down; sudo ip link set can0 type can bitrate 125000 loopback off; sudo ip link set can0 up
#echo "sleep for 10 seconds"
#sleep 10
