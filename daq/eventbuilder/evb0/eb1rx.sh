#!/bin/sh

################################################################################
#
#
# This script should be executed on belle-hltin.
#
# Must be executed by xinetd, do not execute by hand.
#
# Just proxy connection from HLT to eb1tx at belle-rpc1 for the beamtest at DESY
#
#
################################################################################


cd /home/usr/b2daq/eb
killall eb1rx eb1rx-nc > /dev/null 2>&1
sleep 1
exec ./eb1rx-nc tx-rpc1 5111 2> /dev/null
