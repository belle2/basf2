#!/bin/sh
#
# basf2 on ROPC
#
#${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream1.sh 0 5101 10 > ~b2daq/yamadas/cprtest14May/${3}/cpr${4}_slot${2}_basf2_ropc1.txt
#${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream1.sh 0 34000 10 > ~b2daq/yamadas/cprtest14May/${3}/cpr${4}_slot${2}_basf2_ropc1.txt
/usr/bin/xterm -fn 7x14 -geometry 100x10+300+442 -e "nice -19 ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream1.sh 0 34000 10 > ~b2daq/yamadas/cprtest14May/${3}/cpr${4}_slot${2}_basf2_ropc1.txt; sleep 300000;" &

#
# For KLM, CDC, ECL COPPERs slot A,B
#
/usr/bin/xterm -fn 7x14 -geometry 100x10+500+542 -e "nice -19 ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh ${1} 16777217 ${2} 0 33000; sleep 3000000;" &

# Note : argv[3] is a port # speciefd in ~b2daq/eb/eb0.sh
/usr/bin/xterm -fn 7x14 -geometry 100x10+400+492 -e "nice -19 ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh ${1} 0 34000 33000; sleep 3000000;" &

ssh -n -n -XY -l${USER} ttd3s "~nakao/bin/trigft -23 reset"

sleep 5
echo "start trigger"
ssh -n -n -XY -l${USER} ttd3s "~nakao/bin/trigft -23 random 10000 -1"
sleep 40
ssh -n -n -XY -l${USER} ttd3s "~nakao/bin/trigft -23 reset"
~/bin/kill_basf2_all.sh
less ~b2daq/yamadas/cprtest14May/${3}/cpr${4}_slot${2}_basf2_ropc1.txt
