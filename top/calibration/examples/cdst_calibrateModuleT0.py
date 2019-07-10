#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Calibrate module T0 with Bhabha (or dimuon) events using likelihood method
# Note: this method works correctly only if initial calibration is reasonably good
#
# usage: basf2 cdst_calibrateModuleT0.py runFirst runLast
#   job: bsub -q l "basf2 cdst_calibrateModuleT0.py runFirst runLast"
#
# note: runLast is inclusive
# ---------------------------------------------------------------------------------------

from basf2 import *
import sys
import glob
import os

# ----- those need to be adjusted before running --------------------------------------
#
experiment = 7
sample = 'bhabha'
input_dir = '/ghi/fs01/belle2/bdata/Data/e0007/4S/Bucket4/release-03-01-01/DB00000598/'
skim_dir = 'skim/hlt_bhabha/cdst/sub00/'
globalTag = 'data_reprocessing_prompt'  # base global tag
stagingTags = []  # list of staging tags with new calibration constants
localDB = []  # list of local databases with new calibration constants
output_dir = 'moduleT0'  # main output folder
#
# -------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 3:
    print("usage: basf2", argvs[0], "runFirst runLast")
    sys.exit()
run_first = int(argvs[1])
run_last = int(argvs[2])

# Make list of files
files = []
for run in range(run_first, run_last + 1):
    runNo = 'r' + '{:0=5d}'.format(run)
    files += glob.glob(input_dir + '/' + runNo + '/' + skim_dir + '/cdst.*.root')
if len(files) == 0:
    B2ERROR('No cdst files found')
    sys.exit()

# Output folder
expNo = 'e' + '{:0=4d}'.format(experiment)
output_folder = output_dir + '/' + expNo
if not os.path.isdir(output_folder):
    os.makedirs(output_folder)
    print('New folder created: ' + output_folder)

# Output file name
fileName = output_folder + '/moduleT0-' + expNo + '-'
run1 = 'r' + '{:0=5d}'.format(run_first)
run2 = 'r' + '{:0=5d}'.format(run_last)
fileName += run1 + '_to_' + run2 + '.root'

# Database
use_central_database(globalTag)
for tag in stagingTags:
    use_central_database(tag)
for db in localDB:
    if os.path.isfile(db):
        use_local_database(db, invertLogging=True)
    else:
        B2ERROR(db + ": local database not found")
        sys.exit()

# Create path
main = create_path()

# Input (cdst files)
main.add_module('RootInput', inputFileNames=files)

# Initialize TOP geometry parameters
main.add_module('TOPGeometryParInitializer')

# Time Recalibrator
main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)

# Channel masking
main.add_module('TOPChannelMasker')

# Bunch finder
main.add_module('TOPBunchFinder', usePIDLikelihoods=True)

# Module T0 calibration
calibrator = register_module('TOPModuleT0Calibrator')
calibrator.param('sample', sample)
calibrator.param('outputFileName', fileName)
main.add_module(calibrator)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
