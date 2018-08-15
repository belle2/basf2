#!/usr/bin/env python3
# -*- coding: utf-8 -*-


# This steering file computes PXD gain corrections from using bg simulations and
# beam data. The script uses the CAF framework. This script version uses by default
# SimpleRunByRun strategy and is designed to compute run-by-run gain corrections
# on for a large set of runs.
#
# Before calibration, you have to put a file to IoV mapping called 'file_iov_map.pkl' in your
# working directory. The mapping file should map beam all run files (real data) from phase 2
# that you want to calibrate. You can create such a file by using:
#
# basf2 create_file_to_iov_map.py -- --option='metadata' --file_path_pattern='/home/benjamin/BeamRun18/r0*/**/*.root'
# FIXME: adjust this to KEKCC case
#
# The next step is to prepare mc runs containing PXDSimHits. That setup for the simulation
# should mimik the situation of beam data as closely as possible.
#
# basf2 submit_create_mcruns.py -- --backend='local' --outputdir='pxd_mc_phase2' --runLow=0 --runHigh=5000 --expNo=3
# FIXME: adjust this to some resoanable default range
#
# The scribts submicts the creation of mc runs to a CAF.backend (here local) for all for given run range. Runs in the
# specified runs but not found in 'file_iov_map.pkl' will be skipped. The simulated runs will be collected in a folder
# 'pxd_mc_files'.
#
# Finally, a CAF script for the calibration needs to be started:
#
# basf2 gains_caf_runbyrun.py -- --runLow=0 --runHigh=5000 --expNo=3 --mc_input_files='pxd_mc_phase2'
# FIXME: adjust this to some resoanable default range
#
# The results will be collected in a folder 'pxd_calibration_results_range_0_5000_3'. In order to complete the
# process, the check and uploads the outputdb for the PXDGainCalibration to a global tag (GT).
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de

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
parser = argparse.ArgumentParser(description="Compute gain correction maps for PXD from beam data")
parser.add_argument('--runLow', default=0, type=int, help='Compute mask for specific IoV')
parser.add_argument('--runHigh', default=-1, type=int, help='Compute mask for specific IoV')
parser.add_argument('--expNo', default=3, type=int, help='Compute mask for specific IoV')
parser.add_argument('--mc_input_files', default='pxd_mc_phase2', type=str,
                    help='Lets take some file patterns. We could have put wildcards in more places but this is enough for testing.')
parser.add_argument('--maxSubRuns', default=15, type=int, help='Maximum number of subruns to use')
args = parser.parse_args()

# Set the IoV range for this calibration
iov_to_calibrate = IoV(exp_low=args.expNo, run_low=args.runLow, exp_high=args.expNo, run_high=args.runHigh)

# Access files_to_iovs for beam runs
with open("file_iov_map.pkl", 'br') as map_file:
    files_to_iovs = pickle.load(map_file)

# Get list of input files (beam data)
input_files = []

input_file_iov_set = set(files_to_iovs.values())
for file_iov in input_file_iov_set:
    if iov_to_calibrate.contains(file_iov):
        subruns = [k for k, v in files_to_iovs.items() if v == file_iov]
        input_files.extend(subruns[:args.maxSubRuns])

print('Number selected input files:  {}'.format(len(input_files)))

# Get list of input files (MC)
mc_input_files = find_absolute_file_paths(glob.glob(args.mc_input_files + '/*.root'))

print('Number selected mc input files:  {}'.format(len(mc_input_files)))


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
    input_files=input_files)
charge_cal.pre_collector_path = pre_charge_collector_path

# Apply the map to this calibration, now the CAF doesn't have to do it
charge_cal.files_to_iovs = files_to_iovs

# Here we set the AlgorithmStrategy
charge_cal.strategies = SimpleRunByRun
charge_cal.max_files_per_collector_job = 1
charge_cal.use_central_database("Calibration_Offline_Development")

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
    input_files=mc_input_files)
gain_cal.pre_collector_path = pre_gain_collector_path

# Apply the map to this calibration, now the CAF doesn't have to do it
# gain_cal.files_to_iovs = mc_files_to_iovs

# Here we set the AlgorithmStrategy
gain_cal.strategies = SimpleRunByRun
gain_cal.max_files_per_collector_job = 1
gain_cal.use_central_database("Calibration_Offline_Development")

# Define dependencies. In this case: charge_cal -> gain_cal
gain_cal.depends_on(charge_cal)


# Create a CAF instance and add the calibration to it.
cal_fw = CAF()
cal_fw.add_calibration(charge_cal)
cal_fw.add_calibration(gain_cal)
cal_fw.backend = backends.Local(max_processes=16)
# Time between polling checks to the CAF to see if a step (algorithm, collector jobs) is complete
cal_fw.heartbeat = 30
# Can change where your calibration runs
cal_fw.output_dir = 'pxd_calibration_results_range_{}_{}_{}'.format(args.runLow, args.runHigh, args.expNo)
cal_fw.run(iov=iov_to_calibrate)
