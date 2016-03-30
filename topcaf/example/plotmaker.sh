#!/bin/sh
#This script creates topcaf plots for a range of runs with a cuts for a specific trigger type applied
#It will submit them to the batch and combine the output into a single ROOT file
#options are
#1: start run
#2: end run
#3: filename suffix
#4: cut type
#
#or
#1: runlist
#2: cut type
#
#BF 2016-03-29

if [ "$#" -eq 4 ]; then
    
#Create bsubs for many jobs
#perl script to do sub scripts and hadd
    rlpre="${1}${2}${3}"
    rlname="${rlpre}.txt"
    cutname="${4}"

#create new submit scripts
    perl perl_make_list.pl $1 $2 $rlname $3

elif [ "$#" -eq 2 ]; then
    txs=".txt"
    rlname="${1}"
    rlpre="${1%$txs}"
    cutname="${2}"

else
    echo "Script requires 2 or 4 arguments. View code for details."
    exit 1
fi

subname="${rlpre}.sh"

mkdir ${subname}_sh_scripts_batch
perl perl_analyzeHits_extended.pl $rlname $subname $cutname

#sub the jobs
./sub_${subname}analyzeHits_extended.sh
#will wait until the batch jobs finish

#hadd the result
./hadd_${subname}_plothits_${cutname}.sh

#execute plotting script

#clean up crap
echo "Cleaning up"
if [ "$#" -eq 4 ]; then
    rm -rf $rlname
fi
rm -rf sub_${subname}analyzeHits_extended.sh
rm -rf hadd_${subname}_plothits_${cutname}.sh
rm -rf ${subname}_sh_scripts_batch
