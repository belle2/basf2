#!/bin/sh
#
# setup basf2 environment 
#
source  ~/.bash_profile
#
# Read data form COPPER FIFO
#
pgrep -f 'RecvOnlyCOPPER' | xargs kill
sleep 0.1
${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvOnlyCOPPER.sh $1 $2 $3 $4 $5
