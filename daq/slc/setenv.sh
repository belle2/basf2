#!/bin/bash

## set NOT_USE_PSQL yes if postreSQL is not available
#export NOT_USE_PSQL=yes

export BELLE2_DAQ_SLC=$PWD
export PATH=$BELLE2_DAQ_SLC/bin:$PATH
export PGSQL_LIB_PATH=${HOME}/slc/externals/pgsql/lib
export PGSQL_INC_PATH=${HOME}/slc/externals/pgsql/include
#export PGSQL_LIB_PATH=${B2DAQ_EXTERNALS_DIR}/lib
#export PGSQL_INC_PATH=${B2DAQ_EXTERNALS_DIR}/include/pgsql
export LD_LIBRARY_PATH=$BELLE2_DAQ_SLC/lib:$PGSQL_LIB_PATH:$LD_LIBRARY_PATH
export NSM2_INCDIR=$BELLE2_DAQ_SLC/data/nsm
export PYTHONPATH=${BELLE2_DAQ_SLC}/lib:${BELLE2_DAQ_SLC}/scripts:$PYTHONPATH

function setslc () {
    if [ $# -ne 1 ]; then
	echo "Current slowcontrol : " ${BELLE2_SLC_FILE}
	return 1
    fi
    export BELLE2_SLC_FILE=$1
    echo "Set slowcontrol : " ${BELLE2_SLC_FILE}
    return 0
}

function setnsmhost () {
    if [ $# -ne 1 ]; then
	echo "Current NSM2_HOST : " ${NSM2_HOST}
	return 1
    fi
    export NSM2_HOST=$1
    echo "Set NSM2_HOST : " ${NSM2_HOST}
    return 0
}

function setnsmport () {
    if [ $# -ne 1 ]; then
	echo "Current NSM2_PORT : " ${NSM2_PORT}
	return 1
    fi
    export NSM2_PORT=$1
    export NSM2_SHMKEY=$1
    echo "Set NSM2_PORT : " ${NSM2_PORT}
    return 0
}
