#!/bin/bash

# Code

function setupEnvironment {
  local basf2path="$1"

  echo "setupEnvironment received following arguments:"
  echo "$basf2path"

  echo "Setup cluster environment"
  source /opt/ogs/ekpcluster/common/settings.sh
  
  echo "Setup belle2 environment"
  cd "$basf2path"
  source tools/setup_belle2
  cd -
  cd "$basf2path"/basf2
  setuprel
  setoption opt
  cd -
}

function setupGlobalDirectories {
  local collectionDirectory="$1"
  local jobDirectory="$2"
  
  echo "setupGlobalDirectories received following arguments:"
  echo "$collectionDirectory"
  echo "$jobDirectory"
  
  echo "Creating collection directory"
  mkdir -p "$collectionDirectory"
  cp "$steeringFile" "$collectionDirectory"/basf2_steering_file.py

  echo "Clean reschedule counter"
  rm -f "$collectionDirectory"/basf2_reschedule_counter
  touch "$collectionDirectory"/basf2_reschedule_counter

  echo "(Re)Creating job directory"
  rm -rf "$jobDirectory"
  mkdir -p "$jobDirectory"
  
}

function runGlobalBasf2 {
  local collectionDirectory="$1"
  local steeringFile="$2"
  
  echo "runGlobalBasf2 received following arguments:"
  echo "$collectionDirectory"
  echo "$steeringFile"
  
  echo "Run basf2 in collection directory"
  cd "$collectionDirectory"
  basf2 basf2_steering_file.py --dump-path basf2_path.pickle -- -ve -nproc 1
  local result=$?
  cd -
  echo "Result of runGlobalBasf2 is " "$result"
  return result
}


function createJobDirectory {
  local collectionDirectory="$1"
  local jobDirectory="$2"
  local jobNumber="$3"
  
  echo "createJobDirectory received following arguments:"
  echo "$collectionDirectory"
  echo "$jobDirectory"
  echo "$jobNumber"

  echo "Removing old job directory"
  rm -rf "$jobDirectory"/"$jobNumber"

  echo "Create new job directory"
  mkdir -p "$jobDirectory"/"$jobNumber"

  echo "Create symlinks to all files in collection directory"
  #for file in "$collectionDirectory"/*
  for file in "$collectionDirectory"/weights "$collectionDirectory"/basf2_path.pickle "$collectionDirectory"/*.config
  do
    local absolutePath=$(readlink -f "$file")
    ln -s "$absolutePath" "$jobDirectory"/"$jobNumber"/
  done
}

function setupJobDirectory {
  local collectionDirectory="$1"
  local jobDirectory="$2"
  local jobNumber="$3"
  #local mcFiles="$4"
  
  echo "setupJobDirectory received following arguments:"
  echo "$collectionDirectory"
  echo "$jobDirectory"
  echo "$jobNumber"
  echo "${mcFiles[@]}"

  createJobDirectory "$collectionDirectory" "$jobDirectory" "$jobNumber"

  echo "Creating basf2 script"
  cd "$jobDirectory"/"$jobNumber"
  echo -n "basf2 -l warning --execute-path basf2_path.pickle" > basf2_script.sh
  for mcFile in "${mcFiles[@]}"
  do
    echo -n " -i " "$mcFile" >> basf2_script.sh
  done
  echo " && touch basf2_finished_successfully" >> basf2_script.sh
  chmod +x basf2_script.sh
  cd -
}

function runJobBasf2 {
  local collectionDirectory="$1"
  local jobDirectory="$2"
  local jobNumber="$3"
  
  echo "runJobBasf2 received following arguments:"
  echo "$collectionDirectory"
  echo "$jobDirectory"
  echo "$jobNumber"

  cd "$jobDirectory"/"$jobNumber"
  qsub -cwd -q medium,long,nether -e error.log -o output.log -V basf2_script.sh | cut -f 3 -d ' ' > basf2_jobid
  cd -
}

function scheduleJob {
  local collectionDirectory="$1"
  local jobDirectory="$2"
  local jobNumber="$3"
  #local mcFiles="$4" # Passed implicitly because bash cannot pass arrays
  
  echo "scheduleJob received following arguments:"
  echo "$collectionDirectory"
  echo "$jobDirectory"
  echo "$jobNumber"
  echo "${mcFiles[@]}"

  setupJobDirectory "$collectionDirectory" "$jobDirectory" "$jobNumber"
  runJobBasf2 "$collectionDirectory" "$jobDirectory" "$jobNumber"
}

function scheduleJobs {
  local collectionDirectory="$1"
  local jobDirectory="$2"
  local nJobs="$3"
  #local allMcFiles="$4" # Passed implicitly because bash cannot pass arrays
  
  echo "scheduleJobs received following arguments:"
  echo "$collectionDirectory"
  echo "$jobDirectory"
  echo "$nJobs"
  echo "${allMcFiles[@]}"

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
    scheduleJob "$collectionDirectory" "$jobDirectory" "$i"
  done
}

function resetupJobDirectory {
  local collectionDirectory="$1"
  local jobDirectory="$2"
  local jobNumber="$3"
  
  echo "resetupJobDirectory received following arguments:"
  echo "$collectionDirectory"
  echo "$jobDirectory"
  echo "$jobNumber"

  echo "Saving basf2_script and logs"
  mkdir -p "$collectionDirectory"/backup/
  cp "$jobDirectory"/"$jobNumber"/basf2_script.sh "$collectionDirectory"/backup/
  cp "$jobDirectory"/"$jobNumber"/output.log "$collectionDirectory"/backup/
  cp "$jobDirectory"/"$jobNumber"/error.log "$collectionDirectory"/backup/

  createJobDirectory "$collectionDirectory" "$jobDirectory" "$jobNumber"

  echo "Restoring basf2_script and logs"
  cp "$collectionDirectory"/backup/basf2_script.sh "$jobDirectory"/"$jobNumber"/
  cp "$collectionDirectory"/backup/error.log "$jobDirectory"/"$jobNumber"/
  cp "$collectionDirectory"/backup/output.log "$jobDirectory"/"$jobNumber"/

}

function rescheduleJob {
  local collectionDirectory="$1"
  local jobDirectory="$2"
  local jobNumber="$3"
  
  echo "rescheduleJob received following arguments:"
  echo "$collectionDirectory"
  echo "$jobDirectory"
  echo "$jobNumber"

  resetupJobDirectory "$collectionDirectory" "$jobDirectory" "$jobNumber"
  runJobBasf2 "$collectionDirectory" "$jobDirectory" "$jobNumber"
  echo "Rescheduled job " "$jobNumber" >> "$collectionDirectory"/basf2_reschedule_counter
}


function checkJob {
  local collectionDirectory="$1"
  local jobDirectory="$2"
  local jobNumber="$3"
  
  echo "checkJob received following arguments:"
  echo "$collectionDirectory"
  echo "$jobDirectory"
  echo "$jobNumber"
  echo $(pwd)

  local jobID=$(cat "$jobDirectory"/"$jobNumber"/basf2_jobid)
  qstat -j "$jobID"
  local jobStat=$?

  if [ "$jobStat" -eq 0 ]
  then
    echo "Job not finished yet"
  else
    if [ -f "$jobDirectory"/"$jobNumber"/basf2_finished_successfully ]
    then
      echo "Job finished during check"
    else
      echo "Job is finished, but was not successful! Rescheduling job."
      rescheduleJob "$collectionDirectory" "$jobDirectory" "$jobNumber"
    fi
  fi

}

function checkJobs {
  local collectionDirectory="$1"
  local jobDirectory="$2"
  
  echo "checkJobs received following arguments:"
  echo "$collectionDirectory"
  echo "$jobDirectory"
 
  local finished_jobs=0
  local total_jobs=0

  for job in "$jobDirectory"/*
  do
    local jobNumber=$(basename "$job")

    if [ ! -f "$jobDirectory"/"$jobNumber"/basf2_finished_successfully ]
    then
      checkJob "$collectionDirectory" "$jobDirectory" "$jobNumber"
    else
      ((finished_jobs = finished_jobs + 1))
    fi
    ((total_jobs = total_jobs + 1))
  done

  if [ "$total_jobs" -ne "$finished_jobs" ]
  then
    echo "Some jobs are not finished yet"
    return 0
  fi

  echo "All Jobs finished"
  return 1

}

function mergeRootFiles {
  local collectionDirectory="$1"
  local jobDirectory="$2"
  
  echo "mergeRootFiles received following arguments:"
  echo "$collectionDirectory"
  echo "$jobDirectory"
 

  echo "remove symlinks"
  find $jobDirectory -maxdepth 2 -type l -delete

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


function main {
  local collectionDirectory="$1"
  local jobDirectory="$2"
  local steeringFile="$3"
  local nJobs="$4"
  #local allMcFiles="$4"
  
  echo "main received following arguments:"
  echo "$collectionDirectory"
  echo "$jobDirectory"
  echo "$steeringFile"
  echo "$nJobs"
  echo "${allMcFiles[@]}"

  setupGlobalDirectories "$collectionDirectory" "$jobDirectory"
  runGlobalBasf2  "$collectionDirectory" "$steeringFile"
  if [ ! $? ]
  then
    echo "Failed to run global basf2 to create pickled path"
    exit 1
  fi
  scheduleJobs "$collectionDirectory" "$jobDirectory" "$nJobs"

  sleep 300

  checkJobs "$collectionDirectory" "$jobDirectory"
  while [ $? -eq 0 ]
  do
    if [ $(cat "$collectionDirectory"/basf2_reschedule_counter | wc -l) -gt 10 ]
    then
      echo "Too many jobs fail, I won't suffer this no more! Goodbye"
      exit 1
    fi
    sleep 300
    checkJobs "$collectionDirectory" "$jobDirectory"
  done

  mergeRootFiles "$collectionDirectory" "$jobDirectory"
}

allMcFiles=( "$@" )
setupEnvironment "/storage/6/tkeck/"
main "FEI" "jobs" "basf2_steering_file.py" "1000"

