#!/bin/bash
# This file provides an example of how to call EKLM time calibration examples.
# Note that for actual time calibration studies, a larger amount of data
# is necessary.
DIR=${BELLE2_LOCAL_DIR}/eklm/examples
mkdir time_calibration
basf2 ${DIR}/EKLMTimeCalibrationGeneration.py time_calibration/gen.root
basf2 ${DIR}/EKLMTimeCalibrationSimulation.py time_calibration/gen.root time_calibration/sim.root
basf2 ${DIR}/EKLMTimeCalibrationReconstruction.py time_calibration/sim.root time_calibration/rec.root
basf2 ${DIR}/EKLMTimeCalibrationCalibration.py time_calibration/rec.root

