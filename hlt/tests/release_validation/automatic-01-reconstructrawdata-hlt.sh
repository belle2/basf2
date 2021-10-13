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
PARENT="b2code-memoryusage -i 0.1 -m record -p ${OUTPUT_DIR}/beam_reco_monitor-hlt-memory.npz --"

# ... but change the settings if we run in debug mode
if [ "$1" = "debug" ]; then
    PROCESSES="-p0 -n100"
    PARENT="gdb --args"
fi

# Prepare the input file: the ROIs objects must be removed for the HLT validation
INPUT_FILE_BASENAME=`basename ${VALIDATION_RAWDATA} .root`
INPUT_FILE=${OUTPUT_ROOT_DIR}/${INPUT_FILE_BASENAME}-hlt.root
b2file-mix -n 100 --exclude ROIs --keep-eventinfo -o ${INPUT_FILE} ${VALIDATION_RAWDATA}

# Run the HLT script
${PARENT} basf2 ${PROCESSES} \
    ${BELLE2_RELEASE_DIR}/hlt/operation/phase3/global/hlt/evp_scripts/beam_reco_monitor.py -- \
    UNUSED UNUSED 6666 \
    --input-file ${INPUT_FILE} \
    --output-file ${OUTPUT_ROOT_DIR}/beam_reco_monitor-hlt-out.root \
    --histo-output-file ${OUTPUT_ROOT_DIR}/beam_reco_monitor-hlt-dqm.root \
    --local-db-path cdb
