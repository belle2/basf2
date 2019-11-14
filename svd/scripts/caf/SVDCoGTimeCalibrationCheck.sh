#!/bin/bash

echo "run this script: source SVDCoGTimeCalibrationCheck.sh <exp> <input.txt path> <database.txt path> <log files path>"

exp_number=${1}
inputList=${2}
localDBFolder=${3}
logfiles=${4}

if [ ! -d ${logfiles}  ]; then
    echo "WARNING!"
    echo "the output folder ${logfiles} does not exist. Create it before launching simulation."
    echo "mkdir ${logfiles}"
    mkdir ${logfiles}
fi


i=0
for file in ${inputList}/*.txt;
do
  # local IFS="/"
  # read -ra ADDR <<< "${file}"
  run_number=$(echo ${file} | sed 's/[^0-9]*//g')
  echo "RUN: ${run_number} EXP: ${exp_number} FILE: ${file}" 
  
  bsub -q s -oo ${logfiles}/log_cog_${run_number}.txt basf2 SVDCoGTimeCalibrationCheck.py ${localDBFolder} ${file} ${run_number} ${exp_number}

  echo "bsub -q s -oo ${logfiles}/log_cog_${run_number}.txt basf2 SVDCoGTimeCalibrationCheck.py ${localDBFolder} ${file} ${run_number} ${exp_number}" 
  (( i += 1))
done
