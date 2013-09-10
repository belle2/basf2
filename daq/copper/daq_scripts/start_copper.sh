#!/bin/sh
ssh -n -n -XY $1 ${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/copper.sh $1 $2 $3
