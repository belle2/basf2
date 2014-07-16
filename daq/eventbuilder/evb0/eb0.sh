#!/bin/sh
#Thu Oct 17 14:17:13 CEST 2013
cd /home/usr/yamadas/basf2/release/daq/eventbuilder/evb0
killall eb0 > /dev/null 2>&1
sleep 1
./eb0 cpr001:33001 2> /home/usr/yamadas/log.txt

#EOT                    
