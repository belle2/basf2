#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# CAF calibration script: channel masking calibration
# data type: cdst
#
# usage: basf2 run_channelMask_calibration.py expNo runFirst runLast
# ---------------------------------------------------------------------------------------

import sys
import glob
from caf import backends
from caf.framework import CAF
from basf2 import B2ERROR
from top_calibration import channel_mask_calibration

# ----- those parameters need to be adjusted before running -----------------------
#
globalTags = ['data_reprocessing_proc11']  # highest priority first
localDBs = []  # highest priority first, local DB's have higher priority than global tags
data_dir = '/group/belle2/dataprod/Data/OfficialReco/proc11/'
skim_dir = 'skim/hlt_hadron/cdst/sub00'
main_output_dir = 'top_calibration'
new_cdst_format = False  # set to True for input in new cdst format
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
    filename = f"{data_dir}/{expNo}/*/{runNo}/{skim_dir}/cdst*.root"
    inputFiles += glob.glob(filename)

if len(inputFiles) == 0:
    B2ERROR('No cdst files found in ' + data_dir + ' for exp=' + str(experiment) +
            ' runFirst=' + str(run_first) + ' runLast=' + str(run_last) +
            ' (skim_dir=' + skim_dir + ')')
    sys.exit()

# Output folder name
run_range = 'r' + f'{run_first:05d}' + '-' + f'{run_last:05d}'
output_dir = f"{main_output_dir}/channelMask-{expNo}-{run_range}"

# Define calibration
cal = channel_mask_calibration(inputFiles, globalTags, localDBs, new_cdst_format)
cal.backend_args = {"queue": "s"}

# Add calibration to CAF
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
