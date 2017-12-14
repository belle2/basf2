#!/bin/sh
#Thu Oct 17 14:17:13 CEST 2013
cd /home/g0cdc/belle2/releases/v20140711/daq/eventbuilder/evb0
killall eb0 > /dev/null 2>&1
sleep 1
./eb0 cpr2042:33000 cpr2043:33000 

#EOT                    
