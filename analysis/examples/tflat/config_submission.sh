#!/bin/bash

# Set Submission Parameters
export OUTPUT_PATH="tests" # Path where to store the output
export INPUT_DIRECTORY="/group/belle2/group/physics/TDCPV/TFlaT_Belle/Simulated_Samples_Test/" # Small Dataset for testing purposes
#export INPUT_DIRECTORY="/group/belle2/group/physics/TDCPV/TFlaT_Belle/Simulated_Samples/" # Entire Dataset of produced mdst files

#export BASF2_VERSION="light-2601-hyperion" # Desired basf2 version
export BASF2_VERSION="" # Use local developments version
export BELLE="True" # Run over Belle Samples
#export BELLE="False" # Run over Belle II Samples

export UNIQUEIDENTIFIER="TFlaT_Belle_light_2601_hyperion" # .yaml config file for Belle TFLaT Training
#export UNIQUEIDENTIFIER="TFlaT_MC16rd_light_2601_hyperion" #.yaml config file for Belle II TFLaT Training

export CHUNKSIZE=1 # Amount of files to be processed per job
export WORKERS=100 # Amount of Worker nodes requested for the job processing

# basf2 environment setup
if [[ "$BASF2_VERSION" == "" ]]; then
   echo "Using basf2 development version"
else
   source /cvmfs/belle.cern.ch/tools/b2setup "$BASF2_VERSION"
fi

# For development purposes
export BELLE2_LOCAL_DIR=/home/belle2/benjamin/development
