#!/bin/bash

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

