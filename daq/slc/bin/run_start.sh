#!/bin/sh
#
# basf2 on COPPER CPU
#
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+642 -e "ssh -l tkonno -n -n -XY cpr006 ${BELLE2_LOCAL_DIR}/daq/slc/bin/start_copper.sh CPR006" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+642 -e "ssh -l tkonno -n -n -XY cpr015 ${BELLE2_LOCAL_DIR}/daq/slc/bin/start_copper.sh CPR015" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+642 -e "ssh -l tkonno -n -n -XY ropc01 ${BELLE2_LOCAL_DIR}/daq/slc/bin/start_readout.sh ROPC01" &
/usr/bin/xterm -fn 7x14 -geometry 102x10+0+642 -e "ssh -l tkonno -n -n -XY ropc01 ${BELLE2_LOCAL_DIR}/daq/slc/bin/runcontrold.sh" &

# event builder on ROPC
#
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+342 -e ${BELLE2_LOCAL_DIR}/daq/eventbuilder/evb0/eb0 -n 1 cpr006 -D -b &
#/usr/bin/xterm -fn 7x14 -geometry 102x10+0+342 -e ${BELLE2_LOCAL_DIR}/daq/eventbuilder/evb0/eb0 -n 2 cpr006 cpr015 -D -b &


#
# basf2 on ROPC
#
#${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvStream1.sh 0 35000