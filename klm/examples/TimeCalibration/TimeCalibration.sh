#!/bin/bash
# This file provides an example of how to call KLM time calibration examples.
DIR=${BELLE2_LOCAL_DIR}/klm/examples/TimeCalibration
mkdir time_calibration
basf2 -n 100000 ${DIR}/Generation.py time_calibration/gen.root
basf2 ${DIR}/Simulation.py time_calibration/gen.root time_calibration/sim.root
basf2 ${DIR}/Calibration.py ${PWD}/time_calibration/sim.root
