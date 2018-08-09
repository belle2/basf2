#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file computes PXD hot pixel masks from root formatted raw data
# running the CAF
#
# Before running, you have to put a file to IoV mapping called 'file_iov_map.pkl' in your
# working directory. You can create such a file using the script 'create_file_to_iov_map.py'
# in basf2 folder calibration/examples.
#
# Execute as: basf2 hotpixel_caf_runbyrun.py -- --runLow=1700 --runHigh=2000 --expNo=2
#
# This will try to compute hot pixel masks for each run in the given interval
# for experiment 2 using data files found in the file to iov mapping.
#
# There can be many subruns per runs. By default, no more than 25 subruns will
# be used.
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de

from basf2 import *
set_log_level(LogLevel.INFO)

import pickle
import glob
import os
import ROOT
from ROOT.Belle2 import PXDHotPixelMaskCalibrationAlgorithm
from caf.framework import Calibration, CAF
from caf import backends
from caf.utils import IoV
from caf.utils import get_iov_from_file

import argparse
parser = argparse.ArgumentParser(description="Compute hot pixel masks for PXD from rawhit occupancy")
parser.add_argument('--runLow', default=0, type=int, help='Compute mask for specific IoV')
parser.add_argument('--runHigh', default=-1, type=int, help='Compute mask for specific IoV')
parser.add_argument('--expNo', default=3, type=int, help='Compute mask for specific IoV')
parser.add_argument('--maxSubRuns', default=15, type=int, help='Maximum number of subruns to use')
args = parser.parse_args()


# Set the IoV range for this calibration
iov_to_calibrate = IoV(exp_low=args.expNo, run_low=args.runLow, exp_high=args.expNo, run_high=args.runHigh)

map_file_path = "file_iov_map.pkl"
with open(map_file_path, 'br') as map_file:
    files_to_iovs = pickle.load(map_file)


input_file_iov_set = set(files_to_iovs.values())
print('Number of distinct iovs {}'.format(len(input_file_iov_set)))


input_files = []

for file_iov in input_file_iov_set:
    if iov_to_calibrate.contains(file_iov):
        subruns = [k for k, v in files_to_iovs.items() if v == file_iov]
        input_files.extend(subruns[:args.maxSubRuns])


print('Number selected input files:  {}'.format(len(input_files)))

# Create and configure the collector and its pre collector path
hotpixelcollector = register_module("PXDRawHotPixelMaskCollector")
hotpixelcollector.param("granularity", "run")

# The pre collector path must contain geometry and unpacker
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'geometry/Beast2_phase2.xml')
geometry = register_module('Geometry')
geometry.param('components', ['PXD'])
pre_collector_path = create_path()
pre_collector_path.add_module(gearbox)
pre_collector_path.add_module(geometry)
pre_collector_path.add_module('PXDUnpacker')


# Create and configure the calibration algorithm
hotpixelkiller = PXDHotPixelMaskCalibrationAlgorithm()  # Getting a calibration algorithm instance
# We can play around with hotpixelkiller parameters
hotpixelkiller.forceContinueMasking = True   # Continue masking even when few/no events were collected
hotpixelkiller.minEvents = 10000             # Minimum number of collected events for masking
hotpixelkiller.minHits = 15                  # Only consider dead pixel masking when median number of hits per pixel is higher
hotpixelkiller.pixelMultiplier = 5           # Occupancy threshold is median occupancy x multiplier
hotpixelkiller.maskDrains = True             # Set True to allow masking of hot drain lines
hotpixelkiller.minHitsDrain = 200            # Only consider dead drain masking when median number of hits per drain is higher
hotpixelkiller.drainMultiplier = 5           # Occupancy threshold is median occupancy x multiplier
hotpixelkiller.maskRows = True               # Set True to allow masking of hot rows
hotpixelkiller.minHitsRow = 200              # Only consider dead row masking when median number of hits per row is higher
hotpixelkiller.rowMultiplier = 5             # Occupancy threshold is median occupancy x multiplier
# We want to use a specific collector collecting from raw hits
hotpixelkiller.setPrefix("PXDRawHotPixelMaskCollector")

# Create a calibration
cal = Calibration(
    name="PXDHotPixelMaskCalibrationAlgorithm",
    collector=hotpixelcollector,
    algorithms=hotpixelkiller,
    input_files=input_files)
cal.pre_collector_path = pre_collector_path

# Apply the map to this calibration, now the CAF doesn't have to do it
cal.files_to_iovs = files_to_iovs

# Here we set the AlgorithmStrategy for our algorithm
from caf.strategies import SequentialRunByRun, SingleIOV, SimpleRunByRun
# The SequentialRunByRun strategy executes your algorithm over runs
# individually to give you payloads for each one (if successful)
cal.strategies = SimpleRunByRun  # SequentialRunByRun

cal.max_files_per_collector_job = 1

# Create a CAF instance and add the calibration to it.
cal_fw = CAF()
cal_fw.add_calibration(cal)
# cal_fw.backend = backends.LSF()
cal_fw.backend = backends.Local(max_processes=2)
# Time between polling checks to the CAF to see if a step (algorithm, collector jobs) is complete
cal_fw.heartbeat = 30
# Can change where your calibration runs
cal_fw.output_dir = 'calibration_results_range_{}_{}'.format(args.runLow, args.runHigh)
cal_fw.run(iov=iov_to_calibrate)
# Should have created a directory called 'calibration_results'
