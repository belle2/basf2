#!/bin/sh
drop-all
sleep 1
ssh -n -n -XY -lrun eclpc11 ~run/shaper/firmware_download_daqgr.sh

#echo "Press return key"
#read something
shslb-all
sleep 1
stat-hsl
sleep 1
check-all
sleep 1
cini-all
sleep 1
