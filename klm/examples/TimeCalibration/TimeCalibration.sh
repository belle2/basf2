#!/bin/bash
# This file provides an example of how to call KLM time calibration examples.
DIR=${BELLE2_LOCAL_DIR}/klm/examples/TimeCalibration
NEVENTS=1000
NFILES=500
SUBMISSION="bsub -qs"
mkdir time_calibration
for ((i = 0; i < ${NFILES}; i++)); do
  ${SUBMISSION} basf2 -n ${NEVENTS} ${DIR}/Generation.py time_calibration/gen_${i}.root
done
for ((i = 0; i < ${NFILES}; i++)); do
  ${SUBMISSION} basf2 ${DIR}/Simulation.py time_calibration/gen_${i}.root time_calibration/sim_${i}.root
done
basf2 ${DIR}/Calibration.py ${PWD}/time_calibration/sim_*.root
