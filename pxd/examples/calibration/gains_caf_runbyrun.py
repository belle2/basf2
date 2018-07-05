#!/usr/bin/env python3
# -*- coding: utf-8 -*-


# This steering file computes PXD gain corrections from using bg simulations and
# beam data. The script uses the CAF framework.
#
# Before running, you have to put a file to IoV mapping called 'file_iov_map.pkl' in your
# working directory. You can create such a file using the script 'create_file_to_iov_map.py'
# in basf2 folder calibration/examples.
#
# Collector outputs need to be prepared in advance, using script gain_collector_mc.py
# Their file path pattern must be specified from the command line.
#
# Also we assume that hot pixel masks are already uploaded to the GT
# Calibration_Offline_Development (FIXME: create a dependence between calibration and do it in one script)
#
# Execute as: basf2 gains_caf_runbyrun.py -- --runLow=1700 --runHigh=2000 --expNo=3
#
# This will try to compute gain maps for each run in the given interval
# for experiment 3 using data files found in the file to iov mapping.
#
# There can be many subruns per runs. By default, no more than 10 subruns will
# be used.
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de

from basf2 import *
set_log_level(LogLevel.INFO)

import pickle
import glob
import os
import ROOT
from ROOT.Belle2 import PXDGainCalibrationAlgorithm
from caf.framework import Calibration, CAF
from caf import backends
from caf.utils import IoV
from caf.utils import get_iov_from_file
from caf.utils import find_absolute_file_paths


import argparse
parser = argparse.ArgumentParser(description="Compute gain maps for PXD from beam data")
parser.add_argument('--mc_filepath_pattern', default='./mc_gain_collector_outputs/*.root',
                    type=str, help='File pathe pattern to mc collector output files')
parser.add_argument('--runLow', default=0, type=int, help='Compute mask for specific IoV')
parser.add_argument('--runHigh', default=-1, type=int, help='Compute mask for specific IoV')
parser.add_argument('--expNo', default=3, type=int, help='Compute mask for specific IoV')
parser.add_argument('--maxSubRuns', default=15, type=int, help='Maximum number of subruns to use')
args = parser.parse_args()


# FIXME You need to hardcode the absolute paths for the MC collector outputs.
mc_collector_output_files = find_absolute_file_paths(glob.glob(args.mc_filepath_pattern))
print('List of mc collector output files is:  {}'.format(mc_collector_output_files))

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


# Create and configure the collector on beam data and its pre collector path
gaincollector_data = register_module("PXDGainCollector")
gaincollector_data.param("granularity", "run")
gaincollector_data.param("minClusterCharge", 8)
gaincollector_data.param("minClusterSize", 2)
gaincollector_data.param("maxClusterSize", 6)
gaincollector_data.param("collectSimulatedData", False)
gaincollector_data.param("nBinsU", 4)
gaincollector_data.param("nBinsV", 6)

# The pre collector path on data
pre_collector_path_data = create_path()
pre_collector_path_data.add_module("Gearbox", fileName='geometry/Beast2_phase2.xml')
pre_collector_path_data.add_module("Geometry")
pre_collector_path_data.add_module("ActivatePXDPixelMasker")
pre_collector_path_data.add_module('PXDUnpacker')
pre_collector_path_data.add_module("PXDRawHitSorter")
pre_collector_path_data.add_module("PXDClusterizer")


# Create and configure the calibration algorithm
algo = PXDGainCalibrationAlgorithm()

# We can play around with algo parameters
algo.minClusters = 1000      # Minimum number of collected clusters for estimating gains
algo.noiseSigma = 1.0        # Artificial noise sigma for smearing cluster charge

# We want to use a specific collector
algo.setPrefix("PXDGainCollector")


# Create a calibration
cal = Calibration(
    name="PXDGainCalibrationAlgorithm",
    collector=gaincollector_data,
    algorithms=algo,
    input_files=input_files)
cal.pre_collector_path = pre_collector_path_data

# Apply the map to this calibration, now the CAF doesn't have to do it
cal.files_to_iovs = files_to_iovs

# Here we set the AlgorithmStrategy for our algorithm
from caf.strategies import SequentialRunByRun, SingleIOV, SimpleRunByRun
# The SequentialRunByRun strategy executes your algorithm over runs
# individually to give you payloads for each one (if successful)
cal.strategies = SequentialRunByRun

cal.max_files_per_collector_job = 1


def algorithm_inputdata_setup(self, input_file_paths):
    """
    Extending the normal algorithm input file setup to also use some hardcoded files not created during the CAF running.
    We are basically patching the Algorithm class to use this function instead,

    The input files from the collector run during the CAF is the same (see caf.framework.Algorithm).
    It takes all files returned from the `Calibration.output_patterns` and filters for only the CollectorOutput.root files.
    Then it sets them as input files to the CalibrationAlgorithm class being managed.

    The extension happens when it also sets input files from the file list coming from outside.

    Parameters:
      self:             The caf.framework.Algorithm instance we are patching
      input_file_paths: The files found in the collector job output directories that matched `Calibration.output_patterns`
    """
    from caf.utils import B2INFO_MULTILINE
    from pathlib import Path

    # First we do the normal input file stuff
    collector_output_files = list(filter(lambda file_path: "CollectorOutput.root" == Path(file_path).name,
                                         input_file_paths))

    all_input_files = []
    all_input_files.extend(collector_output_files)
    # Now we can add to the input files the collector output files from the list mc_collector_output_files.
    all_input_files.extend(mc_collector_output_files)

    info_lines = ["Input files passed to algorithm {}:".format(self.name)]
    info_lines.extend(all_input_files)
    B2INFO_MULTILINE(info_lines)
    self.algorithm.setInputFileNames(all_input_files)


# Now patch it in
import functools
cal.algorithms[0].data_input = functools.partial(algorithm_inputdata_setup, cal.algorithms[0])

cal.use_central_database("Calibration_Offline_Development")


# Create a CAF instance and add the calibration to it.
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.backend = backends.Local(max_processes=16)
# Time between polling checks to the CAF to see if a step (algorithm, collector jobs) is complete
cal_fw.heartbeat = 30
# Can change where your calibration runs
cal_fw.output_dir = 'gain_calibration_results_range_{}_{}'.format(args.runLow, args.runHigh)
cal_fw.run(iov=iov_to_calibrate)
