#!/bin/bash

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Set number of processes and make sure we profile the execution
PROCESSES="-p ${NCPUS}"
PARENT="b2code-memoryusage -i 0.1 -m record -p ${OUTPUT_DIR}/beam_reco_monitor-expressreco-memory.npz --"

# ... but change the settings if we run in debug mode
if [ "$1" = "debug" ]; then
    PROCESSES="-p0 -n100"
    PARENT="gdb --args"
fi

# Prepare the input file: we want to run just a quick validation for the ExpressReco,
# since at this stage we know that HLT works fine
INPUT_FILE_BASENAME=`basename ${VALIDATION_RAWDATA} .root`
INPUT_FILE=${OUTPUT_ROOT_DIR}/${INPUT_FILE_BASENAME}-expressreco.root
b2file-mix -n 100 --keep-eventinfo -o ${INPUT_FILE} ${VALIDATION_RAWDATA}

# Run the ExpressReco script
${PARENT} basf2 ${PROCESSES} \
    ${BELLE2_RELEASE_DIR}/hlt/operation/phase3/global/expressreco/evp_scripts/beam_reco.py -- \
    UNUSED UNUSED 6666 \
    --input-file ${INPUT_FILE} \
    --output-file ${OUTPUT_ROOT_DIR}/beam_reco_monitor-expressreco-out.root \
    --histo-output-file ${OUTPUT_ROOT_DIR}/beam_reco_monitor-expressreco-dqm.root \
    --local-db-path cdb
