#!/bin/bash

export NSM2_INCDIR=$BELLE2_LOCAL_DIR/daq/slc/data/nsm
export NSM2_HOST=192.168.100.120
export NSM2_PORT=8124
export NSM2_SHMKEY=8124
export NSMD2_LOGDIR=~/run/nsm2/ttd3
mkdir -p ${NSMD2_LOGDIR}
