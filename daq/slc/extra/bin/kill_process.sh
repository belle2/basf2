#!/bin/bash

PROC_PID=`ps aux | grep "nsmd2 ${1}" | grep -v grep | awk '{print $1}'`
if [ ${#PROC_PID} -gt 0 ]; then
kill $PROC_PID &> /dev/null
echo "killed nsmd2 ${1} (pid = ${PROC_PID})"
fi
