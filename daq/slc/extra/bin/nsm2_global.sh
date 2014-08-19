#!/bin/bash

export NSM2_INCDIR=$BELLE2_LOCAL_DIR/daq/slc/data/nsm
export NSM2_HOST=172.22.37.11
export NSM2_PORT=9122
export NSM2_SHMKEY=9122
export NSMD2_LOGDIR=$HOME/log/nsm
mkdir -p ${NSMD2_LOGDIR}
