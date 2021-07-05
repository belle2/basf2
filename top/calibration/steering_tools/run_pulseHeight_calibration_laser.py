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
# CAF calibration script: pulse height distributions and threshold efficiencies
# data type: local runs with laser
#
# usage: basf2 run_pulseHeight_calibration_laser.py expNo run_1 run_2 ... run_n
# ---------------------------------------------------------------------------------------

import sys
import glob
from caf import backends
from caf.framework import CAF
from basf2 import B2ERROR
from top_calibration import pulseHeight_calibration_laser

# ----- those parameters need to be adjusted before running -----------------------
#
globalTags = ['Reco_master_patch', 'data_reprocessing_proc10']  # highest priority first
localDBs = []  # highest priority first, local DB's have higher priority than global tags
data_dir = '/ghi/fs01/belle2/bdata/group/detector/TOP/2019-*/data_sroot_global/'
main_output_dir = 'top_calibration'
look_back = 28  # look-back window setting (set to 0 if look-back setting available in DB)
t_min = -50.0  # lower edge of time window to select laser signal [ns]
t_max = 0.0    # upper edge of time window to select laser signal [ns]
#
# ---------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 3:
    print("usage: basf2", argvs[0], "experiment run_1 run_2 ... run_n")
    sys.exit()
experiment = int(argvs[1])
run_numbers = sorted([int(r) for r in argvs[2:]])
run_first = run_numbers[0]
run_last = run_numbers[-1]

# Make list of files
inputFiles = []
expNo = 'e' + '{:0=4d}'.format(experiment)
for run in run_numbers:
    expRun = '{:0=4d}'.format(experiment) + '.' + '{:0=5d}'.format(run)
    filename = f"{data_dir}/top.{expRun}.*.sroot"
    inputFiles += glob.glob(filename)

if len(inputFiles) == 0:
    runs = "".join([str(r) + "," for r in run_numbers])[:-1]
    B2ERROR(f'No sroot files found in {data_dir} for exp={str(experiment)} runs={runs}')
    sys.exit()

# Output folder name
run_range = 'r' + '{:0=5d}'.format(run_first) + '-' + '{:0=5d}'.format(run_last)
output_dir = f"{main_output_dir}/pulseHeight-laser-{expNo}-{run_range}"

# Define calibration
cal = pulseHeight_calibration_laser(inputFiles, t_min, t_max, look_back,
                                    globalTags, localDBs)
cal.backend_args = {"queue": "s"}

# Add calibration to CAF
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
