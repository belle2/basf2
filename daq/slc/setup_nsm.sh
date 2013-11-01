#!/bin/bash

export B2SC_SERVER_HOST=`hostname`;

## setup for slow control system libraries ##
export B2SLC_PATH=$PWD
export PATH=$PATH:$B2SLC_PATH/bin
export LD_LIBRARY_PATH=$B2SLC_PATH/lib:$LD_LIBRARY_PATH

## NSM configuration ##
export NSM2_HOST=${B2SC_SERVER_HOST}
export NSM2_PORT=8222
export NSM2_SHMKEY=8222
export NSM2_INCDIR=$B2SLC_PATH/bin
export NSMD2_DEBUG=1
export NSMD2_LOGDIR=$B2SLC_PATH/log/nsm2

