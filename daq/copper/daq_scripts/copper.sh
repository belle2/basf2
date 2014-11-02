#!/bin/sh
#
# setup basf2 environment (See "Setup of Software Tools" at https://belle2.cc.kek.jp/~twiki/bin/view/Software/SoftwareInstallation
#
source  ~/.bash_profile
#
# Read data form COPPER FIFO
#
${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvSendCOPPER.sh $1 $2 $3 $4 $5 # Read from COPPER FIFO

#
# Use dummy data instead of data from COPPER FIFO
#
#${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvSendCOPPERDummy.sh $1 $2 $3 $4 $5 # Dummy data produced by COPPER CPU
