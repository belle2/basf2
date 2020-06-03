#!/bin/bash
# The root file has to be cleared from ROI objects first.
RAWDATA="${BELLE2_VALIDATION_DATA_DIR}/hltvalidation/physics.0012.02750.HLT1.f00000.root"
if [ ! -f "$RAWDATA" ]; then
    echo "Validation data ${RAWDATA} doesn't exist. Have you installed the validation data with b2install-data?" >&2
    exit 1
fi

b2code-memoryusage -m record -p ${OUTPUT_DIR}/bream_reco_monitor-memory.npz -- basf2 -p ${NCPUS} \
    ${BELLE2_RELEASE_DIR}/hlt/operation/phase3/global/hlt/evp_scripts/beam_reco_monitor.py -- \
    UNUSED UNUSED 6666 \
    --input-file ${RAWDATA} \
    --output-file beam_reco_monitor-out.root \
    --histo-output-file beam_reco_monitor-dqm.root \
    --central-db-tag online
