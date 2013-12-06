#!/bin/sh
#
# basf2 on COPPER CPU
#

/usr/bin/xterm -fn 7x14 -geometry 102x10+0+242 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr014 1 1 0; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+442 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr003 2 1 0; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+642 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr004 3 1 0; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+750+002 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr008 4 1 0; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+750+152 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr009 5 1 0; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+750+302 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr012 6 1 0; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+750+452 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr014 7 1 0; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+750+602 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr016 8 1 0; sleep 3000000;" &

#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+642 -e sh -c '${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr006 1 1; read line' &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+642 -e sh -c 'source  ~/.bash_profile; basf2 ${BELLE2_LOCAL_DIR}/daq/rawdata/examples/RecvSendCOPPER.py cpr006 1 1 --no-stat; read line' &

#/usr/bin/xterm -fn 7x14 -geometry 102x10+750+642 -e ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr015 2 1 0 &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+750+642 -e sh -c '${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr007 1 1; read line' &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+750+642 -e sh -c 'source  ~/.bash_profile; basf2 ${BELLE2_LOCAL_DIR}/daq/rawdata/examples/RecvSendCOPPER.py cpr007 1 1 --no-stat; read line' &
#
# event builder on ROPC
#
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+342 -e ${BELLE2_LOCAL_DIR}/daq/eventbuilder/evb0/eb0 -n 1 cpr006 -D -b &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+342 -e ${BELLE2_LOCAL_DIR}/daq/eventbuilder/evb0/eb0 -n 2 cpr006 cpr015 -D -b &


#
# basf2 on ROPC
#
${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream1.sh 0 5101