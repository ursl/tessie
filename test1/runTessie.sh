#!/bin/bash
sudo ip link set can0 down; sudo ip link set can0 type can bitrate 125000; sudo ip link set can0 up
./tessie &
cd ../node/test1 && node server3.js &
