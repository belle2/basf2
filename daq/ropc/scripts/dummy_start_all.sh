#!/bin/sh
#
# For KLM, CDC, ECL COPPERs slot A,B
#
#    print 'Usage : RecvSendCOPPER.py <COPPER hostname> <COPPER node ID> <bit flag of FINNESEs> <Use NSM(Network Shared Memory)? yes=1/no=0> <NSM nodename>'
# bit flag of FINESSE
# slot a : 1, slot b : 2, slot c : 4, slot d : 8
# e.g. slot abcd -> bitflag=15, slot bd -> bit flag=10

# SVD
/usr/bin/xterm -fn 7x14 -geometry 40x7+30+30 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh svd01 0x01000001 9001 115 6 1;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+30+150 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh svd02 0x01000007 9001 115 6 1;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+30+270 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh svd03 0x0100000d 9001 115 6 1;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+30+390 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh svd04 0x01000013 9001 115 6 1;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+30+510 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh svd05 0x01000019 9001 115 6 1;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+30+630 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh svd06 0x0100001f 9001 115 6 1;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+340+30 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh svd07 0x01000025 9001 115 6 1;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+340+150 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh svd08 0x0100002b 9001 115 6 1;" &
#CDC
/usr/bin/xterm -fn 7x14 -geometry 40x7+340+270 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh cdc01 0x02000001 9001 10 9 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+340+390 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh cdc02 0x0200000a 9001 10 9 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+340+510 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh cdc03 0x02000013 9001 10 9 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+340+630 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh cdc04 0x0200001c 9001 10 8 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+180+30 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh cdc05 0x02000022 9001 10 8 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+180+150 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh cdc06 0x02000028 9001 10 8 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+180+270 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh cdc07 0x0200002e 9001 10 8 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+180+390 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh cdc08 0x02000036 9001 10 8 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+180+510 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh cdc09 0x0200003c 9001 10 8 4;" &
#TOP
/usr/bin/xterm -fn 7x14 -geometry 40x7+180+630 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh top01 0x03000001 9001 13 4 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+790+30 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh top02 0x03000005 9001 13 4 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+790+150 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh top03 0x03000009 9001 13 4 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+790+270 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh top04 0x0300000d 9001 13 4 4;" &
#ARICH
/usr/bin/xterm -fn 7x14 -geometry 40x7+790+390 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh arich01 0x04000001 9001 12 7 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+790+510 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh arich02 0x04000008 9001 12 7 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+790+630 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh   cdc10 0x0400000f 9001 12 6 4;" &
#ECL
/usr/bin/xterm -fn 7x14 -geometry 40x7+655+30 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh ecl01 0x05000001 9001 58 5 2;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+655+150 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh ecl02 0x05000006 9001 58 4 2;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+655+270 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh ecl03 0x0500000a 9001 58 4 2;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+655+390 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh ecl04 0x0500000e 9001 58 5 2;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+655+510 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh ecl05 0x06000001 9001 58 4 2;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+655+630 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh ecl06 0x06000005 9001 58 4 2;" &
#KLM
/usr/bin/xterm -fn 7x14 -geometry 40x7+50+100 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh klm01 0x07000001 9001 21 3 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+50+220 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh klm02 0x07000004 9001 21 3 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+50+370 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh klm03 0x08000001 9001 10 5 4;" &
/usr/bin/xterm -fn 7x14 -geometry 40x7+50+490 -e "/home/usr/yamadas/basf2/release/daq/rawdata/src/dummy_start.sh ecl07 0x08000006 9001 10 4 4;" &


