#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# CAF calibration script: BS13d carrier shifts
# data type: cdst hlt_bhabha
#
# usage: basf2 run_BS13d_calibration_bhabha.py expNo runFirst runLast
#
# author: M. Staric
# ---------------------------------------------------------------------------------------

import sys
import os
import glob
from caf import backends
from caf.framework import CAF
from basf2 import B2ERROR
from top_calibration import BS13d_calibration_cdst

# ----- those parameters need to be adjusted before running -----------------------------
#
globalTags = ['data_reprocessing_prompt_rel4_patchb']  # highest priority first
localDBs = []  # highest priority first, local DB's have higher priority than global tags
data_dir = '/group/belle2/dataprod/Data/release-03-02-02/DB00000654/proc9/'
skim_dir = 'skim/hlt_bhabha/cdst/sub00'
main_output_dir = 'top_calibration'
maxFiles = 10  # maximum number of input files per run (0 or negative means all)
time_offset = -66.8  # set to 0 for release-4 processed data or newer
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
expNo = 'e' + '{:0=4d}'.format(experiment)
for run in range(run_first, run_last + 1):
    runNo = 'r' + '{:0=5d}'.format(run)
    filename = f"{data_dir}/{expNo}/*/{runNo}/{skim_dir}/cdst*.root"
    files = glob.glob(filename)
    if maxFiles > 0:
        for i in range(min(len(files), maxFiles)):
            inputFiles.append(files[i])
    else:
        inputFiles += files

if len(inputFiles) == 0:
    B2ERROR('No cdst files found in ' + data_dir + ' for exp=' + str(experiment) +
            ' runFirst=' + str(run_first) + ' runLast=' + str(run_last) +
            ' (skim_dir=' + skim_dir + ')')
    sys.exit()

# Output folder name
run_range = 'r' + '{:0=5d}'.format(run_first) + '-' + '{:0=5d}'.format(run_last)
output_dir = f"{main_output_dir}/BS13d-bhabha-{expNo}-{run_range}"

# Define calibration
cal = BS13d_calibration_cdst(inputFiles, time_offset, globalTags, localDBs)
cal.backend_args = {"queue": "s"}

# Add calibration to CAF
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
