#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# CAF calibration script: pulse height distributions and threshold efficiencies
# data type: raw data
#
# usage: basf2 run_pulseHeight_calibration_rawdata.py expNo runFirst runLast
# ---------------------------------------------------------------------------------------

import sys
import glob
from caf import backends
from caf.framework import CAF
from basf2 import B2ERROR
from top_calibration import pulseHeight_calibration_rawdata

# ----- those parameters need to be adjusted before running -----------------------
#
globalTags = ['Reco_master_patch', 'data_reprocessing_proc10']  # highest priority first
localDBs = []  # highest priority first, local DB's have higher priority than global tags
data_dir = '/group/belle2/dataprod/Data/Raw'
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
    filename = f"{data_dir}/{expNo}/{runNo}/sub00/physics.*.root"
    inputFiles += glob.glob(filename)

if len(inputFiles) == 0:
    B2ERROR('No rawdata files found in ' + data_dir + ' for exp=' + str(experiment) +
            ' runFirst=' + str(run_first) + ' runLast=' + str(run_last))
    sys.exit()

# Output folder name
run_range = 'r' + f'{run_first:05d}' + '-' + f'{run_last:05d}'
output_dir = f"{main_output_dir}/pulseHeight-rawdata-{expNo}-{run_range}"

# Define calibration
cal = pulseHeight_calibration_rawdata(inputFiles, globalTags, localDBs)
cal.backend_args = {"queue": "s"}

# Add calibration to CAF
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
