#!/bin/bash
# This file provides the execution of basf2 scripts for creating several calibration
# DSTs ready for use by the CAF example scripts.
# They don't include reconstruction steps so they are at least somewhat close to DSTs
# as they will appear in real data.

#################################################################################################
####                                            WARNING                                      ####
# You won't be able to use these DSTs with many of the real calibration algorithms directly as  #
# the number of events is just too low. Rather than include a script which by default generates #
# enormous files, the event numbers here are small. Just to get the test scripts working at all.#
# You should use your own scripts and numbers of events to generate sensible dataset sizes for  #
# yourself.                                                                                     #
####                                                                                         ####
#################################################################################################
# You should be setting a directory where you want the output to be stored (it will be created if
# it doesn't exist)
[ $# -eq 0 ] && { echo "Usage: $0 <data directory>"; exit 1; }

# Define and create directories to store the output.
EXAMPLE_DIR=$1
mkdir -p ${EXAMPLE_DIR}
pushd ${EXAMPLE_DIR}
# This creates cosmic track events usually used for VXD alignment (they are nice and quick to
# generate) for a few runs.
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/1_generate.py 1 1 500 1
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/1_generate.py 1 2 500 1
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/1_generate.py 1 3 500 1
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/1_generate.py 1 4 500 1
popd > /dev/null
