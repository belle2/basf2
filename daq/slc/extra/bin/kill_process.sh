#!/bin/bash

PROC_PID=`ps aux | grep "${1} ${2}" | grep -v grep | grep -v kill_process | awk '{print \$2}'`
if [ ${#PROC_PID} -gt 0 ]; then
kill $PROC_PID &> /dev/null
echo "killed ${1} ${2} (pid = ${PROC_PID})"
fi
