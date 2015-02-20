#!/bin/sh
#
# For KLM, CDC, ECL COPPERs slot A,B
#
/usr/bin/xterm -fn 7x14 -geometry 40x7+500+542 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh ${1} 5026 ${2} 0 33000; sleep 3000000;" &

# Note : argv[3] is a port # speciefd in ~b2daq/eb/eb0.sh
/usr/bin/xterm -fn 7x14 -geometry 40x10+400+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream2.sh ${1} 0 34000 33000; sleep 3000000;" &

#
# basf2 on ROPC
#
${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream1.sh 0 5101 10 > ~b2daq/yamadas/cprtest14May/${3}/cpr${4}_slot${2}_basf2_ropc1.txt

