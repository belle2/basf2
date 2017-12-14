#!/bin/sh
#
# For KLM, CDC, ECL COPPERs slot A,B
#
#    print 'Usage : RecvSendCOPPER.py <COPPER hostname> <COPPER node ID> <bit flag of FINNESEs> <Use NSM(Network Shared Memory)? yes=1/no=0> <NSM nodename>'
# bit flag of FINESSE
# slot a : 1, slot b : 2, slot c : 4, slot d : 8
# e.g. slot abcd -> bitflag=15, slot bd -> bit flag=10
/usr/bin/xterm -fn 7x14 -geometry 40x7+300+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper_mono.sh cpr008 83886088 15 0 1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+320+262 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper_mono.sh cpr009 83886089 15 0 1; sleep 3000000;" &


#
# basf2 program BEFORE eb0(event builder 0) on a readout PC
#
#    print 'Usage : RecvStream0.py <COPPER hostname> <Use NSM(Network Shared Memory)? yes=1/no=0> <port # of eb0> <NSM nodename>
/usr/bin/xterm -fn 7x14 -geometry 40x10+400+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2_mono.sh cpr008 0 34008 hogehoge1000; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x10+420+462 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2_mono.sh cpr009 0 34009 hogehoge1000; sleep 3000000;" &

#
# event builder on ROPC (usually automatically invoked by inetd)
#
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+342 -e ${BELLE2_LOCAL_DIR}/daq/eventbuilder/evb0/eb0 -n 1 cpr006 -D -b &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+342 -e ${BELLE2_LOCAL_DIR}/daq/eventbuilder/evb0/eb0 -n 2 cpr006 cpr015 -D -b &


#
# basf2 program AFTER eb0(event builder 0) on a readout PC
#
#    print 'Usage : RecvStream1.py <Use NSM(Network Shared Memory)? yes=1/no=0> <port # of eb0> <NSM nodename>
${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream1.sh 0 314159265 hogehoge1000

