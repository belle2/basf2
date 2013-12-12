#killall nsmd2
BIN_DIR=${BELLE2_LOCAL_DIR}/daq/slc/extra/bin

killall nsmd2
killall runcontrold
killall tempd
/usr/bin/xterm -e "source ${BIN_DIR}/start_global.sh nsmd2"&
echo -n "enter a key to continue>"
read INPUT
/usr/bin/xterm -e "source ${BIN_DIR}/start_global.sh runcontrold runcontrol"&
/usr/bin/xterm -e "source ${BIN_DIR}/nsm2_global.sh; tempd TTD"&
/usr/bin/xterm -e "source ${BIN_DIR}/nsm2_global.sh; tempd PXD"&
/usr/bin/xterm -e "source ${BIN_DIR}/nsm2_global.sh; tempd HLT"&
/usr/bin/xterm -e "source ${BIN_DIR}/nsm2_global.sh; tempd DR"&

