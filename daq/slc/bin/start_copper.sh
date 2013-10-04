#!/bin/sh

source ~/release/daq/slc/bin/setup.sh
${BELLE2_LOCAL_DIR}/daq/slc/bin/cprcontrold $1 
sleep 1000

