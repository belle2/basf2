#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file computes PXD gain corrections from using bg simulations and
# beam data. The script uses the CAF framework. This version of the script uses SingleIOV
# strategy. Input data files are specified using an input file pattern.
#
# basf2 gains_caf.py -- --data_filepath_pattern='/home/benjamin/BeamRun18/r03360/sub00/*.root'
#                       --mc_filepath_pattern='./pxd_mc_phase2/beam.0003.03360.*.root'
#
# The user is responsible to make sure that the ExpRuns for Data and MC match up. If this is to difficult,
# use the script gains_caf_runbyrun.py instead.
#
# The results will be collected in a folder 'gain_calibration_results'. In order to complete the
# process, the check and uploads the outputdb for the PXDGainCalibration to a global tag (GT).
#
# author: benjamin.schwenker@phys.uni-goettingen.de


from basf2 import *
set_log_level(LogLevel.INFO)

import pickle
import glob
import os
import ROOT
from ROOT.Belle2 import PXDGainCalibrationAlgorithm
from ROOT.Belle2 import PXDMedianChargeCalibrationAlgorithm
from caf.framework import Calibration, CAF
from caf import backends
from caf.utils import IoV
from caf.utils import get_iov_from_file
from caf.utils import find_absolute_file_paths
from caf.strategies import SequentialRunByRun, SingleIOV, SimpleRunByRun


import argparse
parser = argparse.ArgumentParser(description="Compute PXD gain calibrations from beam data runs")
parser.add_argument('--data_filepath_pattern', default='', type=str, help='Data file path pattern')
parser.add_argument('--mc_filepath_pattern', default='./pxd_mc_phase2/*.root', type=str, help='MC file pathe pattern')
args = parser.parse_args()


input_files_mc = find_absolute_file_paths(glob.glob(args.mc_filepath_pattern))
print('List of mc input files is:  {}'.format(input_files_mc))

input_files_data = find_absolute_file_paths(glob.glob(args.data_filepath_pattern))
print('List of data input files is:  {}'.format(input_files_data))


# Create and configure the collector on beam data and its pre collector path
charge_collector = register_module("PXDClusterChargeCollector")
charge_collector.param("granularity", "run")
charge_collector.param("minClusterCharge", 8)
charge_collector.param("minClusterSize", 2)
charge_collector.param("maxClusterSize", 6)
charge_collector.param("nBinsU", 4)
charge_collector.param("nBinsV", 6)

# The pre collector path on data
pre_charge_collector_path = create_path()
pre_charge_collector_path.add_module("Gearbox", fileName='geometry/Beast2_phase2.xml')
pre_charge_collector_path.add_module("Geometry")
pre_charge_collector_path.add_module("ActivatePXDPixelMasker")
pre_charge_collector_path.add_module("PXDUnpacker")
pre_charge_collector_path.add_module("PXDRawHitSorter")
pre_charge_collector_path.add_module("PXDClusterizer")


# Create and configure the calibration algorithm
charge_algo = PXDMedianChargeCalibrationAlgorithm()

# We can play around with algo parameters
charge_algo.minClusters = 1000      # Minimum number of collected clusters for estimating gains
charge_algo.noiseSigma = 0.6        # Artificial noise sigma for smearing cluster charge
charge_algo.forceContinue = True    # Force continue algorithm instead of c_notEnoughData

# We want to use a specific collector
charge_algo.setPrefix("PXDClusterChargeCollector")

# Create a charge calibration
charge_cal = Calibration(
    name="PXDMedianChargeCalibrationAlgorithm",
    collector=charge_collector,
    algorithms=charge_algo,
    input_files=input_files_data)
charge_cal.pre_collector_path = pre_charge_collector_path

# Here we set the AlgorithmStrategy
charge_cal.strategies = SingleIOV
charge_cal.max_files_per_collector_job = 1
charge_cal.use_central_database("Calibration_Offline_Development")
charge_cal.max_files_per_collector_job = 1

# Create and configure the collector on mc data and its pre collector path
gain_collector = register_module("PXDClusterChargeCollector")
gain_collector.param("granularity", "run")
gain_collector.param("minClusterCharge", 8)
gain_collector.param("minClusterSize", 2)
gain_collector.param("maxClusterSize", 6)
gain_collector.param("nBinsU", 4)
gain_collector.param("nBinsV", 6)

# The pre collector path on mc files
pre_gain_collector_path = create_path()
pre_gain_collector_path.add_module("Gearbox", fileName='geometry/Beast2_phase2.xml')
pre_gain_collector_path.add_module("Geometry")
pre_gain_collector_path.add_module("PXDDigitizer")
pre_gain_collector_path.add_module("PXDClusterizer")


# Create and configure the calibration algorithm
gain_algo = PXDGainCalibrationAlgorithm()

# We can play around with algo parameters
gain_algo.minClusters = 1000      # Minimum number of collected clusters for estimating gains
gain_algo.noiseSigma = 0.6        # Artificial noise sigma for smearing cluster charge
gain_algo.forceContinue = True    # Force continue algorithm instead of c_notEnoughData

# We want to use a specific collector
gain_algo.setPrefix("PXDClusterChargeCollector")

# Create a charge calibration
gain_cal = Calibration(
    name="PXDGainChargeCalibrationAlgorithm",
    collector=gain_collector,
    algorithms=gain_algo,
    input_files=input_files_mc)
gain_cal.pre_collector_path = pre_gain_collector_path

# Here we set the AlgorithmStrategy
gain_cal.strategies = SingleIOV
gain_cal.max_files_per_collector_job = 1
gain_cal.use_central_database("Calibration_Offline_Development")

# Define dependencies. In this case: charge_cal -> gain_cal
gain_cal.depends_on(charge_cal)

# Create a CAF instance and add the calibration to it.
cal_fw = CAF()
cal_fw.add_calibration(charge_cal)
cal_fw.add_calibration(gain_cal)
cal_fw.backend = backends.Local(max_processes=2)
# Time between polling checks to the CAF to see if a step (algorithm, collector jobs) is complete
cal_fw.heartbeat = 30
# Can change where your calibration runs
cal_fw.output_dir = 'gain_calibration_results'
cal_fw.run()
# Should have created a directory called 'calibration_results'
