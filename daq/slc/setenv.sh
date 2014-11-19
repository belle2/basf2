#!/bin/bash

export BELLE2_LOCAL_DIR=$HOME/belle2/release
export BELLE2_DAQ_SLC=$PWD
export PATH=$BELLE2_DAQ_SLC/bin:$PATH
export PGSQL_LIB_PATH=/usr/pgsql-9.3/
export LD_LIBRARY_PATH=$BELLE2_DAQ_SLC/lib:$PGSQL_LIB_PATH/lib:$LD_LIBRARY_PATH
export NSM2_INCDIR=$BELLE2_LOCAL_DIR/daq/slc/data/nsm

#export BELLE2_DAQ_SLC=$PWD
#export PATH=$BELLE2_DAQ_SLC/bin:$PATH
#export PGSQL_LIB_PATH=/usr/pgsql-9.3/
#export LD_LIBRARY_PATH=$BELLE2_DAQ_SLC/lib:$PGSQL_LIB_PATH/lib:$LD_LIBRARY_PATH

