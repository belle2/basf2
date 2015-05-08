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