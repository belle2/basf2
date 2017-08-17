#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# This steering file generates several events and
# does a simple dE/dx measurement with path length
# correction. The results are stored in a ROOT file.
#
# Usage: basf2 dedxPID_evtgen.py
#
# Input: None
# Output: dedxPID_evtgen.root
#
# Contributors: Jake Bennett
#
# Example steering file - 2011 Belle II Collaboration
#############################################################

from basf2 import *
from ROOT import Belle2

main = create_path()

use_local_database("calibration_results/CDCDedxRunGainCalibration/outputdb/database.txt",
                   "calibration_results/CDCDedxRunGainCalibration/outputdb")
use_local_database("calibration_results/CDCDedxWireGainCalibration/outputdb/database.txt",
                   "calibration_results/CDCDedxWireGainCalibration/outputdb")
use_local_database("calibration_results/CDCDedxCosineCalibration/outputdb/database.txt",
                   "calibration_results/CDCDedxCosineCalibration/outputdb")

input_file = 'B2Electrons.root'
main.add_module('RootInput', inputFileName=input_file)

main.add_module('DedxCorrection')

output_file = 'NewB2Electrons.root'
main.add_module('RootOutput', outputFileName=output_file)

process(main)
print(statistics)
