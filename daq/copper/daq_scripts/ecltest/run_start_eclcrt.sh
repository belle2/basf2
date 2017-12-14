#!/bin/sh
#
# For KLM, CDC, ECL COPPERs slot A,B
#
#    print 'Usage : RecvSendCOPPER.py <COPPER hostname> <COPPER node ID> <bit flag of FINNESEs> <Use NSM(Network Shared Memory)? yes=1/no=0> <NSM nodename>'
# bit flag of FINESSE
# slot a : 1, slot b : 2, slot c : 4, slot d : 8
# e.g. slot abcd -> bitflag=15, slot bd -> bit flag=10


/usr/bin/xterm -fn 7x14 -geometry 102x10+500+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/ecltest/stop_trigger.sh 172.22.32.48" 

/usr/bin/xterm -fn 7x14 -geometry 40x7+000+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr5001 83886081 3 0 1 $1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+010+292 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr5002 83886082 3 0 2 $1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+020+342 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr5003 83886083 3 0 2 $1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+030+392 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr5004 83886084 3 0 2 $1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+040+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr5005 83886085 3 0 2 $1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+050+492 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr5006 83886086 3 0 2 $1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+060+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr5007 83886087 3 0 2 $1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+070+292 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr5008 83886088 3 0 2 $1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+080+342 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr5009 83886089 3 0 2 $1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+090+392 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr5010 83886090 3 0 2 $1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+100+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr5011 83886091 3 0 2 $1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+110+492 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr5012 83886092 3 0 2 $1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+120+542 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr5013 83886093 3 0 2 $1; sleep 3000000;" &


#
# event builder on ROPC
#
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+342 -e ${BELLE2_LOCAL_DIR}/daq/eventbuilder/evb0/eb0 -n 1 cpr006 -D -b &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+342 -e ${BELLE2_LOCAL_DIR}/daq/eventbuilder/evb0/eb0 -n 2 cpr006 cpr015 -D -b &

#
# basf2 on ROPC
#
#    print 'Usage : RecvStream0.py <COPPER hostname> <Use NSM(Network Shared Memory)? yes=1/no=0> <port # of eb0> <NSM nodename>
#/usr/bin/xterm -fn 7x14 -geometry 102x10+400+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr001 0 32001 hogehoge1000; sleep 3000000;" &
#case 
/usr/bin/xterm -fn 7x14 -geometry 102x10+400+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr5001 0 34001 hogehoge1000; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+410+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr5002 0 34002 hogehoge1000; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+420+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr5003 0 34003 hogehoge1000 >> ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_RecvStream2_ecl01_03_$1.log; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+430+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr5004 0 34004 hogehoge1000 >> ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_RecvStream2_ecl01_04_$1.log; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+440+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr5005 0 34005 hogehoge1000 >> ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_RecvStream2_ecl01_05_$1.log; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+450+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr5006 0 34006 hogehoge1000 >> ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_RecvStream2_ecl01_06_$1.log; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+460+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr5007 0 34007 hogehoge1000 >> ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_RecvStream2_ecl01_07_$1.log; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+470+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr5008 0 34008 hogehoge1000 >> ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_RecvStream2_ecl01_08_$1.log; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+480+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr5009 0 34009 hogehoge1000 >> ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_RecvStream2_ecl01_09_$1.log; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+490+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr5010 0 34010 hogehoge1000 >> ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_RecvStream2_ecl01_10_$1.log; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+500+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr5011 0 34011 hogehoge1000 >> ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_RecvStream2_ecl01_11_$1.log; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+510+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr5012 0 34012 hogehoge1000 >> ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_RecvStream2_ecl01_12_$1.log; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+520+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh cpr5013 0 34013 hogehoge1000 >> ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_RecvStream2_ecl01_13_$1.log; sleep 3000000;" &

#
# event builder on ROPC (usually automatically invoked by inetd)
#
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+342 -e ${BELLE2_LOCAL_DIR}/daq/eventbuilder/evb0/eb0 -n 1 cpr006 -D -b &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+342 -e ${BELLE2_LOCAL_DIR}/daq/eventbuilder/evb0/eb0 -n 2 cpr006 cpr015 -D -b &


#
# basf2 program AFTER eb0(event builder 0) on a readout PC
#
#    print 'Usage : RecvStream1.py <Use NSM(Network Shared Memory)? yes=1/no=0> <port # of eb0> <NSM nodename>
/usr/bin/xterm -fn 7x14 -geometry 102x10+100+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/ecltest/start_run_start.sh ecl02 $1; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+100+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream1.sh 0 5101 hogehoge1000 $1 ; sleep 3000000; " &

sleep 10
/usr/bin/xterm -fn 7x14 -geometry 102x10+500+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/ecltest/start_trigger.sh 172.22.32.48" &


while :
do
#cp temp.txt temp2.txt
touch temp.txt
echo $cnt
sleep 1
cnt=`expr $cnt + 1`
date >> ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_FFSTA_$1.log
${HOME}/bin/check_FFSTA_all.sh >> ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_FFSTA_$1.log
${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/ecltest/start_check_FFSTA.sh ecl02 $1
if test $cnt -ge 40
then 
if test  ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_RecvStream1_ecl01_$1.log -nt temp.txt
then
echo ""
cnt=1
else
break
fi
fi
done

/usr/bin/xterm -fn 7x14 -geometry 102x10+100+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/stop_trigger.sh 172.22.32.48" &