#!/bin/sh
ssh -n -n -XY -l${USER} $1 ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/copper_wobasf2.sh $1 $2 $3 $4 $5
