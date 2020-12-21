#!/bin/sh
#
# DAQ software test with dummy data for a new readout system
#
DES_SER_PATH=/home/usr/yamadas/basf2/releases/release_DAQ_upgrade/daq/ropc/
DUMMY_DATA_PATH=/home/usr/yamadas/basf2/releases/release_DAQ_upgrade/daq/rawdata/tools
#/usr/bin/xterm -fn 7x14 -geometry 40x7+300+242 -e "${HOME}/cprdaq/scripts/start_copper_mono.sh cpr008 83886088 15 0 1; sleep 3000000;" &


#Usage : ./dummy_data_distrib <node ID> <run#> <nwords of det. buf per FEE> <# of CPR per COPPER> <# of HSLBs>
/usr/bin/xterm -fn 7x14 -geometry 60x7+600+0 -e "${DUMMY_DATA_PATH}/dummy_data_threads 1 1 1000 1 1" &
sleep 3

#
# Need to increase the number of streams according to NUM_CLIENTS in dummy_data_distrib.cc
#

#/usr/bin/xterm -fn 7x14 -geometry 60x7+800+360 -e "${DES_SER_PATH}/des_ser_ROPC_main localhost 0 5102 test > ~/temp.txt" &
#/usr/bin/xterm -fn 7x14 -geometry 60x7+800+480 -e "${DES_SER_PATH}/des_ser_ROPC_main localhost 0 5103 test" &
#/usr/bin/xterm -fn 7x14 -geometry 60x7+800+500 -e "${DES_SER_PATH}/des_ser_ROPC_main localhost 0 5104 test" &
#/usr/bin/xterm -fn 7x14 -geometry 60x7+800+520 -e "${DES_SER_PATH}/des_ser_ROPC_main localhost 0 5105 test" &
#/usr/bin/xterm -fn 7x14 -geometry 60x7+800+540 -e "${DES_SER_PATH}/des_ser_ROPC_main localhost 0 5106 test" &
#sleep 3
/usr/bin/xterm -fn 7x14 -geometry 60x7+900+0   -e "nc localhost 30000 > /dev/null" &
/usr/bin/xterm -fn 7x14 -geometry 60x7+900+150 -e "nc localhost 30001 > /dev/null" &
/usr/bin/xterm -fn 7x14 -geometry 60x7+900+300 -e "nc localhost 30002 > /dev/null" &
/usr/bin/xterm -fn 7x14 -geometry 60x7+900+450 -e "nc localhost 30003 > /dev/null" &
/usr/bin/xterm -fn 7x14 -geometry 60x7+900+600 -e "nc localhost 30004 > /dev/null" &


