#!/bin/sh

################################################################################
#
#
# This script should be executed on belle-rpc1.
#
# Must be executed by xinetd, do not execute by hand.
#
# -e option indicates the port number where BASF2 is listening
# for the beamtest at DESY, it is 127.0.0.1:36000
#
# -i option indicates number of HLT units
# for the beamtest at DESY, it is 1
#
#
################################################################################

cd /home/usr/b2daq/eb
killall eb1tx > /dev/null 2>&1
#/usr/bin/nc 127.0.0.1 36000
./eb1tx -e 127.0.0.1:36000 -i 1
#cd /home/usr/eb2daq/eb
#./eb1tx -e 127.0.0.1:5001 -i 2
