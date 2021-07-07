#!/bin/bash
set +e
source /root/setup_basf2 "" > /dev/null 2> /dev/null;

nsmd2 -b -s 9120 -p 9120 -h `hostname` -l /nsm -o > /dev/null &

if [[ "$(hostname)" == "erctl" ]]; then
    nsmd2 -b -s 9020 -p 9020 -h `hostname` -l /nsm -o > /dev/null &
fi

sleep 10

sleep infinity
