#!/bin/sh

################################################################################
#
#
# This script should be executed on belle-hltout.
#
# Must be executed by xinetd, do not execute by hand.
#
# Just proxy connection from eb2rx to HLT for the beamtest at DESY
#
# As HLT takes slightly long time to start,
# this script tries repeatedly the connection to HLT in 30 times.
#
#
#
################################################################################

cd /home/usr/b2daq/eb
host=127.0.0.1
port=4000
maxretry=30
retry=0
killall eb2tx-nc > /dev/null 2>&1
while sleep 1
do
logger -t "eb2tx:[$$]" spawn eb2tx-nc $host $port "($retry/$maxretry)"
./eb2tx-nc $host $port 2> /dev/null
if [ $? != 1 ] ;  then
	exit 0
fi
retry=$(( $retry + 1 ))
if [ $retry -gt $maxretry ]; then
    exit 0
fi
#logger -t "eb2tx:[$$]" calling eof checker
./iseof && exit 0
done
