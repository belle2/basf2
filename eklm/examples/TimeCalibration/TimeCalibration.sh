#!/bin/bash
# This file provides an example of how to call EKLM time calibration examples.
# Note that for actual time calibration studies, a larger amount of data
# is necessary.
DIR=${BELLE2_LOCAL_DIR}/eklm/examples/TimeCalibration
mkdir time_calibration
basf2 ${DIR}/Generation.py time_calibration/gen.root
basf2 ${DIR}/Simulation.py time_calibration/gen.root time_calibration/sim.root
basf2 ${DIR}/Reconstruction.py time_calibration/sim.root time_calibration/rec.root
basf2 ${DIR}/Calibration.py time_calibration/rec.root

