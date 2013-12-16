#!/bin/bash

export NSM2_INCDIR=$BELLE2_LOCAL_DIR/daq/slc/data/nsm
export NSM2_HOST=`/sbin/ifconfig | grep 192.168.100 | sed "s/:/ /g" | awk '{print $3}'`
export NSM2_PORT=8122
export NSM2_SHMKEY=8122
