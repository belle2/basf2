#!/bin/sh
#${BELLE2_LOCAL_DIR}/daq/rawdata/examples/RecvStream2.py $1 $2 $3 $4 --no-stat 
#${BELLE2_LOCAL_DIR}/daq/rawdata/src/des_ser_prePC_main $1 $2 $3 $4 
${BELLE2_LOCAL_DIR}/daq/ropc/des_ser_ROPC_main $1 $2 $3 $4 
