#!/bin/sh
ssh -n -n -XY -l${USER} $1 ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/run_start.sh $2
