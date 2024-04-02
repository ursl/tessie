#!/bin/bash

cd "$(dirname "$0")"

TESSIEDIR="$PWD/test1"
TESSIENODEDIR="$PWD/node/test1"
TESSIENODEBIN="server3.js"
PIDFILE="$PWD/.current.pid"

handlePidFile(){
   if [ -r $PIDFILE ]; then
      if pgrep -F $PIDFILE >/dev/null; then
            echo "TESSIE is already running. ABORT"
            exit 1
      else
            echo "Removing stale pid file"
            rm $PIDFILE
      fi
   fi
   echo $$ > $PIDFILE
}

case ${1}x in
  "servicex")
    handlePidFile
    #./resetCAN.sh
    cd ${TESSIEDIR}
    exec ${TESSIEDIR}/tessie
    ;;
  "nodex")
    cd ${TESSIENODEDIR}
    exec /usr/bin/node ${TESSIENODEBIN}
    ;;
  *)
    echo "USAGE: $0 [service|node]"
    exit 1
esac

