#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# CAF calibration script: module T0 with cosmics
# data type: raw or cdst
#
# usage: basf2 run_moduleT0_with_cosmics.py expNo runFirst runLast
# ---------------------------------------------------------------------------------------

import sys
import glob
from caf import backends
from caf.framework import CAF
from basf2 import B2ERROR
from top_calibration import moduleT0_calibration_cosmics

# ----- those parameters need to be adjusted before running -----------------------------
#
globalTags = ['patch_main_release-08',
              'patch_main_release-07_noTOP',
              'data_reprocessing_prompt',
              'online']  # highest priority first
localDBs = []  # highest priority first, local DB's have higher priority than global tags
data_dir = '/gpfs/group/belle2/dataprod/Data/PromptSkim'
skim_dir = 'skim/cosmic_calib'
data_format = 'raw'  # raw or cdst
full_reco = True  # run full cosmic reconstruction if data_format == 'cdst' (we need separate track segments!)
main_output_dir = 'top_calibration'
#
# ---------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 4:
    print("usage: basf2", argvs[0], "experiment runFirst runLast")
    sys.exit()
experiment = int(argvs[1])
run_first = int(argvs[2])
run_last = int(argvs[3])

# Make list of files
inputFiles = []
expNo = 'e' + f'{experiment:04d}'
for run in range(run_first, run_last + 1):
    runNo = 'r' + f'{run:05d}'
    filename = f"{data_dir}/{expNo}/*/{runNo}/{skim_dir}/{data_format}/sub00/*.root"
    inputFiles += glob.glob(filename)

if len(inputFiles) == 0:
    B2ERROR('No files found in ' + data_dir + ' for exp=' + str(experiment) +
            ' runFirst=' + str(run_first) + ' runLast=' + str(run_last) +
            ' (skim_dir=' + skim_dir + ')')
    sys.exit()

# Output folder name
run_range = 'r' + f'{run_first:05d}' + '-' + f'{run_last:05d}'
output_dir = f"{main_output_dir}/moduleT0-cosmics-{expNo}-{run_range}"

# Define calibrations
cal = moduleT0_calibration_cosmics(inputFiles, globalTags, localDBs, data_format, full_reco)
if data_format == 'raw' or full_reco:
    cal.backend_args = {"queue": "l"}
else:
    cal.backend_args = {"queue": "s"}

# Add calibrations to CAF
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
