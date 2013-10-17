#!/bin/sh
#
# basf2 on COPPER CPU
#
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+642 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr001 1 1 0; sleep 3000000;" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+642 -e "${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr002 2 1 0; sleep 3000000;" &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+642 -e ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr006 1 3 0 &
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