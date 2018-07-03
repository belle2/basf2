#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file computes PXD gain corrections from using bg simulations and
# beam data. The script uses the CAF framework.
#
# Execute as: basf2 gains_caf.py -- --mc_filepath_pattern='/whatever/*.root' --data_filepath_pattern='/whatever/*.root'
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
parser = argparse.ArgumentParser(description="Compute hot pixel masks for PXD from rawhit occupancy")
parser.add_argument('--mc_filepath_pattern', default='', type=str, help='File path pattern')
parser.add_argument('--data_filepath_pattern', default='', type=str, help='File path pattern')
args = parser.parse_args()


input_files_data = find_absolute_file_paths(glob.glob(args.data_filepath_pattern))
print('List of data input files is:  {}'.format(input_files_data))

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

input_files_mc = find_absolute_file_paths(glob.glob(args.mc_filepath_pattern))
print('List of mc input files is:  {}'.format(input_files_mc))

# Create and configure the collector on mc data and its pre collector path
gaincollector_mc = register_module("PXDGainCollector")
gaincollector_mc.param("granularity", "run")
gaincollector_mc.param("minClusterCharge", 8)
gaincollector_mc.param("minClusterSize", 2)
gaincollector_mc.param("maxClusterSize", 6)
gaincollector_mc.param("collectSimulatedData", True)
gaincollector_mc.param("nBinsU", 4)
gaincollector_mc.param("nBinsV", 6)

# The pre collector path on data
pre_collector_path_mc = create_path()
pre_collector_path_mc.add_module("Gearbox", fileName='geometry/Beast2_phase2.xml')
pre_collector_path_mc.add_module("Geometry")
pre_collector_path_mc.add_module("ActivatePXDPixelMasker")
pre_collector_path_mc.add_module("PXDDigitizer")
pre_collector_path_mc.add_module("PXDClusterizer")


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
    collector=gaincollector_mc,
    algorithms=algo,
    input_files=input_files)
cal.pre_collector_path = pre_collector_path_mc


# Here we set the AlgorithmStrategy for our algorithm
from caf.strategies import SequentialRunByRun, SingleIOV, SimpleRunByRun
# The SequentialRunByRun strategy executes your algorithm over runs
# individually to give you payloads for each one (if successful)
cal.strategies = SingleIOV  # SequentialRunByRun

cal.max_files_per_collector_job = 1

# Create a CAF instance and add the calibration to it.
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.backend = backends.Local(max_processes=2)
# Time between polling checks to the CAF to see if a step (algorithm, collector jobs) is complete
cal_fw.heartbeat = 30
# Can change where your calibration runs
cal_fw.output_dir = 'calibration_results'
cal_fw.run()
# Should have created a directory called 'calibration_results'
