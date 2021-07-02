##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# This steering file computes PXD calibrations for energy loss (gain). The script uses the CAF framework.
# This script uses by default the SequentialRunByRun strategy and is designed to try to compute new calibration
# constants regularly to follow temporal changes of the PXD.
#
# There are options to:
# * obtain MC charge (typically done only once per given geometry & beam condition
# * obtain Data charge (for studies and cross checks)
# * derive gain calibration constants (Data/MC ratio) from Data charge computed on-the-fly and
#   MC charge from existing DB
#
# (0) Prerequisite
#
# * Hot pixel masks should be available for the given iov range, either in GT or in the specified localDB.
#
# (1) Preparation for Data
#
# The script allows you to create a list of runs that will be ignored in the calibration.
# Use this list for known bad runs.
# By default, "RunListCosmics" and "RunListBeam" files on KEKCC are read in to skip these runs.
# (Best to avoid these runs for calibration unless there exist corresponding MC samples to model them.)
#
# Before calibration, you have to put a file to IoV mapping for Data called 'file_iov_map.pkl' in your
# working directory. The mapping file should map file locations to IoV ranges and allows to
# logically address data by experiment and run numbers.  You can create such a map file by using
# the tool b2caf-filemap. See also the option --help.
#
# b2caf-filemap -m raw  -p "/hsm/belle2/bdata/Data/Raw/e0008/r*/**/*.root"
#
# (2) Preparation for MC
#
# The next step is to prepare mc runs containing PXDSimHits. That setup for the simulation
# should mimic the situation of beam data as closely as possible.
# In this version of calibration, mc file is created only once for the entire iov range.
#
# basf2 submit_create_mcruns.py -- --backend='local' --outputdir='pxd_mc_phase3' --runLow=0 --runHigh=0 --expNo=7
#
# The scripts submits the creation of mc runs to a CAF.backend (here local) for all for given run range. Runs in the
# specified runs but not found in 'file_iov_map.pkl' will be skipped. The simulated runs will be collected in the folder
# outputdir. It is best to create another mapping file for the mc data.
#
# b2caf-filemap -m metadata  -p --output-file "dummy_mc_file_iov_map.pkl" "pxd_mc_phase3/*.root"
#
# (3) Run charge calibration (measurement)
#
# A stand-alone process to collect charge and creating Cluster Charge payloads for MC is needed before
# running gain calibration on data. This needs to be done once for a given sample and the DB listing has to be
# modified by hand to make it valid for the calibration range for data.
#
# basf2 caf_pxd_datamcgain.py -- --runLow=0 --runHigh=0 --expNo=7 --mcOnly
#
# Check the output in a folder 'pxd_calibration_results_e7_range_0_0' and modify outputdb/database.txt
# such that the iov covers the entire range, e.g. 7 0 -1 -1
#
# One can run the above script with --dataOnly to get Cluster Charge payloads for data without computing gain calibration.
#
# (4) Run gain calibration
#
# Finally, a CAF script for the calibration needs to be started:
#
# basf2 caf_pxd_datamcgain.py -- --runLow=4120 --runHigh=4120 --expNo=7 /
#                                --localDB="/hsm/belle2/bdata/group/detector/PXD/calibration/mcdb_median/database.txt"
#
# The results will be collected in a folder 'pxd_calibration_results_eE_range_XY'. In order to complete the
# process, the check and uploads the outputdbs to a global tag (GT).
#
# b2conditionsdb upload pxd_calibration ./database.txt
#
# The option --help provides extensive help for the b2conditionsdb tool.
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de, maiko.takahashi@desy.de


import argparse
from tracking import add_tracking_reconstruction
from rawdata import add_unpackers
from caf.strategies import SequentialRunByRun
from caf.utils import CentralDatabase
from caf.utils import LocalDatabase
from caf.utils import ExpRun, IoV
from caf.backends import LSF
from caf.framework import Calibration, CAF
from ROOT.Belle2 import PXDDataMCGainCalibrationAlgorithm
import ROOT
import pickle
import basf2 as b2
b2.set_log_level(b2.LogLevel.INFO)


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
parser.add_argument('--maxSubRuns', default=-1, type=int,
                    help='Maximum number of subruns to use')
parser.add_argument('--localDB', default="", type=str,
                    help='path to local DB database.txt')
parser.add_argument(
    '--ignoreRuns',
    default="DefaultFromKEKCC",
    type=str,
    help='Full paths to list of runs to ignore, separate multiple by a comma , ')
parser.add_argument(
    '--mcOnly',
    dest='mcOnly',
    action="store_true",
    help='Run charge calibration for MC only, otherwise specify --dataOnly \
                          or run the full gain calibration on data using MC charge from DB')
parser.add_argument(
    '--dataOnly',
    dest='dataOnly',
    action="store_true",
    help='Run charge calibration for Data only, otherwise specify --mcOnly \
                          or run the full gain calibration on data using MC charge from DB')
parser.add_argument(
    '--useTrackClusters',
    default=0,
    type=int,
    help='Flag to use track matched clusters (=1) and apply theta angle projection to cluster charge (=2)')

args = parser.parse_args()

ROOT.gROOT.SetBatch(True)

###############
# input files #
###############

# Set the IoV range for this calibration
iov_to_calibrate = IoV(
    exp_low=args.expNo,
    run_low=args.runLow,
    exp_high=args.expNo,
    run_high=args.runHigh)

input_files = []
pxd_ignore_run_list = []

if args.mcOnly:
    # Access files_to_iovs for MC runs
    with open("mc_file_iov_map.pkl", 'br') as map_file:
        files_to_iovs = pickle.load(map_file)
    input_files = list(files_to_iovs.keys())
    print('Number selected mc input files:  {}'.format(len(input_files)))

else:

    # odd runs to ignore for data
    pxd_ignore_run_list = [ExpRun(3, 484), ExpRun(3, 485), ExpRun(3, 486), ExpRun(3, 524),  # from Phase2
                           ExpRun(
        7, 1000),    # 19 modules excluded, and 1 remaining module with high occupancy
        # problem processing one file, anyway a short 'debug' beam run
        ExpRun(8, 106),
        ExpRun(8, 676),     # LER beam lost at early point
    ]

    # load ignore run list
    ignoreRuns = args.ignoreRuns
    if "DefaultFromKEKCC" in ignoreRuns:
        dirkekcc = "/hsm/belle2/bdata/Data/Raw/e000{}/".format(args.expNo)
        ignoreRuns = dirkekcc + "RunListBeam," + dirkekcc + "RunListCosmic"
    if ignoreRuns:
        for flist in ignoreRuns.split(","):
            fignore = open(flist, 'r')
            line = fignore.readline()
            while line:
                run = line[0:line.find(',')]
                pxd_ignore_run_list.append(ExpRun(args.expNo, int(run)))
                line = fignore.readline()
            fignore.close()

        print('List of ignored runs')
        print(pxd_ignore_run_list)

    # Access files_to_iovs for beam runs
    with open("file_iov_map.pkl", 'br') as map_file:
        files_to_iovs = pickle.load(map_file)

    input_file_iov_set = set(files_to_iovs.values())
    for file_iov in input_file_iov_set:
        if iov_to_calibrate.contains(file_iov):
            subruns = [k for k, v in files_to_iovs.items() if v == file_iov]
            input_files.extend(subruns[:args.maxSubRuns])

    print('Number selected data input files:  {}'.format(len(input_files)))


###################
# Input Collector #
###################

# Charge collector for MC or data

charge_collector = b2.register_module("PXDClusterChargeCollector")
charge_collector.param("granularity", "run")
charge_collector.param("minClusterCharge", 8)
charge_collector.param("minClusterSize", 2)
charge_collector.param("maxClusterSize", 6)
charge_collector.param("nBinsU", 4)
charge_collector.param("nBinsV", 6)
# For gain calibration, collect charge for data and use MC charge from DB
if not args.mcOnly and not args.dataOnly:
    charge_collector.param(
        "chargePayloadName",
        "PXDMCClusterChargeMapPar")  # MC from DB
charge_collector.param("fillChargeHistogram", True)
charge_collector.param("matchTrack", args.useTrackClusters)

# The pre collector path for MC or data

pre_charge_collector_path = b2.create_path()
pre_charge_collector_path.add_module("Gearbox")
pre_charge_collector_path.add_module("Geometry")
if args.mcOnly:
    # only needed when starting from PXDSimHits
    pre_charge_collector_path.add_module("PXDDigitizer")
    if args.useTrackClusters:
        add_tracking_reconstruction(pre_charge_collector_path)
    else:
        pre_charge_collector_path.add_module("PXDClusterizer")
else:
    if args.useTrackClusters:
        add_unpackers(pre_charge_collector_path, ['PXD', 'SVD', 'CDC'])
        add_tracking_reconstruction(pre_charge_collector_path)
    else:
        add_unpackers(pre_charge_collector_path, ['PXD'])
        pre_charge_collector_path.add_module("PXDClusterizer")

for module in pre_charge_collector_path.modules():
    if module.name() == 'Geeometry':
        module.param('excludedComponents', ['ECL', 'KLM', 'TOP'])
    if module.name() == 'PXDUnpacker':
        module.param('SuppressErrorMask', 0xffffffff)
    if module.name() == 'PXDPostErrorChecker':
        module.param('CriticalErrorMask', 0)

#########################
# Calibration Algorithm #
#########################

datamc_algo = PXDDataMCGainCalibrationAlgorithm()

# We can play around with algo parameters
# Minimum number of collected clusters for estimating gains
datamc_algo.minClusters = 5000
# Artificial noise sigma for smearing cluster charge
datamc_algo.noiseSigma = 0.0
# Force continue algorithm instead of c_notEnoughData, set True for Cosmics
datamc_algo.forceContinue = False
datamc_algo.strategy = 0	           # 0: medians, 1: landau fit
if args.mcOnly or args.dataOnly:
    # only estimate charge MPV from median or landau fit
    datamc_algo.doCalibration = False
    if args.mcOnly:
        # payload name to store on DB for MC
        datamc_algo.chargePayloadName = "PXDMCClusterChargeMapPar"
    if args.dataOnly:
        # payload name to store on DB for data
        datamc_algo.chargePayloadName = "PXDClusterChargeMapPar"
else:
    # do gain calibration on data against MC from DB
    datamc_algo.doCalibration = True
# use histogram rather than tree input to save time
datamc_algo.useChargeHistogram = True
# We want to use a specific collector
datamc_algo.setPrefix("PXDClusterChargeCollector")

# create calibration
charge_cal = Calibration(
    name="PXDDataMCGainCalibrationAlgorithm",
    collector=charge_collector,
    algorithms=datamc_algo,
    input_files=input_files,
    pre_collector_path=pre_charge_collector_path,
    database_chain=[
        CentralDatabase("data_reprocessing_prompt"),
        CentralDatabase("pxd_calibration"),
        LocalDatabase(
            args.localDB)])

# Apply the map to this calibration, now the CAF doesn't have to do it
charge_cal.files_to_iovs = files_to_iovs

# Here we set the AlgorithmStrategy
charge_cal.strategies = SequentialRunByRun
charge_cal.max_files_per_collector_job = 1

charge_cal.algorithms[0].params["iov_coverage"] = iov_to_calibrate
charge_cal.ignored_runs = pxd_ignore_run_list

#######
# CAF #
#######

# create a CAF instance and add the calibration
cal_fw = CAF()
cal_fw.add_calibration(charge_cal)
cal_fw.backend = LSF()  # KEKCC batch
# cal_fw.backend = backends.Local(max_processes=20) # interactive
cal_fw.output_dir = 'pxd_calibration_results_e{}_range_{}_{}'.format(
    args.expNo, args.runLow, args.runHigh)
cal_fw.run(iov=iov_to_calibrate)
