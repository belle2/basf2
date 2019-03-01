#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Calibrate module T0 with Bhabha (or dimuon) events using new constants from local DB
#
# usage: cdst_calibrateModuleT0.py -i <cdst_file.root>
#
# Note: replace local database name/location before running or comment it out
#       check global tag: it must be the same as used in production of input file(s)
# ---------------------------------------------------------------------------------------

from basf2 import *
import sys

# Database:
# - replace the name and location of the local DB before running!
# - payloads are searched for in the reverse order of DB's given below;
#   therefore the new calibration, if provided, is taken from the local DB.
# - one can even use several local DB's
use_central_database('data_reprocessing_proc7')  # global tag used in reprocessing
use_local_database('localDB/localDB.txt', 'localDB/')  # new calibration

# Create path
main = create_path()

# Input: cdst file(s) - bhabha skim, use -i option
roinput = register_module('RootInput')
main.add_module(roinput)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Time Recalibrator
recalibrator = register_module('TOPTimeRecalibrator')
recalibrator.param('useSampleTimeCalibration', True)
recalibrator.param('useChannelT0Calibration', True)
recalibrator.param('useModuleT0Calibration', False)
recalibrator.param('useCommonT0Calibration', True)
recalibrator.param('subtractBunchTime', True)
main.add_module(recalibrator)

# Channel masking
main.add_module('TOPChannelMasker')

# Module T0 calibration
calibrator = register_module('TOPModuleT0Calibrator')
calibrator.param('sample', 'bhabha')
calibrator.param('outputFileName', 'moduleT0_r*.root')
main.add_module(calibrator)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
