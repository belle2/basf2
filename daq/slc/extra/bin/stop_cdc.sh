#killall nsmd2
BIN_DIR=${BELLE2_LOCAL_DIR}/daq/slc/extra/bin

killall nsmd2
ssh -n -n -XY -l${USER} cpr006 killall nsmd2
ssh -n -n -XY -l${USER} cpr009 killall nsmd2


