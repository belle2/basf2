#!/bin/sh
#
# basf2 on COPPER CPU
#
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper_recvonly.sh cpr013 1 1 0 1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper_recvonly.sh cpr014 1 1 0 2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper_recvonly.sh cpr005 1 8 0 1; sleep 3000000;" &
