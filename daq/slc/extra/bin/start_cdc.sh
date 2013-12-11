#killall nsmd2
BIN_DIR=${BELLE2_LOCAL_DIR}/daq/slc/extra/bin

killall nsmd2
killall tempd
killall runcontrold
ssh -n -n -XY -l${USER} cpr006 killall nsmd2
ssh -n -n -XY -l${USER} cpr006 killall tempd
ssh -n -n -XY -l${USER} cpr009 killall nsmd2
ssh -n -n -XY -l${USER} cpr009 killall tempd
/usr/bin/xterm -e "source ${BIN_DIR}/start_global.sh nsmd2"&
/usr/bin/xterm -e "source ${BIN_DIR}/start_local.sh nsmd2"&
echo -n "enter a key to continue>"
read INPUT
/usr/bin/xterm -e "ssh -n -n -XY -l${USER} cpr006 source ${BIN_DIR}/start_local.sh nsmd2"&
sleep 2
/usr/bin/xterm -e "ssh -n -n -XY -l${USER} cpr009 source ${BIN_DIR}/start_local.sh nsmd2"&
echo -n "enter a key to continue>"
read INPUT
#/usr/bin/xterm -e "ssh -n -n -XY -l${USER} cpr006 source ${BIN_DIR}/start_local.sh tempd CPR006"&
#/usr/bin/xterm -e "ssh -n -n -XY -l${USER} cpr009 source ${BIN_DIR}/start_local.sh tempd CPR009"&
/usr/bin/xterm -e "ssh -XY -l${USER} cpr006"&
/usr/bin/xterm -e "ssh -XY -l${USER} cpr009"&
/usr/bin/xterm -e "source ${BIN_DIR}/start_local.sh tempd ROPC01"&
/usr/bin/xterm -e "source ${BIN_DIR}/start_global.sh runcontrold cdc"&
#/usr/bin/xterm -e "source ${BIN_DIR}/nsm2_global.sh; tempd PXD"&
#/usr/bin/xterm -e "source ${BIN_DIR}/nsm2_global.sh; tempd HLT"&
#/usr/bin/xterm -e "source ${BIN_DIR}/nsm2_global.sh; tempd DR"&

