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
#exec ./eb1rx ecl01:5111 ecl02:5111 
#exec ./eb1rx decl01:5111 decl02:5111 
#exec ./eb1rx dcdc01:5111 dcdc02:5111 dcdc03:5111 dcdc42:5111 dcdc51:5111
#exec ./eb1rx dcdc01:5101 dcdc02:5101 dcdc04:5101 dcdc05:5101
#exec ./eb1rx dcdc01:5101 
exec ./eb1rx din:5101 
#exec ./eb1rx dcdc01:5101 dcdc02:5101 dcdc04:5101 dcdc05:5101
#exec ./eb1rx dcdc02:5101 dcdc05:5101
#exec ./eb1rx ecl02:5111 
#exec ./eb1rx ecl01:5111 ecl02:5111 2> /dev/null 
#exec ./eb1rx-nc ecl01 5111 2> /dev/null
# exec ./eb1rx-nc tx-rpc1 5111 2> /dev/null
