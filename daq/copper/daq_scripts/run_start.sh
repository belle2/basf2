#!/bin/sh
#
# For KLM, CDC, ECL COPPERs slot A,B
#
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr001 0 2 0 0; sleep 3000000;" &
#                                                                                                                          *** means FINESSE_STA
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr004 0 1 0 0; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr015 0 1 0 0; sleep 3000000;" &


#
# For KLM, CDC COPPERs slot C,D
#0
##/usr/bin/xterm -fn 7x14 -geometry 102x10+0+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr001 0 c 0 0; sleep 3000000;" &

#
# basf2 on ROPC
#
${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream1.sh 0 5101 10
