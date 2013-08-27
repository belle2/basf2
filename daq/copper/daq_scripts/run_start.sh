#!/bin/sh
#
# basf2 on COPPER CPU
#
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+642 -e ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr006 1 &
/usr/bin/xterm -fn 7x14 -geometry 102x10+750+642 -e ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_copper.sh cpr007 2 &
#
# event builder on ROPC
#
${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/start_eb0.sh &

#
# basf2 on ROPC
#
${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream1.sh