#!/bin/sh
#Thu Oct 17 14:17:13 CEST 2013
cd /home/usr/b2daq/eb
killall eb0 > /dev/null 2>&1
sleep 1
./eb0 cpr010:33000 cpr011:33000 cpr012:33000

#EOT                    
