#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# 2 - calibrate
# Run the electron calibration on the sample generated in
# step 1. For now, this is a dummy calibration for testing.
#
# Usage: basf2 2_calibrate.py
#
# Input: B2Electrons.root (use 1_generate.py)
# Output: calibration_results/CDCElectronCalibration/outputdb/
# dbstore_CDCElectronCollector_rev_1.root
#
# Example steering file - 2011 Belle II Collaboration
#############################################################

import os
from basf2 import *
from caf.framework import Calibration, CAF
from caf import backends
import ROOT

# Create Calibration object from Collector, Algorithm(s), and input files
collector = register_module('CDCElectronCollector')
algorithm = ROOT.Belle2.CDCElectronCalibrationAlgorithm()
input_files = os.path.expandvars('$BELLE2_LOCAL_DIR/reconstruction/examples/B2Electrons.root')
cal = Calibration('CDCElectronCalibration', collector, algorithm, input_files)

# Create a CAF instance to configure how we will run
cal_fw = CAF()
cal_fw.max_iterations = 3
cal_fw.add_calibration(cal)

# Run the calibration
cal_fw.run()  # Creates local database files when finished (no auto upload)
