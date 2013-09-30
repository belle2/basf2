#!/bin/sh
#
# setup basf2 environment (See "Setup of Software Tools" at https://belle2.cc.kek.jp/~twiki/bin/view/Computing/SoftwareInstallation
#
source  ~/.bash_profile
#
# Read data form COPPER FIFO
#
basf2 ${BELLE2_LOCAL_DIR}/daq/rawdata/examples/RecvSendCOPPER.py $1 $2 $3 $4 --no-stat