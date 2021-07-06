#!/bin/bash

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

echo "run this script: source SVDCoGTimeCalibrationCheck.sh <exp> <input.txt path> <database.txt path> <log files path>"

exp_number=${1}
inputList=${2}
localDBFolder=${3}
logfiles=${4}
OUTPUTCHECKFILES="tree"

if [ ! -d ${logfiles}  ]; then
    echo "WARNING!"
    echo "the output folder ${logfiles} does not exist. Create it before launching simulation."
    echo "mkdir ${logfiles}"
    mkdir ${logfiles}
fi

if [ ! -d ${OUTPUTCHECKFILES}  ]; then
    echo "WARNING!"
    echo "the output folder ${OUTPUTCHECKFILES} does not exist. Create it before launching simulation."
    echo "mkdir ${OUTPUTCHECKFILES}"
    mkdir ${OUTPUTCHECKFILES}
fi

changeIFSlocal() {
  IFS="/"
}

changeIFSglobal() {
  IFS=""
}

i=0
for file in ${inputList}/*.txt;
do  
  changeIFSlocal
  # IFS="/"
  read -ra ADDR <<< "${file}"
  run_number=$(echo ${ADDR[1]} | sed 's/[^0-9]*//g')
  changeIFSglobal

  echo "RUN: ${run_number} EXP: ${exp_number} FILE: ${file}" 
  
  bsub -q s -oo ${logfiles}/log_cog_${run_number}.txt basf2 SVDCoGTimeCalibrationCheck.py ${localDBFolder} ${file} ${run_number} ${exp_number}

  echo "bsub -q s -oo ${logfiles}/log_cog_${run_number}.txt basf2 SVDCoGTimeCalibrationCheck.py ${localDBFolder} ${file} ${run_number} ${exp_number}" 
  (( i += 1))
done
