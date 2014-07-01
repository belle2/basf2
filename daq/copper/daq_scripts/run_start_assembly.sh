#!/bin/sh
#
# For KLM, CDC, ECL COPPERs slot A,B
#
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh ${1} 0 ${2} 0 0; sleep 3000000;" &

#
# basf2 on ROPC
#
${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream1.sh 0 5101 10
