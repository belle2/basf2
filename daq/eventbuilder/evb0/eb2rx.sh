#!/bin/sh


################################################################################
#
#
# This script should be executed on belle-rpc2.
#
# Must be executed by xinetd, do not execute by hand.
#
# If ONSEN is active, eb2rx must be used.
# eb2rx needs at least two arguments
# first argument must be specify the host+port of the HLT output
# later argments must be host+port to ONSEN.
#
# In the case of the beamtest at DESY, there is only one ONSEN.
# So it will be
#
# ./eb2rx 192.168.102.6:5131 10.10.10.81:24
#
# If ONSEN is inactive, just NC is enough.
# ./eb2rx-nc 192.168.102.6 5131
#
# If you want to use ONSEN simulrator,
# ./eb2rx 192.168.102.6:5131 [HOSTNAME for SIMULATOR]:[PORTNUMBER of SIMURATOR]
#
# But the format of ONSEN simuraltor we have at 2014-01-13 is older than
# that eb2rx expects. So do not use it.
#
#
################################################################################

WITH_ONSEN=yes # please specify yes or no

cd /home/usr/b2daq/eb
killall eb2rx > /dev/null 2>&1
killall eb2rx-nc > /dev/null 2>&1
sleep 0.5


if [ "$WITH_ONSEN" = yes ] ; then
    ./eb2rx 192.168.102.6:5131 10.10.10.81:24
else
#    ./eb2rx-nc 192.168.102.6 5131 | tee out-from-eb2.dat.$$
    ./eb2rx-nc 192.168.102.6 5131
fi

#
# with ONSEN, 2014-01-05
#
#./eb2rx 192.168.102.6:5131 10.10.10.81:24
#
# without ONSEN, 2013-12-27 ~ 2014-1-2
#
#./eb2rx-nc 192.168.102.6 5131
#
# with ONSEN simulator on rpc2, 2014-1-3
#
#./eb2rx 192.168.102.6:5131 127.0.0.1:8024
#
# with ONSEN, 2013-10
#
#./eb2rx 192.168.102.6:5131 10.10.10.81:24 | tee out-from-eb2.dat.$$
#./eb2rx 192.168.102.6:5131 127.1:8024 | tee /x01/suzuki/out-from-eb2.dat.$$
