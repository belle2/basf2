#! /bin/bash
trap "echo signal; killall basf2; exit" INT TERM HUP
#ncore=`cat /proc/cpuinfo | grep "processor" | wc | awk '{print $1}'` / 2
ncore=8
basf2 --no-stats processor.py $1 $2 $3 $ncore &
#basf2 --no-stats roitest.py $1 $2 $3 $ncore &

while :;do
  sleep 2
done
