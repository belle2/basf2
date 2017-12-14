#!/bin/sh
#
# setup basf2 environment 
#
source  ~/.bash_profile
#
# Read data form COPPER FIFO
#
${BELLE2_LOCAL_DIR}/daq/ropc/des_ser_COPPER_main $1 $2  $3 $4 $5 #

#
# Use dummy data instead of data from COPPER FIFO
#
#${BELLE2_LOCAL_DIR}/daq/copper/daq_scripts/RecvSendCOPPERDummy.sh $1 $2  $3 $4 $5 # Dummy data produced by COPPER CPU
#${BELLE2_LOCAL_DIR}/daq/rawdata/src/dummy_data_src $2 1234 100 1 4
# Usage : dummy_data_src <node ID> <run#> <nwords of det. buf per FEE> <# of CPR per COPPER> <# of HSLBs>
