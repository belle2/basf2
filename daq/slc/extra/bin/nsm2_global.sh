#!/bin/bash

export NSM2_INCDIR=$BELLE2_LOCAL_DIR/daq/slc/data/nsm
export NSM2_HOST=130.87.227.252
export NSM2_PORT=8122
export NSM2_SHMKEY=8122
export NSMD2_LOGDIR=$HOME/log/nsm
mkdir -p ${NSMD2_LOGDIR}
