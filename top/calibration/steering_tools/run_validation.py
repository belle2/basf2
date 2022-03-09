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
# CAF calibration script: calibration validation
# data type: cdst (dimuon or bhabha)
#
# usage: basf2 run_validation.py expNo runFirst runLast [sample]
#        sample = bhabha/dimuon (D = dimuon)
# ---------------------------------------------------------------------------------------

import sys
import glob
from caf import backends
from caf.framework import CAF
from basf2 import B2ERROR
from top_calibration import calibration_validation

# ----- those parameters need to be adjusted before running -----------------------------
#
globalTags = ['data_reprocessing_prompt', 'dp_recon_release6_patch', 'online']  # highest priority first
localDBs = []  # highest priority first, local DB's have higher priority than global tags
data_dir = '/gpfs/group/belle2/dataprod/Data/PromptReco/bucket16_calib/'
bhabha_skim_dir = 'skim/bhabha_all_calib/cdst/sub00/'
dimuon_skim_dir = 'skim/mumutight_calib/cdst/sub00'
main_output_dir = 'top_calibration'
default_sample = 'dimuon'
new_cdst_format = True  # set to True for input in new cdst format
#
# ---------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 4:
    print("usage: basf2", argvs[0], "experiment runFirst runLast [sample]")
    print("       sample = bhabha/dimuon (D = dimuon)")
    sys.exit()
experiment = int(argvs[1])
run_first = int(argvs[2])
run_last = int(argvs[3])
sample = default_sample

if len(argvs) >= 5:
    sample = argvs[4]

if sample == 'bhabha':
    skim_dir = bhabha_skim_dir
elif sample == 'dimuon':
    skim_dir = dimuon_skim_dir
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

# Output folder name
run_range = 'r' + '{:0=5d}'.format(run_first) + '-' + '{:0=5d}'.format(run_last)
output_dir = f"{main_output_dir}/validation-{sample}-{expNo}-{run_range}"

# Define calibration
cal = calibration_validation(inputFiles, sample, globalTags, localDBs, new_cdst_format)
cal.backend_args = {"queue": "s"}

# Add calibration to CAF
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
