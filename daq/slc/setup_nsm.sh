#!/bin/bash

export B2SC_SERVER_HOST=`hostname`;

## setup for slow control system libraries ##
export BELLE2_DAQ_SLC_DIR=$PWD
export PATH=$PATH:$BELLE2_DAQ_SLC_DIR/bin
export LD_LIBRARY_PATH=$BELLE2_DAQ_SLC_DIR/lib:$LD_LIBRARY_PATH

## NSM configuration ##
export NSM2_HOST=${B2SC_SERVER_HOST}
export NSM2_PORT=8222
export NSM2_SHMKEY=8222
export NSM2_INCDIR=$BELLE2_DAQ_SLC_DIR/bin
export NSMD2_DEBUG=1
export NSMD2_LOGDIR=$BELLE2_DAQ_SLC_DIR/log/nsm2

