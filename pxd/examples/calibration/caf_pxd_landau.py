##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This steering file computes PXD calibrations for hot pixels, dead pixels, hit occupancy and
# energy loss (gain). The script uses the CAF framework. This script uses by default the
# SequentialRunByRun strategy and is designed to try to compute new calibration constants
# regularly to follow temporal changes of the PXD.
#
# The script allows you to create a list of runs that will be ignored in the calibration.
# Use this list for known bad runs.
#
# Before calibration, you have to put a file to IoV mapping called 'file_iov_map.pkl' in your
# working directory. The mapping file should map file locations to IoV ranges and allows to
# logically address data by experiment and run numbers.  You can create such a map file by using
# the tool b2caf-filemap. See also the option --help.
#
# b2caf-filemap -m raw  -p "/hsm/belle2/bdata/Data/Raw/e0003/r*/**/*.root"
#
# The next step is to prepare mc runs containing PXDSimHits. That setup for the simulation
# should mimik the situation of beam data as closely as possible.
#
# basf2 submit_create_mcruns.py -- --backend='local' --outputdir='pxd_mc_phase2' --runLow=4000 --runHigh=6522 --expNo=3
#
# The scripts submits the creation of mc runs to a CAF.backend (here local) for all for given run range. Runs in the
# specified runs but not found in 'file_iov_map.pkl' will be skipped. The simulated runs will be collected in the folder
# outputdir. It is best to create another mapping file for the mc data.
#
# b2caf-filemap -m metadata  -p --output-file "dummy_mc_file_iov_map.pkl" "pxd_mc_phase2/*.root"
#
# Finally, a CAF script for the calibration needs to be started:
#
# basf2 caf_pxd_landau.py -- --runLow=5613 --runHigh=5613 --expNo=3
#
# The results will be collected in a folder 'pxd_calibration_results_range_XY'. In order to complete the
# process, the check and uploads the outputdbs to a global tag (GT).
#
# b2conditionsdb upload Calibration_Offline_Development ./database.txt
#
# The option --help provides extensive help for the b2conditionsdb tool.


import argparse
from caf.strategies import SequentialRunByRun
from caf.utils import ExpRun, IoV
from caf.backends import LSF
from caf.framework import Calibration, CAF
from ROOT.Belle2 import PXDHotPixelMaskCalibrationAlgorithm
from ROOT.Belle2 import PXDChargeCalibrationAlgorithm
from ROOT.Belle2 import PXDGainCalibrationAlgorithm
import ROOT
import pickle
import basf2 as b2
b2.set_log_level(b2.LogLevel.INFO)

ROOT.gROOT.SetBatch(True)


parser = argparse.ArgumentParser(
    description="Compute gain correction maps for PXD from beam data")
parser.add_argument(
    '--runLow',
    default=0,
    type=int,
    help='Compute mask for specific IoV')
parser.add_argument('--runHigh', default=-1, type=int,
                    help='Compute mask for specific IoV')
parser.add_argument(
    '--expNo',
    default=3,
    type=int,
    help='Compute mask for specific IoV')
parser.add_argument('--maxSubRuns', default=20, type=int,
                    help='Maximum number of subruns to use')
args = parser.parse_args()


# input files
pxd_ignore_run_list = [ExpRun(3, 484), ExpRun(3, 485), ExpRun(3, 486), ExpRun(3, 524)]

# Set the IoV range for this calibration
iov_to_calibrate = IoV(
    exp_low=args.expNo,
    run_low=args.runLow,
    exp_high=args.expNo,
    run_high=args.runHigh)


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


# Access files_to_iovs for MC runs
with open("mc_file_iov_map.pkl", 'br') as map_file:
    mc_files_to_iovs = pickle.load(map_file)

mc_input_files = list(mc_files_to_iovs.keys())


print('Number selected mc input files:  {}'.format(len(mc_input_files)))


# HOTPIXEl CALIBRATION

# Create and configure the collector and its pre collector path
hotpixel_collector = b2.register_module("PXDRawHotPixelMaskCollector")
hotpixel_collector.param("granularity", "run")

# The pre collector path must contain geometry and unpacker
pre_hotpixel_collector_path = b2.create_path()
pre_hotpixel_collector_path.add_module(
    "Gearbox", fileName='geometry/Beast2_phase2.xml')
pre_hotpixel_collector_path.add_module("Geometry", useDB=False)
pre_hotpixel_collector_path.add_module('PXDUnpacker')
pre_hotpixel_collector_path.add_module('PXDPostErrorChecker')
pre_hotpixel_collector_path.add_module('Progress')
# Create and configure the calibration algorithm
hotpixel_algo = PXDHotPixelMaskCalibrationAlgorithm()

# We can play around with hotpixelkiller parameters
# Continue masking even when few/no events were collected
hotpixel_algo.forceContinueMasking = False
# Minimum number of collected events for masking
hotpixel_algo.minEvents = 10000
# Only consider dead pixel masking when median number of hits per pixel is
# higher
hotpixel_algo.minHits = 15
# Occupancy threshold is median occupancy x multiplier
hotpixel_algo.pixelMultiplier = 7
# Set True to allow masking of hot drain lines
hotpixel_algo.maskDrains = True
# Occupancy threshold is median occupancy x multiplier
hotpixel_algo.drainMultiplier = 7
# Set True to allow masking of hot rows
hotpixel_algo.maskRows = True
# Occupancy threshold is median occupancy x multiplier
hotpixel_algo.rowMultiplier = 7

# We want to use a specific collector collecting from raw hits
hotpixel_algo.setPrefix("PXDRawHotPixelMaskCollector")

# Create a hotpixel calibration
hotpixel_cal = Calibration(
    name="PXDHotPixelMaskCalibrationAlgorithm",
    collector=hotpixel_collector,
    algorithms=hotpixel_algo,
    input_files=input_files)
hotpixel_cal.pre_collector_path = pre_hotpixel_collector_path

# Apply the map to this calibration, now the CAF doesn't have to do it
hotpixel_cal.files_to_iovs = files_to_iovs

# Here we set the AlgorithmStrategy
hotpixel_cal.strategies = SequentialRunByRun
hotpixel_cal.max_files_per_collector_job = 1
hotpixel_cal.use_central_database("Calibration_Offline_Development")

hotpixel_cal.ignored_runs = pxd_ignore_run_list
hotpixel_cal.algorithms[0].params["iov_coverage"] = iov_to_calibrate


# Landau Fit on DATA


# Create and configure the collector on beam data and its pre collector path
charge_collector = b2.register_module("PXDClusterChargeCollector")
charge_collector.param("granularity", "run")
charge_collector.param("minClusterCharge", 8)
charge_collector.param("minClusterSize", 2)
charge_collector.param("maxClusterSize", 6)
charge_collector.param("nBinsU", 4)
charge_collector.param("nBinsV", 6)


# The pre collector path on data
pre_charge_collector_path = b2.create_path()
pre_charge_collector_path.add_module(
    "Gearbox", fileName='geometry/Beast2_phase2.xml')
pre_charge_collector_path.add_module("Geometry", useDB=False)
pre_charge_collector_path.add_module("ActivatePXDPixelMasker")
pre_charge_collector_path.add_module("PXDUnpacker")
pre_charge_collector_path.add_module('PXDPostErrorChecker')
pre_charge_collector_path.add_module("PXDRawHitSorter")
pre_charge_collector_path.add_module("PXDClusterizer")


landau_algo = PXDChargeCalibrationAlgorithm()

# We can play around with algo parameters
# Minimum number of collected clusters for estimating gains
landau_algo.minClusters = 5000
# Artificial noise sigma for smearing cluster charge
landau_algo.noiseSigma = 0.6
# Force continue algorithm instead of c_notEnoughData
landau_algo.forceContinue = False
landau_algo.strategy = 1	    # 0: medians, 1: landau fit
# We want to use a specific collector
landau_algo.setPrefix("PXDClusterChargeCollector")

# create calibration
charge_cal = Calibration(
    name="PXDChargeCalibrationAlgorithm",
    collector=charge_collector,
    algorithms=landau_algo,
    input_files=input_files)

charge_cal.pre_collector_path = pre_charge_collector_path

# Apply the map to this calibration, now the CAF doesn't have to do it
charge_cal.files_to_iovs = files_to_iovs

# Here we set the AlgorithmStrategy
charge_cal.strategies = SequentialRunByRun
charge_cal.max_files_per_collector_job = 1

charge_cal.algorithms[0].params["iov_coverage"] = iov_to_calibrate
charge_cal.ignored_runs = pxd_ignore_run_list

charge_cal.use_central_database("Calibration_Offline_Development")


# gain calibration on mc

gain_collector = b2.register_module("PXDClusterChargeCollector")
gain_collector.param("granularity", "run")
gain_collector.param("minClusterCharge", 8)
gain_collector.param("minClusterSize", 2)
gain_collector.param("maxClusterSize", 6)
gain_collector.param("nBinsU", 4)
gain_collector.param("nBinsV", 6)

# The pre collector path on mc files
pre_gain_collector_path = b2.create_path()
pre_gain_collector_path.add_module(
    "Gearbox", fileName='geometry/Beast2_phase2.xml')
pre_gain_collector_path.add_module("Geometry", useDB=False)
pre_gain_collector_path.add_module("PXDDigitizer")
pre_gain_collector_path.add_module("PXDClusterizer")


# Create and configure the calibration algorithm
gain_algo = PXDGainCalibrationAlgorithm()

# We can play around with algo parameters
# Minimum number of collected clusters for estimating gains
gain_algo.minClusters = 3000
# Artificial noise sigma for smearing cluster charge
gain_algo.noiseSigma = 0.6
# Force continue algorithm instead of c_notEnoughData
gain_algo.forceContinue = False
gain_algo.strategy = 1		 # 0: median, 1: landau fit
# We want to use a specific collector
gain_algo.setPrefix("PXDClusterChargeCollector")

# Create a charge calibration
gain_cal = Calibration(
    name="PXDGainCalibrationAlgorithm",
    collector=gain_collector,
    algorithms=gain_algo,
    input_files=mc_input_files)

gain_cal.pre_collector_path = pre_gain_collector_path

# Apply the map to this calibration, now the CAF doesn't have to do it
gain_cal.files_to_iovs = mc_files_to_iovs

# Here we set the AlgorithmStrategy
gain_cal.strategies = SequentialRunByRun
gain_cal.max_files_per_collector_job = 1

gain_cal.algorithms[0].params["iov_coverage"] = iov_to_calibrate
gain_cal.ignored_runs = pxd_ignore_run_list

gain_cal.use_central_database("Calibration_Offline_Development")


# CAF

# Define dependencies. In this case: hotpixel_cal -> charge_cal -> gain_cal
charge_cal.depends_on(hotpixel_cal)
gain_cal.depends_on(charge_cal)

# create a CAF instance and add the calibration
cal_fw = CAF()
cal_fw.add_calibration(hotpixel_cal)
cal_fw.add_calibration(charge_cal)
cal_fw.add_calibration(gain_cal)
# cal_fw.backend = backends.Local(max_processes=20)
cal_fw.backend = LSF()
cal_fw.output_dir = 'pxd_calibration_results_range_{}_{}_{}'.format(
    args.runLow, args.runHigh, args.expNo)
cal_fw.run(iov=iov_to_calibrate)
