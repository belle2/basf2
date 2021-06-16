#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# CAF calibration script: module alignment
# data type: cdst (dimuon or bhabha)
#
# usage: basf2 run_alignment.py expNo runFirst runLast [sample]
#        sample = dimuon/bhabha (D = dimuon)
#
# Note: input data or databases must be well calibrated (including moduleT0 and commonT0)
#
# author: M. Staric
# ---------------------------------------------------------------------------------------

import sys
import glob
from caf import backends
from caf.framework import CAF
from basf2 import B2ERROR
from top_calibration import module_alignment

# ----- those parameters need to be adjusted before running -----------------------------
#
globalTags = ['Reco_master_patch', 'data_reprocessing_proc10']  # highest priority first
localDBs = []  # highest priority first, local DB's have higher priority than global tags
data_dir = '/group/belle2/dataprod/Data/OfficialReco/proc10/'
dimuon_skim_dir = 'offskim/offskim_mumutop/cdst/sub00'
bhabha_skim_dir = 'skim/hlt_bhabha/cdst/sub00'
main_output_dir = 'top_calibration'
default_sample = 'dimuon'
fixedParameters = None  # list of parameters to be fixed, for the names see: basf2 -m TOPAlignmentCollector
new_cdst_format = False  # set to True for input in new cdst format
#
# ---------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 4:
    print("usage: basf2", argvs[0], "experiment runFirst runLast [sample]")
    print("       sample = dimuon/bhabha (D = dimuon)")
    sys.exit()
experiment = int(argvs[1])
run_first = int(argvs[2])
run_last = int(argvs[3])
sample = default_sample

if len(argvs) > 4:
    sample = argvs[4]

if sample == 'dimuon':
    skim_dir = dimuon_skim_dir
elif sample == 'bhabha':
    skim_dir = bhabha_skim_dir
else:
    B2ERROR("Invalid sample name: " + sample)
    sys.exit()

# Make list of files
inputFiles = []
expNo = 'e' + '{:0=4d}'.format(experiment)
for run in range(run_first, run_last + 1):
    runNo = 'r' + '{:0=5d}'.format(run)
    filename = f"{data_dir}/{expNo}/*/{runNo}/{skim_dir}/cdst*.root"
    inputFiles += glob.glob(filename)

if len(inputFiles) == 0:
    B2ERROR('No cdst files found in ' + data_dir + ' for exp=' + str(experiment) +
            ' runFirst=' + str(run_first) + ' runLast=' + str(run_last) +
            ' (skim_dir=' + skim_dir + ')')
    sys.exit()

# Output folder
run_range = 'r' + '{:0=5d}'.format(run_first) + '-' + '{:0=5d}'.format(run_last)
output_dir = f"{main_output_dir}/alignment-{sample}-{expNo}-{run_range}"

# Define calibration
backend_args = {"queue": "l"}
cal = module_alignment(inputFiles, sample, fixedParameters,
                       globalTags, localDBs, new_cdst_format, backend_args)

# Add calibrations to CAF
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
