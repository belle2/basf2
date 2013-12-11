#killall nsmd2
BIN_DIR=${BELLE2_LOCAL_DIR}/daq/slc/extra/bin
/usr/bin/xterm -e "source ${BIN_DIR}/start_local.sh nsmd2"&
sleep 10
/usr/bin/xterm -e "ssh -n -n -XY -l${USER} cpr006; source ${BIN_DIR}/start_local.sh nsmd2"&
/usr/bin/xterm -e "ssh -n -n -XY -l${USER} cpr009; source ${BIN_DIR}/start_local.sh nsmd2"&
#/usr/bin/xterm -e "source ${BIN_DIR}/nsm2_global.sh; runcontrold runcontrol"&
#/usr/bin/xterm -e "source ${BIN_DIR}/nsm2_global.sh; tempd PXD"&
#/usr/bin/xterm -e "source ${BIN_DIR}/nsm2_global.sh; tempd HLT"&
#/usr/bin/xterm -e "source ${BIN_DIR}/nsm2_global.sh; tempd DR"&

