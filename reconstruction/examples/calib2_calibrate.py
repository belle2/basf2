#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# 2 - calibrate
#
# Run the dE/dx calibration with the CAF:
#  - collect the samples with the collector modules
#  - run the calibrations with the given algorithms
#  - produce output database files
#
# Usage: basf2 firstDedxAttempt.py
#
# Input: B2Electrons.root (from calib1_generate.py)
# Output: calibration_results
#
# Contributors: Jake Bennett
#
# Example steering file - 2017 Belle II Collaboration
#############################################################

from basf2 import *
import os
import sys
import ROOT
from ROOT.Belle2 import CDCDedxWireGainAlgorithm, CDCDedxRunGainAlgorithm, CDCDedxCosineAlgorithm
from ROOT.Belle2 import CDCDedx2DCorrectionAlgorithm, CDCDedx1DCleanupAlgorithm
from caf.framework import Calibration, CAF

ROOT.gROOT.SetBatch(True)

# Specify the input file(s)
input_files = [os.path.abspath('B2Electrons.root')]

# Define the calibration algorithms
run_gain_alg = CDCDedxRunGainAlgorithm()
wire_gain_alg = CDCDedxWireGainAlgorithm()
cosine_alg = CDCDedxCosineAlgorithm()
twod_alg = CDCDedx2DCorrectionAlgorithm()
oned_alg = CDCDedx1DCleanupAlgorithm()

# Create Calibration objects from Collector, Algorithm(s), and input files
cdc_dedx_cal = Calibration(
    name='CDCDedxCalibrations',
    collector='CDCDedxElectronCollector',
    algorithms=[oned_alg, twod_alg, cosine_alg, wire_gain_alg, run_gain_alg],
    input_files=input_files)

# Create a CAF instance and add calibrations
caf_fw = CAF()
caf_fw.add_calibration(cdc_dedx_cal)

# Run the calibration
caf_fw.run()  # Creates local database files when finished (no auto upload)
