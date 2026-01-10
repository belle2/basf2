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
# data type: local runs with laser
#
# usage: basf2 run_pulseHeight_calibration_laser.py expNo -- --runs 'run_1 run_2 ...' [OPTIONS]
# ---------------------------------------------------------------------------------------

import argparse
import sys
from os import makedirs
import glob
from caf import backends
from caf.framework import CAF
from basf2 import B2ERROR
from top_calibration import pulseHeight_calibration_laser


# Command line argument parser
parser = argparse.ArgumentParser()
parser.add_argument('exp')
parser.add_argument('--runs')
parser.add_argument('--main_output_dir', default='top_pulse_height_calibration')
parser.add_argument('--look_back', type=int, default=28)
parser.add_argument('--t_min', type=float, default=-60.)
parser.add_argument('--t_max', type=float, default=-10.)
parser.add_argument('--source_dir', default='/ghi/fs01/belle2/bdata/group/detector/TOP/current_data_sroot_global')
parser.add_argument(
    '--localDBs',
    default='/group/belle2/group/detector/TOP/calibration/TBC/official/TBC_2025c_Nov25/localDB/localDB.txt',
    type=str
)
args = parser.parse_args()


# ----- These parameters may need to be adjusted --------------------
#
# globalTags = ['data_reprocessing_proc11', 'data_reprocessing_prompt']  # highest priority first
# globalTags = ['data_reprocessing_prompt', 'online']   # highest priority first
globalTags = ['online']
localDBs = args.localDBs.split()

data_dir = args.source_dir
main_output_dir = args.main_output_dir
look_back = args.look_back  # look-back window setting (set to 0 if look-back setting available in DB)
t_min = args.t_min  # lower edge of time window to select laser signal [ns]
t_max = args.t_max   # upper edge of time window to select laser signal [ns]

# -------------------------------------------------------------------

# Argument parsing
experiment = int(args.exp)
runs = args.runs.split(' ')
run_numbers = sorted([int(r) for r in runs])
run_first = run_numbers[0]
run_last = run_numbers[-1]

# Make list of files
inputFiles = []
expNo = 'e' + '{:0=4d}'.format(experiment)

for run in run_numbers:
    expRun = '{:0=4d}'.format(experiment) + '.' + '{:0=5d}'.format(run)
    filename = f"{data_dir}/top.{expRun}.*.root"
    # filename = f"{data_dir}/top.{expRun}.*.root"
    print(filename)
    inputFiles += glob.glob(filename)

if len(inputFiles) == 0:
    runs = "".join([str(r) + "," for r in run_numbers])[:-1]
    B2ERROR(f'No root files found in {data_dir} for exp={str(experiment)} runs={runs}')
    sys.exit()

# Output folder name
run_range = 'r' + '{:0=5d}'.format(run_first) + '-' + '{:0=5d}'.format(run_last)
output_dir = f"{main_output_dir}/pulseHeight-laser-{expNo}-{run_range}"
makedirs(output_dir, exist_ok=True)

# Define calibration
cal = pulseHeight_calibration_laser(inputFiles, t_min, t_max, look_back, globalTags, localDBs)
# cal.backend_args = {"queue": "s"}

# Add calibration to CAF
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()  # at KEKCC
# cal_fw.backend = backends.HTCondor() # at NAF

# Run calibration
cal_fw.run()
