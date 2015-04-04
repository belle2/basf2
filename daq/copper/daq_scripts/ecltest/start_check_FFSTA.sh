#!/bin/sh
ssh -n -n -XY -l${USER} $1 ${HOME}/bin/check_FFSTA_all.sh >> ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/log/150215_FFSTA_$2.log
