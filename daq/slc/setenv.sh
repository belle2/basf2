#!/bin/bash

## set NOT_USE_PSQL yes if postreSQL is not available
#export NOT_USE_PSQL=yes

export BELLE2_DAQ_SLC=$PWD
export PATH=$BELLE2_DAQ_SLC/bin:$PATH
if ! test "$BELLE2_LOCAL_DIR" = "" ; then
  if [ -d $BELLE2_EXTERNALS_DIR/lib/$BELLE2_EXTERNALS_SUBDIR/ ]; then
    export PGSQL_LIB_PATH=$BELLE2_EXTERNALS_DIR/lib/$BELLE2_EXTERNALS_SUBDIR
  else 
    export PGSQL_LIB_PATH=$BELLE2_EXTERNALS_DIR/$BELLE2_EXTERNALS_SUBDIR/lib
  fi
  export PGSQL_INC_PATH=$BELLE2_EXTERNALS_DIR/include/pgsql
  export LD_LIBRARY_PATH=$BELLE2_DAQ_SLC/lib:$LD_LIBRARY_PATH
else
  export PGSQL_LIB_PATH=/usr/pgsql-9.3/lib
  export PGSQL_INC_PATH=/usr/pgsql-9.3/include
fi
export LD_LIBRARY_PATH=$BELLE2_DAQ_SLC/lib:$PGSQL_LIB_PATH/lib:$LD_LIBRARY_PATH
export NSM2_INCDIR=$BELLE2_DAQ_SLC/data/nsm

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
