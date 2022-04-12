#!/bin/bash

# Submits 15 files with 1Mio events each

i=0
nfiles=20
while [ $i -lt ${nfiles} ]; do 
        echo "JOB #: "$i
        log="./collector_$i.log"
        echo "output log file will be located: "$log
        bsub -q l -o $log "basf2 cluster_position_collector.py -n 1000000 -- --index $i"

        i=$(($i+1))
done
