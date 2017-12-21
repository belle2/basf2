#!/bin/bash
set -e

# This is wrapper to simplify usage of basf2 with grid-control jobs
export BELLE2_NO_TOOLS_CHECK=1

echo "Setting up basf2 tools from ${BASF2_TOOLS_LOCATION}"
source ${BASF2_TOOLS_LOCATION}/setup_belle2.sh

echo "Setting up basf2 release from ${BASF2_RELEASE_LOCATION}"
cd ${BASF2_RELEASE_LOCATION}
setuprel
setoption ${BASF2_COMPILE_OPTION}

# back to the jobs scratch folder
cd ${GC_SCRATCH}
echo "Starting steering file..."
basf2 ${BASF2_STEERING_FILE}
