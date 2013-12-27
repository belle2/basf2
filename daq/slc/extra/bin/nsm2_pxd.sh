#!/bin/bash

export NSM2_INCDIR=$BELLE2_LOCAL_DIR/daq/slc/data/nsm
export NSM2_HOST=`hostname`
export NSM2_PORT=8122
export NSM2_SHMKEY=8122
export NSMD2_LOGDIR=~/run/nsmd2/${NSM2_HOST}
mkdir -p ${NSMD2_LOGDIR}
