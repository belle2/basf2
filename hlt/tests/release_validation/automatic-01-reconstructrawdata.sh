#!/bin/bash
# The root file has to be cleared from ROI objects first.

PROCESSES="-p ${NCPUS}"
PARENT="b2code-memoryusage -i 0.1 -m record -p ${OUTPUT_DIR}/beam_reco_monitor-memory.npz --"

if [ "$1" = "debug" ]; then
    PROCESSES="-p0 -n100"
    PARENT="gdb --args"
fi

${PARENT} basf2 ${PROCESSES} \
    ${BELLE2_RELEASE_DIR}/hlt/operation/phase3/global/hlt/evp_scripts/beam_reco_monitor.py -- \
    UNUSED UNUSED 6666 \
    --input-file ${VALIDATION_RAWDATA} \
    --output-file beam_reco_monitor-out.root \
    --histo-output-file ./beam_reco_monitor-dqm.root \
    --local-db-path cdb
