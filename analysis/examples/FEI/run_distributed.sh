#!/usr/bin/env bash
# Trains FEI on a provided input sample, running on KEKCC cluster

#abort on error
set -e


################################################################################
# User settings
# You'll need to change most of the paths to point to your own directories.
# Some temporary files must stay on /home for technical reasons (otherwise your jobs
# will be killed), while larger files will be moved to HSM (/ghi/...). Members of the
# belle2 group can create their own directories inside /ghi/fs01/belle2/bdata/users.

#split input data into this many jobs
nJobs="2000"
#steering file to run
steeringFile="/home/belle/pulver/basf2/analysis/examples/FEI/B_generic.py"

#collects training output, plots, etc.
#note: MUST be on home, not HSM (larger files only stored temporarily)
collectionDirectory="/home/belle/pulver/B_generic/collection"

#contains working directories for jobs
#note: MUST be on home, not HSM (larger files only stored temporarily)
jobDirectory="/home/belle/pulver/B_generic/jobs"

#use HSM for large amounts of data (.root files from training, ntuples)
collectionDirectoryReal="/ghi/fs01/belle2/bdata/users/pulver/B_generic/collection"
#cache intermediate information (probably needs at least as much space as input data, should be on HSM)
persistentDirectory="/ghi/fs01/belle2/bdata/users/pulver/B_generic/persistent"

#Data files to use as input:
#streams 0-39
#allMcFiles=(/hsm/belle2/bdata/MC/generic/{mixed,charged}/mcprod1405/BGx0/*_s{0{0..9},{10..39}}/*.root)
#10 independent streams
allMcFiles=(/hsm/belle2/bdata/MC/generic/{mixed,charged}/mcprod1405/BGx0/*_s{40..49}/*.root)
#all 50 streams
#allMcFiles=(/hsm/belle2/bdata/MC/generic/{mixed,charged}/mcprod1405/BGx1/*_s{0{0..9},{10..49}}/*.root)
################################################################################

#Reducing the numberof jobs if necessary
fileCount=0
for mcFile in ${allMcFiles[@]}
do
  fileCount=$((fileCount + 1))
done

if [ $fileCount -lt $nJobs ]; then
  echo "Less input files than number of jobs!"
  echo "Reducing numberof jobs to fit amount of files..."
  nJobs=$fileCount
  echo "Working with " $nJobs " jobs."
fi
################################################################################





source $BELLE2_LOCAL_DIR/analysis/examples/FEI/distributed_fei.sh

create_play

while [ ! -f $collectionDirectory/FEI_finished_its_training ]; do
  next_act
done
