#!/bin/bash

if [ ${#1} ]; then
source ~/belle2/release/daq/slc/extra/bin/nsm2_$1.sh
fi

NSMD_PID=`ps aux | grep "nsmd2:${NSM2_PORT}" | grep -v grep | awk '{print $2}'`
if [ ${#NSMD_PID} -gt 0 ]; then
kill $NSMD_PID &> /dev/null
echo "killed nsmd2:${NSM2_PORT} (pid = ${NSMD_PID})"
fi
