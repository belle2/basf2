#!/bin/sh
#
# setup basf2 environment (See "Setup of Software Tools" at https://belle2.cc.kek.jp/~twiki/bin/view/Computing/SoftwareInstallation
#
source  ~/.bash_profile
#
# Read data form COPPER FIFO
#
${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvSendCOPPER.sh $1 $2 $3 $4 $5
