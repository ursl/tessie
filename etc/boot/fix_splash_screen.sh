#!/bin/sh

set -e
cd "$(dirname "$0")"

test -e  .fixed  && echo "Script already run. Doing nothing" && exit 1
touch .fixed

sudo apt -y install rpd-plym-splash
sudo sed -i -e '1s/$/ quiet splash plymouth.ignore-serial-consoles logo.nologo vt.global_cursor_default=0/' /boot/firmware/cmdline.txt
sudo sh -c 'echo "disable_splash=1" >> /boot/firmware/tessi_config.txt'

PIXDIR='/usr/share/plymouth/themes/pix'
sudo mv $PIXDIR/splash.png $PIXDIR/splash.png.bk
sudo cp splash.png $PIXDIR
