#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
#
# Test script to produce DQM plots for CDC dE/dx
#
# Usage: basf2 runCDCDedxDQM.py
#
# Input: B2Electrons.root (from calib1_generate.py),
#        calibration_results (from calib2_calibrate.py)
# Output: CDCDedxDQM.root
#
#
# Example steering file - 2011 Belle II Collaboration
#############################################################

import os
from basf2 import *

# use the calibration results from calib2_calibrate.py
use_local_database("calibration_results/CDCDedxRunGainCalibration/outputdb/database.txt",
                   "calibration_results/CDCDedxRunGainCalibration/outputdb")
use_local_database("calibration_results/CDCDedxWireGainCalibration/outputdb/database.txt",
                   "calibration_results/CDCDedxWireGainCalibration/outputdb")
use_local_database("calibration_results/CDCDedxCosineCalibration/outputdb/database.txt",
                   "calibration_results/CDCDedxCosineCalibration/outputdb")

# main path
main = create_path()

# read in a sample
input_file = 'B2Electrons.root'
main.add_module("RootInput", inputFileName=input_file)

# register the HistoManager and specify output file
main.add_module("HistoManager", histoFileName="CDCDedxDQM.root")

# run the DQM module
main.add_module("CDCDedxDQM")

# Process events
process(main)
