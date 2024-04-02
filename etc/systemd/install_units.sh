#!/bin/sh

set -e
cd "$(dirname "$0")"

TESSIEDIR=$(readlink -f ../..)
UNITDIR="/etc/systemd/system/"

for svcstub in *.stub; do
	sed -re "s,%USER%,$USER,g" -e "s,%TESSIEDIR%,$TESSIEDIR,g" $svcstub > ${svcstub%.stub}
	sudo mv ${svcstub%.stub} $UNITDIR
done
sudo systemctl daemon-reload
sudo systemctl enable ${1:+--now} tessie-server
sudo systemctl enable ${1:+--now} tessie-web
