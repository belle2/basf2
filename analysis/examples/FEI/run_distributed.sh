#!/usr/bin/env bash
# Trains FEI on a provided input sample, running on KEKCC cluster

#abort on error
set -e


################################################################################
# User settings
nJobs="2000"
steeringFile="/home/belle/pulver/basf2/analysis/examples/FEI/B_generic.py"

#collects training output, plots, etc.
#note: MUST be on home, not HSM (larger files only stored temporarily)
collectionDirectory="/home/belle/pulver/B_generic/collection"

#contains working directories for jobs
#note: MUST be on home, not HSM (larger files only stored temporarily)
jobDirectory="/home/belle/pulver/B_generic/jobs"

#use HSM for large amounts of data (.root files from training, ntuples)
collectionDirectoryReal="/ghi/fs01/belle2/bdata/pulver/B_generic_daughter0mass/collection"
#cache intermediate information (probably needs at least as much space as input data, should be on HSM)
persistentDirectory="/ghi/fs01/belle2/bdata/pulver/B_generic/persistent"

#Data files to use as input:
#streams 0-39
#allMcFiles=(/group/belle2/MC/generic/{mixed,charged,ccbar,ssbar,ddbar,uubar}/mcprod1405/BGx0/*_s{0{0..9},{10..39}}/*.root)
#10 independent streams
allMcFiles=(/group/belle2/MC/generic/{mixed,charged,ccbar,ssbar,ddbar,uubar}/mcprod1405/BGx0/*_s{40..49}/*.root)
#all 50 streams
#allMcFiles=(/group/belle2/MC/generic/{mixed,charged,ccbar,ssbar,ddbar,uubar}/mcprod1405/BGx1/*_s{0{0..9},{10..49}}/*.root)
################################################################################



source $BELLE2_LOCAL_DIR/analysis/examples/FEI/distributed_fei.sh

create_play

while [ ! -f $collectionDirectory/FEIsummary.tex ]; do
  next_act
done
