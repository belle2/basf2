#!/bin/bash

#export BELLE2_LOCAL_DIR=$HOME/belle2/release
#export BELLE2_EXTERNAL_LOCAL_DIR=$HOME/belle2/external/v00-05-02
export BELLE2_LOCAL_DIR=../..
export BELLE2_EXTERNAL_LOCAL_DIR=../../../external/v00-05-02
export BELLE2_DAQ_SLC=$BELLE2_LOCAL_DIR/daq/slc
export PATH=$BELLE2_DAQ_SLC/bin:$PATH
export LD_LIBRARY_PATH=$BELLE2_DAQ_SLC/lib:$LD_LIBRARY_PATH

#export NSM2_INCDIR=$BELLE2_LOCAL_DIR/daq/slc/data/nsm
#export NSM2_HOST=daqhvpc01.kek.jp
#export NSM2_PORT=8122
#export NSM2_SHMKEY=8122
#export NSMD2_LOGDIR=$HOME/log/nsm
#mkdir -p ${NSMD2_LOGDIR}
