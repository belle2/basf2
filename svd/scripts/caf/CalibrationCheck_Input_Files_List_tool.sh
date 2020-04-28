#!/bin/bash

echo "run this scipt: source CalibrationCheck_Input_Files_List_tool.sh <name of the folder that will cointain the txt output files>"

OUTPUTLIST=${1}

echo "###############################################"
echo " "
echo "Creating folder for calibration input file list"
echo " "
echo "###############################################"
if [ ! -d ${OUTPUTLIST}  ]; then
    echo "WARNING!"
    echo "the output folder ${OUTPUTLIST} does not exist. Create it before launching simulation."
    echo "mkdir ${OUTPUTLIST}"
    mkdir ${OUTPUTLIST}
fi

if [ -f "file.list" ]; then
  rm -rf file.list
fi

i=0
#for folder in /group/belle2/dataprod/Data/release-04-00-02/DB00000748/proc10/e0008/4S/r*;
for folder in /group/belle2/dataprod/dp_managers/ddossett/airflow/output_data/43/hlt_hadron/e10/r*;
#/group/belle2/dataprod/dp_managers/ddossett/airflow/output_data/43/hlt_hadron/e10/r3605/sub00
do
  IFS='/' read -r -a array <<< "${folder}"
  run="${array[@]: -1:1}"
  run_number=$(echo ${run} | sed 's/[^0-9]*//g')
  # echo "RUN: ${run_number}" 
  if [ -f "eachrun_${run_number}.txt" ]; then
    rm -rf eachrun_${run_number}.txt
  fi
  #if [ -d "${folder}/skim/hlt_hadron/cdst/sub00/" ]; then
  if [ -d "${folder}/sub00/" ]; then
    j=0
    #for f in ${folder}/skim/hlt_hadron/cdst/sub00/*.root;
    for f in ${folder}/sub00/*.root;
    do
      ls ${f} >> ${OUTPUTLIST}/eachrun_${run_number}.txt
      (( j += 1 ))
    done
  fi
  #cdst.physics.0010.03605.HLT0.hlt_hadron.f00000.root
  #file="${folder}/skim/hlt_hadron/cdst/sub00/cdst.physics.0008.${run_number}.HLT*"
  #file="${folder}/sub00/cdst.physics.0008.${run_number}.HLT*"
  file="${folder}/sub00/cdst.physics.0010.${run_number}.HLT*"
  echo "FILE: ${file}"
  #if [ -d "${folder}/skim/hlt_hadron/cdst/sub00/" ]; then
  if [ -d "${folder}/sub00/" ]; then
    echo "${file}" >> ${OUTPUTLIST}/file.list  
  fi
  (( i += 1))
done
