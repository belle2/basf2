#############################################################
# Run the dE/dx calibration with the CAF:
#  - collect the samples with the Collector modules
#  - run the calibrations with the given algorithms
#  - produce output database files
#
# Usage: basf2 firstDedxAttempt.py
#############################################################

from basf2 import *

import os
import sys

import ROOT
from ROOT.Belle2 import CDCDedxWireGainAlgorithm, CDCDedxRunGainAlgorithm, CDCDedxCosineAlgorithm
from caf.framework import Calibration, CAF

# Specify the input file(s)
input_files = [os.path.abspath('../B2Electrons.root')]

# Define the calibration algorithms
run_gain_alg = CDCDedxRunGainAlgorithm()
wire_gain_alg = CDCDedxWireGainAlgorithm()
cosine_alg = CDCDedxCosineAlgorithm()

# Create Calibration objects from Collector, Algorithm(s), and input files
run_gains = Calibration(
    name='CDCDedxRunGainCalibration',
    collector="CDCDedxRunGainCollector",
    algorithms=run_gain_alg,
    input_files=input_files)
wire_gains = Calibration(
    name='CDCDedxWireGainCalibration',
    collector="CDCDedxWireGainCollector",
    algorithms=wire_gain_alg,
    input_files=input_files)
cosine = Calibration(
    name='CDCDedxCosineCalibration',
    collector="CDCDedxCosineCollector",
    algorithms=cosine_alg,
    input_files=input_files)

# Create a CAF instance and add calibrations
caf = CAF()
# caf.add_calibration(run_gains)
caf.add_calibration(wire_gains)
# caf.add_calibration(cosine)

# Run the calibration
caf.run()  # Creates local database files when finished (no auto upload)
