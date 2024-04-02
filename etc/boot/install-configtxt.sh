#!/bin/bash

cd "$(dirname "$0")"

CFGDIR="/boot$([[ -e /boot/firmware/config.txt ]] && echo /firmware)"
TESSIECONF="${CFGDIR}/tessie_config.txt"

[[ -e $TESSIECONF ]] && echo "Already configured. Abort." && exit 1

sudo cp tessie_config.txt $CFGDIR
sudo bash -c "echo -e '\ninclude tessie_config.txt' >> $CFGDIR/config.txt"
echo "Reboot for changes to take effect"
