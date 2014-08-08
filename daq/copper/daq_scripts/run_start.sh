#!/bin/sh
#
# For KLM, CDC, ECL COPPERs slot A,B
#
#    print 'Usage : RecvSendCOPPER.py <COPPER hostname> <COPPER node ID> <bit flag of FINNESEs> <Use NSM(Network Shared Memory)? yes=1/no=0> <NSM nodename>'
# bit flag of FINESSE
# slot a : 1, slot b : 2, slot c : 4, slot d : 8
# e.g. slot abcd -> bitflag=15, slot bd -> bit flag=10

#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr001 1 1 0 hogehoge1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr002 5002 3 0 hogehoge2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr005 1 8 0 hogehoge3; sleep 3000000;" &

#
# basf2 on ROPC
#
#    print 'Usage : RecvStream1.py <Use NSM(Network Shared Memory)? yes=1/no=0> <port # of eb0> <NSM nodename>'
${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream1.sh 0 5101 hogehoge1000
