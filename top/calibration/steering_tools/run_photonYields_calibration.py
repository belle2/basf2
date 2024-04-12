#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# CAF calibration script: photon yields
# data type: cdst (bhabha or dimuon)
#
# usage: basf2 run_photonYields_calibration.py expNo runFirst runLast [sample]
#        sample = bhabha/dimuon (D = dimuon)
# ---------------------------------------------------------------------------------------

import sys
import glob
from caf import backends
from caf.framework import CAF
from basf2 import B2ERROR
from top_calibration import photonYields_calibration
from caf.strategies import SingleIOV

# ----- those parameters need to be adjusted before running -----------------------------
#
globalTags = ['patch_main_release-08', 'patch_main_release-07_noTOP',
              'data_reprocessing_proc13', 'online']  # highest priority first
localDBs = []  # highest priority first, local DB's have higher priority than global tags
data_dir = '/gpfs/group/belle2/dataprod/Data/PromptReco/bucket16_calib/'
bhabha_skim_dir = 'skim/bhabha_all_calib/cdst/sub00'
dimuon_skim_dir = 'skim/mumutight_calib/cdst/sub00/'
main_output_dir = 'top_calibration'
default_sample = 'dimuon'
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

if len(argvs) > 4:
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
output_dir = f"{main_output_dir}/photonYields-{sample}-{expNo}-{run_range}"

# Define calibration
cal = photonYields_calibration(inputFiles, sample, globalTags, localDBs)
cal.strategies = SingleIOV
cal.backend_args = {"queue": "s"}

# Add calibration to CAF
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
