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
run_gains = Calibration(
    name='CDCDedxRunGainCalibration',
    collector="CDCDedxElectronCollector",
    algorithms=run_gain_alg,
    input_files=input_files)
wire_gains = Calibration(
    name='CDCDedxWireGainCalibration',
    collector="CDCDedxElectronCollector",
    algorithms=wire_gain_alg,
    input_files=input_files)
cosine = Calibration(
    name='CDCDedxCosineCalibration',
    collector="CDCDedxElectronCollector",
    algorithms=cosine_alg,
    input_files=input_files)
twod = Calibration(
    name='CDCDedx2DCalibration',
    collector="CDCDedxElectronCollector",
    algorithms=twod_alg,
    input_files=input_files)
oned = Calibration(
    name='CDCDedx1DCleanup',
    collector="CDCDedxElectronCollector",
    algorithms=oned_alg,
    input_files=input_files)

# Create a CAF instance and add calibrations
caf = CAF()
caf.add_calibration(run_gains)
caf.add_calibration(wire_gains)
caf.add_calibration(cosine)
caf.add_calibration(twod)
caf.add_calibration(oned)

# Run the calibration
caf.run()  # Creates local database files when finished (no auto upload)
