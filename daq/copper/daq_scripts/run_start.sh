#!/bin/sh
# 
# For KLM, CDC, ECL COPPERs slot A,B
#
#    print 'Usage : RecvSendCOPPER.py <COPPER hostname> <COPPER node ID> <bit flag of FINNESEs> <Use NSM(Network Shared Memory)? yes=1/no=0> <NSM nodename>'
# bit flag of FINESSE
# slot a : 1, slot b : 2, slot c : 4, slot d : 8
# e.g. slot abcd -> bitflag=15, slot bd -> bit flag=10

#/usr/bin/xterm -fn 7x14 -geometry 40x7+300+492 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr001 83886081 3 0 2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x7+500+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr002 83886082 3 0 2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x7+500+292 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr003 83886083 3 0 2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x7+500+342 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr004 83886084 3 0 2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x7+500+392 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr005 83886085 3 0 2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x7+500+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr006 83886086 3 0 2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x7+500+492 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr007 83886087 3 0 2; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+300+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr008 83886088 3 0 1; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x7+300+292 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr009 83886089 3 0 2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x7+300+342 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr010 83886090 3 0 2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x7+300+392 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr011 83886091 3 0 2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x7+300+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr012 83886092 3 0 2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x7+500+542 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr013 83886093 3 0 2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x7+500+542 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr014 83886094 3 0 2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x7+500+542 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr015 83886095 3 0 2; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x7+500+542 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr016 83886096 3 0 2; sleep 3000000;" &

#
# basf2 program BEFORE eb0(event builder 0) on a readout PC
#
#    print 'Usage : RecvStream0.py <COPPER hostname> <Use NSM(Network Shared Memory)? yes=1/no=0> <port # of eb0> <NSM nodename>
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr001 0 34001 hogehoge1000; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr002 0 34002 hogehoge1000; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr003 0 34003 hogehoge1000; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr004 0 34004 hogehoge1000; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr005 0 34005 hogehoge1000; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr006 0 34006 hogehoge1000; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr007 0 34007 hogehoge1000; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x10+400+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr008 0 34007 hogehoge1000; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr009 0 34009 hogehoge1000; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr010 0 34010 hogehoge1000; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr011 0 34011 hogehoge1000; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr012 0 34012 hogehoge1000; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr013 0 34013 hogehoge1000; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr014 0 34014 hogehoge1000; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr015 0 34015 hogehoge1000; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 40x10+400+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr016 0 34016 hogehoge1000; sleep 3000000;" &

#
# event builder on ROPC (usually automatically invoked by inetd)
#
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+342 -e ${BELLE2_LOCAL_DIR}/daq/eventbuilder/evb0/eb0 -n 1 cpr006 -D -b &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+342 -e ${BELLE2_LOCAL_DIR}/daq/eventbuilder/evb0/eb0 -n 2 cpr006 cpr015 -D -b &


#
# basf2 program AFTER eb0(event builder 0) on a readout PC
#
#    print 'Usage : RecvStream1.py <Use NSM(Network Shared Memory)? yes=1/no=0> <port # of eb0> <NSM nodename>
${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream1.sh 0 34007 hogehoge1000

