#!/bin/bash
set -e

module () {
	eval `/usr/bin/modulecmd bash $*`
}

# This is wrapper to simplify usage of basf2 with grid-control jobs
export BELLE2_NO_TOOLS_CHECK=1

echo "Setting up basf2 tools from ${BASF2_TOOLS_LOCATION}"
source ${BASF2_TOOLS_LOCATION}/setup_belle2

echo "Setting up basf2 release from ${BASF2_RELEASE_LOCATION}"
cd ${BASF2_RELEASE_LOCATION}
setuprel
setoption ${BASF2_COMPILE_OPTION}

# back to the jobs scratch folder
cd ${GC_SCRATCH}

# this will ouput the number of event in the input file
# useful for later analysis
b2file-metadata-show ${FILE_NAMES}

echo "Starting steering file..."
basf2_command="basf2 ${BASF2_STEERING_FILE} -- --input-file ${FILE_NAMES} --no-output --histo-output-file dqm_out.root --root-output-file output.root --local-db-path ${BASF2_LOCAL_DB_PATH} input_buffer output_buffer 2222 0"

if [ -z "${BASF2_USE_GDB}" ]; then
	eval_command=$basf2_command
else
	eval_command="gdb -return-child-result -batch -ex 'run' -ex 'backtrace' --args ${basf2_command}"
fi
echo "Executing: ${eval_command}"
eval $eval_command
