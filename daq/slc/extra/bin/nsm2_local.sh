#!/bin/bash

export NSM2_INCDIR=$BELLE2_LOCAL_DIR/daq/slc/data/nsm
export NSM2_HOST=`hostname`
export NSM2_PORT=9122
export NSM2_SHMKEY=9122
#export NSM2_HOST=192.168.100.2
#export NSM2_PORT=9120
#export NSM2_SHMKEY=9120
export NSMD2_LOGDIR=~/run/nsm2/${NSM2_HOST}
mkdir -p ${NSMD2_LOGDIR}
