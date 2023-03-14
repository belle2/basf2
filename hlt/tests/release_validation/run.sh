#!/bin/bash

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

echo $@ | grep -E -- "(^|\\s)(--help|-h)($|\\s)" &>/dev/null
if [ $? -eq 0 ]; then
    cat <<EOT
This script runs the (semi-)automated online release validation which will make sure
the release can run on the real online systems (HLT and ExpressReco) with raw data
from the detector recorded in a previous run.

It has two optional arguments which are the stage of the validation to run and output
directory for the validation output. If not given it will run the automatic part and
put the output in "${STAGE}_validation-${BELLE2_RELEASE:-head}".

    `basename $0` [<stage>] [<output_directory>]

It will use as many cores as present on the system a provided by the nproc utility
(nproc=$(nproc)).

By default, the root files produced during the validation are removed at the end of
the validation. If the root files must be kept for further studies, please set
the environment variable "BELLE2_RELEASE_VALIDATION_KEEP_ROOT" to any values, e.g.:

    export BELLE2_RELEASE_VALIDATION_KEEP_ROOT=1
EOT
    exit 1;
fi

# unset DISPLAY to prevent crashes when ROOT tries to open canvases
unset DISPLAY

# make sure we have a release directory
if [ -z "$BELLE2_RELEASE_DIR" ]; then
    if [ ! -z "$BELLE2_LOCAL_DIR" ]; then
        export BELLE2_RELEASE_DIR=$BELLE2_LOCAL_DIR
    else
        echo "\$BELLE2_RELEASE_DIR not set, please run b2setup" >&2
        exit 1
    fi
fi

# make sure the validation data dir exists
if [ -z "$BELLE2_VALIDATION_DATA_DIR" ]; then
    echo "\$BELLE2_VALIDATION_DATA_DIR not set, cannot find validation data" >&2
    exit 1
fi
if [ ! -d "$BELLE2_VALIDATION_DATA_DIR" ]; then
    echo "\$BELLE2_VALIDATION_DATA_DIR doesn't exist, did you install the validation data with b2install-data?" >&2
    exit 1
fi

# take settings from validation data dir unless all are already set to allow overriding which file to use in bamboo
if [ -z "$VALIDATION_RAWDATA" ] || [ -z "$VALIDATION_EXP" ] || [ -z "$VALIDATION_RUN" ]; then
    if [ ! -f "${BELLE2_VALIDATION_DATA_DIR}/hltvalidation/info.sh" ]; then
        echo "no info.sh in \${BELLE2_VALIDATION_DATA_DIR}/hltvalidation, cannot continue"
        exit 1
    fi
    source ${BELLE2_VALIDATION_DATA_DIR}/hltvalidation/info.sh
fi

# make the file path absolute ...
export VALIDATION_RAWDATA="${BELLE2_VALIDATION_DATA_DIR}/hltvalidation/${VALIDATION_RAWDATA}"

# make sure we have the rawdata
if [ ! -f "$VALIDATION_RAWDATA" ]; then
    echo "Validation data ${RAWDATA} doesn't exist. Have you installed the validation data with b2install-data?" >&2
    exit 1
fi

# set arguments
STAGE=${1:-automatic}
if [[ $STAGE != "automatic" && $STAGE != "manual" ]]; then
    echo "First argument 'stage' needs to be either automatic or manual" >&2
    exit 1
fi
export RELEASE=${BELLE2_RELEASE:-head}
export NCPUS=$(nproc)
export OUTPUT_DIR=$(realpath ${2:-${STAGE}-validation-${RELEASE}})
export OUTPUT_ROOT_DIR=`mktemp -d tmp_XXXXX`

# yes, from here on out we want to exit on every error
set -e
# and in case of pipes we want the last (rightmost) command to exit with a non-zero status
set -o pipefail
# and make sure we replace non-matching patterns with null instead of keeping them unexpanded.
shopt -s nullglob

# delete the output root files unless specified
function cleanup() {
    echo "Deleting the output root files"
    rm -rf ${OUTPUT_ROOT_DIR}
}
if [ -z "$BELLE2_RELEASE_VALIDATION_KEEP_ROOT" ]; then
    trap cleanup EXIT
else
    echo "The output root files will be stored in ${OUTPUT_ROOT_DIR}"
fi

# setup the output directory
mkdir -p ${OUTPUT_DIR}

# save the environment variables we have
env > ${OUTPUT_DIR}/env.log

function run_stage() {
    for file in $BELLE2_RELEASE_DIR/hlt/tests/release_validation/$1-*; do
        if [[ -f "$file" && -x $(realpath "$file") ]]; then
            echo "Executing $(basename $file)"
            $file 2>&1 | tee $OUTPUT_DIR/$(basename ${file%.*}).log
        elif [[ -f "$file" && "$file" == *.py ]]; then
            echo "Running 'basf2 $(basename $file)'"
            basf2 -- $file 2>&1 | tee $OUTPUT_DIR/$(basename ${file%.*}).log
        fi
    done
}

# always run the setup first
run_stage setup

# and then the real stage
run_stage $STAGE
