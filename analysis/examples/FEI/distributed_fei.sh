#!/usr/bin/env bash

collectionDirectory="FEI"
jobDirectory="jobs"
persistentDirectory="persistent"
nJobs="1000"
steeringFile="B_generic.py"

set -a
source /opt/ogs/ekpcluster/common/settings.sh
set +a


function create_play {

  local allMcFiles=( "$@" )
 
  echo "Recreate directories"
  rm -rf "$persistentDirectory"
  mkdir -p "$persistentDirectory"
  rm -rf "$collectionDirectory"
  mkdir -p "$collectionDirectory"
  rm -rf "$jobDirectory"
  mkdir -p "$jobDirectory"
  
  echo "Create persistent job files"
  for i in $(seq "$nJobs")
  do
    unset mcFiles
    declare -a mcFiles
    local count=0
    for mcFile in "${allMcFiles[@]}"
    do
      count=$((count + 1))
      if [ $((count % nJobs)) -eq $((i-1)) ]
      then
        mcFiles[count]="$mcFile"
      fi
    done
    mkdir -p  "$jobDirectory"/"$i"
    mkdir -p  "$persistentDirectory"/"$i"
    cd "$persistentDirectory"/"$i"
    hadd basf2_input.root "${mcFiles[@]}"
    cd -

    cd "$jobDirectory"/"$i"
    echo -n "basf2 -l warning --execute-path basf2_path.pickle " > basf2_script.sh
    echo -n "-i ../../$persistentDirectory/$i/basf2_input.root " >> basf2_script.sh
    echo -n "-o ../../$persistentDirectory/$i/basf2_output.root " >> basf2_script.sh
    echo -n "&& touch basf2_finished_successfully" >> basf2_script.sh
    cd -
  done
    
}

function run_basf2 {
  cd "$collectionDirectory"
  basf2 ../$steeringFile --dump-path basf2_path.pickle -- -ve -nproc 2 -preload -cache mycachefile.pkl
  #basf2 ../$steeringFile --dump-path basf2_path.pickle -- -ve -preload -cache mycachefile.pkl
  cd -
}

function setup_jobDirectory {
  for file in "$collectionDirectory"/weights "$collectionDirectory"/basf2_path.pickle "$collectionDirectory"/*.config
  do
    if [ -e "$file" ]
    then
      local absolutePath=$(readlink -f "$file")
      for i in $(seq "$nJobs")
      do
            ln -s "$absolutePath" "$jobDirectory"/"$i"/
      done
    fi
  done
}

function submit_jobs {
  
  for i in $(seq "$nJobs")
  do
    cd "$jobDirectory"/"$i"
    qsub -cwd -q express,short -e error.log -o output.log -V basf2_script.sh | cut -f 3 -d ' ' > basf2_jobid
    cd -
  done
}

function wait_for_jobs {

  while [ $(ls -l "$jobDirectory"/*/basf2_finished_successfully | wc -l) -lt "$nJobs" ]
  do
    sleep 30
  done
}

function merge_root_files {
  
  if [ $(ls "$jobDirectory"/1/*.root | wc -l) -eq 0 ]
  then
    echo "There are no root files to merge"
  else
    echo "Merge root files"
    for file in "$jobDirectory"/1/*.root
    do
      file=$(basename "$file")
      hadd "$collectionDirectory"/"$file" "$jobDirectory"/*/"$file"
    done
  fi

}

function update_input_files {
  
  for i in $(seq "$nJobs")
  do
    cd "$persistentDirectory"/"$i"
    mv basf2_output.root basf2_input.root
    cd -
  done

}

function clean_jobDirectory {
  
  find $jobDirectory -maxdepth 2 -type l -delete
  find $jobDirectory -maxdepth 2 -name basf2_finished_successfully -delete
  find $jobDirectory -maxdepth 2 -name *.root -delete
  
}

function next_act {
  
  run_basf2
  echo "Finished basf2 here -- wait 3 seconds"
  sleep 3
  setup_jobDirectory
  submit_jobs
  wait_for_jobs
  echo "Finished basf2 cluster -- wait 3 seconds"
  sleep 3
  merge_root_files
  update_input_files
  clean_jobDirectory

}
