#!/bin/sh
#
# For KLM, CDC, ECL COPPERs slot A,B
#
#    print 'Usage : RecvSendCOPPER.py <COPPER hostname> <COPPER nodeID> <bit flag of FINNESEs> <Use shared memory? yes=1/no=0> <nodename>'
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr001 1 1 0 0; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+292 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr002 2 1 0 0; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+342 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr003 3 1 0 0; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+200+392 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr004 4 1 0 0; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+200+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr005 5 1 0 0; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+200+492 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr006 6 1 0 0; sleep 3000000;" &
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
