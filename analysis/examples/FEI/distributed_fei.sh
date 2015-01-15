#Not actually an example file, sourced from run_distributed.sh

#set -a
#source /opt/ogs/ekpcluster/common/settings.sh
#set +a


function create_play {
  echo "Recreate directories"
  rm -rf "$persistentDirectory"
  mkdir -p "$persistentDirectory"
  rm -rf "$collectionDirectory"
  rm -rf "$collectionDirectoryReal"
  mkdir -p "$collectionDirectory"
  mkdir -p "$collectionDirectoryReal"
  rm -rf "$jobDirectory"
  mkdir -p "$jobDirectory"

  echo "Create persistent job files"
  for i in $(seq "$nJobs")
  do
    echo "Creating setup for job $i..."
    unset mcFiles
    declare -a mcFiles
    local count=0
    for mcFile in ${allMcFiles[@]}
    do
        #echo "test: $mcFile"
      count=$((count + 1))
      if [ $((count % nJobs)) -eq $((i-1)) ]
      then
        mcFiles[count]="$mcFile"
      fi
    done
    mkdir -p  "$jobDirectory"/"$i"
    mkdir -p  "$persistentDirectory"/"$i"
    cd "$persistentDirectory"/"$i"
    #hadd basf2_input.root "${mcFiles[@]}"
    for mcFile in ${mcFiles[@]}
    do
        basename=`basename "$mcFile"`
        ln -s "$mcFile" "rawinput_$basename"
    done
    cd - > /dev/null
  done
}

function create_ntuples_with_finished_training {
  
  echo "Create persistent job files"
  for i in $(seq "$nJobs")
  do
    echo "Creating setup for job $i..."
    unset mcFiles
    declare -a mcFiles
    local count=0
    for mcFile in ${allMcFiles[@]}
    do
        #echo "test: $mcFile"
      count=$((count + 1))
      if [ $((count % nJobs)) -eq $((i-1)) ]
      then
        mcFiles[count]="$mcFile"
      fi
    done
    mkdir -p  "$jobDirectory"/"$i"
    mkdir -p  "$persistentDirectory"/"$i"
    cd "$persistentDirectory"/"$i"
    for mcFile in ${mcFiles[@]}
    do
        basename=`basename "$mcFile"`
        ln -s "$mcFile" "rawinput_$basename"
    done
    cd - > /dev/null

    cd "$jobDirectory"/"$i"
    echo -n "time basf2 -l warning --execute-path basf2_path.pickle " > basf2_script.sh
    echo -n "-i $persistentDirectory/$i/rawinput_\*.root " >> basf2_script.sh
    echo -n "-o $persistentDirectory/$i/basf2_output.root " >> basf2_script.sh
    #echo -n " &> /home/belle/pulver/joboutput/${i}_my_output_hack.log " >> basf2_script.sh
    echo -n " &> my_output_hack.log " >> basf2_script.sh
    echo -n "&& touch basf2_finished_successfully" >> basf2_script.sh
    chmod +x basf2_script.sh
    cd - > /dev/null
  done
    
}

function run_basf2 {
  cd "$collectionDirectory"
  basf2 $steeringFile --dump-path basf2_path.pickle -i "$persistentDirectory"/1/basf2_input.root -- -ve -nproc 20 -cache cache.pkl --preload -summary || return 1
  cd - > /dev/null
}

function setup_jobDirectory {
  for file in "$collectionDirectory"/weights "$collectionDirectory"/basf2_path.pickle "$collectionDirectory"/*.config
  do
    if [ -e "$file" ]
    then
      local absolutePath=$(readlink -f "$file")
      for i in $(seq "$nJobs")
      do
            mkdir -p  "$jobDirectory"/"$i"
            cd "$jobDirectory"/"$i"
            #here doc with no quotes to allow variable substitution
            cat << EOF > basf2_script.sh
#abort on error
set -e

if [ -f "$persistentDirectory/$i/basf2_input.root" ]; then
  INPUT="$persistentDirectory/$i/basf2_input.root"
else
  INPUT="$persistentDirectory/$i/rawinput_*.root"
fi
time basf2 -l warning --execute-path basf2_path.pickle -i "\$INPUT" -o $persistentDirectory/$i/basf2_output.root &> my_output_hack.log || touch basf2_job_error
for f in *.root;
do
  #ignore symlinks
  test -h "\$f" && continue

  mv "\$f" $persistentDirectory/$i/
  ln -s "$persistentDirectory/$i/\$f" .
done
touch basf2_finished_successfully
EOF
            chmod +x basf2_script.sh
            cd - > /dev/null

            ln -s "$absolutePath" "$jobDirectory"/"$i"/
      done
    fi
  done
}

function submit_jobs {
  
  for i in $(seq "$nJobs")
  do
    echo "submitting $i"
    cd "$jobDirectory"/"$i"
    #qsub -cwd -q express,short,medium,long -e error.log -o output.log -V basf2_script.sh | cut -f 3 -d ' ' > basf2_jobid
    #bsub -q s -e error.log -o output.log ./basf2_script.sh | cut -f 2 -d ' ' | sed 's/<//' | sed 's/>//' > basf2_jobid
    bsub -q s -e /home/belle/pulver/joboutput -o /home/belle/pulver/joboutput ./basf2_script.sh | cut -f 2 -d ' ' | sed 's/<//' | sed 's/>//' > basf2_jobid
    cd - > /dev/null
  done
}

function wait_for_jobs {

  while [ $(ls -l "$jobDirectory"/*/basf2_finished_successfully 2> /dev/null | wc -l ) -lt "$nJobs" ]
  do
    if [ $(ls -l "$jobDirectory"/*/basf2_job_error 2> /dev/null | wc -l) -ne 0 ]
    then
      echo "basf2 execution failed! Error occurred in:"
      ls -l "$jobDirectory"/*/basf2_job_error
      return 1
    fi
    echo -n "."
    sleep 40
  done
}

function merge_root_files {
  
  if [ $(ls "$jobDirectory"/1/*.root | wc -l) -eq 0 ]
  then
    echo "There are no root files to merge"
  else
    echo "Merging root files..."
    for file in "$jobDirectory"/1/*.root
    do
      file=$(basename "$file")
      rm -f "$collectionDirectoryReal"/"$file"
      fei_merge_files "$collectionDirectoryReal"/"$file" "$jobDirectory"/*/"$file" || return 1
      ln -s "$collectionDirectoryReal"/"$file" "$collectionDirectory"/"$file"
    done
  fi

}

function update_input_files {
  
  for i in $(seq "$nJobs")
  do
    cd "$persistentDirectory"/"$i"
    #use mv without alias (i.e. overwrite without warning)
    command mv basf2_output.root basf2_input.root
    cd - > /dev/null
  done

}

function clean_jobDirectory {
  
  find $jobDirectory -maxdepth 2 -type l -delete
  find $jobDirectory -maxdepth 2 -name basf2_finished_successfully -delete
  find $jobDirectory -maxdepth 2 -name error.log -delete
  find $jobDirectory -maxdepth 2 -name output.log -delete
  find $jobDirectory -maxdepth 2 -name my_output_hack.log -delete
  find $jobDirectory -maxdepth 2 -name *.root -delete
  
}

function next_act {
  
  run_basf2 || return 1
  echo "Finished basf2 here -- wait 3 seconds"
  sleep 3
  setup_jobDirectory
  submit_jobs
  wait_for_jobs || return 1
  echo "Finished basf2 cluster -- wait 3 seconds"
  sleep 3
  merge_root_files || return 1
  update_input_files
  clean_jobDirectory

}
