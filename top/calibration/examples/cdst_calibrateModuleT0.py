#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Calibrate module T0 with Bhabha (or dimuon) events using likelihood method
# Note: this method works correctly only if initial calibration is reasonably good
# (M. Staric, 2019-07-10)
#
# usage: basf2 cdst_calibrateModuleT0.py experiment runFirst runLast
#   job: bsub -q l "basf2 cdst_calibrateModuleT0.py experiment runFirst runLast"
#
# note: runLast is inclusive
# ---------------------------------------------------------------------------------------

import basf2 as b2
from ROOT import Belle2
import sys
import glob
import os

# ----- those need to be adjusted before running --------------------------------------
#
sampleType = 'bhabha'  # sample type: 'bhabha' or 'dimuon'
data_dir = '/group/belle2/dataprod/Data/release-03-02-02/DB00000635/proc00000009_nofilter'
skim_dir = 'skim/hlt_bhabha/cdst/sub00'
globalTag = 'data_reprocessing_prompt'  # base global tag (fall-back)
stagingTags = ['staging_data_reprocessing']  # list of global tags with new calibration
localDB = []  # list of local databases with new calibration
output_dir = 'moduleT0'  # main output folder
#
# -------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 4:
    print("usage: basf2", argvs[0], "experiment runFirst runLast")
    sys.exit()
experiment = int(argvs[1])
run_first = int(argvs[2])
run_last = int(argvs[3])

expNo = 'e' + '{:0=4d}'.format(experiment)

# Make list of files
files = []
for run in range(run_first, run_last + 1):
    runNo = 'r' + '{:0=5d}'.format(run)
    for typ in ['4S', 'Continuum', 'Scan']:
        folder = data_dir + '/' + expNo + '/' + typ + '/' + runNo + '/' + skim_dir
        files += glob.glob(folder + '/cdst.*.root')
if len(files) == 0:
    b2.B2ERROR('No cdst files found')
    sys.exit()

# Output folder
method = 'LL'
output_folder = output_dir + '/' + expNo + '/' + sampleType + '/' + method
if not os.path.isdir(output_folder):
    os.makedirs(output_folder)
    print('New folder created: ' + output_folder)

# Output file name
fileName = output_folder + '/moduleT0-' + expNo + '-'
run1 = 'r' + '{:0=5d}'.format(run_first)
run2 = 'r' + '{:0=5d}'.format(run_last)
fileName += run1 + '_to_' + run2 + '.root'
print('Output file:', fileName)


class Mask_BS13d(b2.Module):
    ''' exclude (mask-out) BS 13d '''

    def event(self):
        ''' event processing '''

        for digit in Belle2.PyStoreArray('TOPDigits'):
            if digit.getModuleID() == 13 and digit.getBoardstackNumber() == 3:
                digit.setHitQuality(Belle2.TOPDigit.c_Junk)


# Database
b2.use_central_database(globalTag)
for tag in stagingTags:
    b2.use_central_database(tag)
for db in localDB:
    if os.path.isfile(db):
        b2.use_local_database(db, invertLogging=True)
    else:
        b2.B2ERROR(db + ": local database not found")
        sys.exit()

# Create path
main = b2.create_path()

# Input (cdst files)
main.add_module('RootInput', inputFileNames=files)

# Initialize TOP geometry parameters
main.add_module('TOPGeometryParInitializer')

# Time Recalibrator
main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)

# Channel masking
main.add_module('TOPChannelMasker')

# Exclude BS13d
main.add_module(Mask_BS13d())

# Bunch finder
main.add_module('TOPBunchFinder', usePIDLikelihoods=True, subtractRunningOffset=False)

# Module T0 calibration
main.add_module('TOPModuleT0Calibrator', sample=sampleType, outputFileName=fileName)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
