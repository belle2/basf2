#!/bin/bash

export NSM2_INCDIR=$BELLE2_LOCAL_DIR/daq/slc/data/nsm
export NSM2_HOST=b2slow2.kek.jp
export NSM2_PORT=8122
export NSM2_SHMKEY=8122
export NSMD2_LOGDIR=$HOME/log/nsm
mkdir -p ${NSMD2_LOGDIR}
