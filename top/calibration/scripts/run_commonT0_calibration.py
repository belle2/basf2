#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# CAF calibration script: common T0
# data type: cdst (bhabha or dimuon)
#
# usage: basf2 run_commonT0_calibration.py expNo runFirst runLast [sample method]
#        sample = bhabha/dimuon (D = bhabha)
#        method = BF/LL (D = BF)
#
# author: M. Staric
# ---------------------------------------------------------------------------------------

import basf2
import sys
import os
import glob
from caf import backends
from caf.framework import Calibration, CAF
from caf.strategies import SequentialRunByRun, SingleIOV, SimpleRunByRun
from ROOT import Belle2
from ROOT.Belle2 import TOP
from basf2 import B2ERROR

# ----- those parameters need to be adjusted before running -----------------------------
#
globalTag = 'data_reprocessing_prompt_rel4_patch'
data_dir = '/group/belle2/dataprod/Data/release-03-02-02/DB00000635/proc00000009'
bhabha_skim_dir = 'skim/hlt_bhabha/cdst/sub00'
dimuon_skim_dir = 'offskim/offskim_mumutop/cdst/sub00'
main_output_dir = 'top_calibration'
default_sample = 'bhabha'
default_method = 'BF'
#
# ---------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 4:
    print("usage: basf2", argvs[0], "experiment runFirst runLast [sample method]")
    print("       sample = bhabha/dimuon (D = bhabha)")
    print("       method = BF/LL (D = BF)")
    sys.exit()
experiment = int(argvs[1])
run_first = int(argvs[2])
run_last = int(argvs[3])
sample = default_sample
method = default_method

if len(argvs) == 5:
    sample = argvs[4]
elif len(argvs) > 5:
    sample = argvs[4]
    method = argvs[5]

if sample == 'bhabha':
    skim_dir = bhabha_skim_dir
elif sample == 'dimuon':
    skim_dir = dimuon_skim_dir
else:
    B2ERROR("Invalid sample name: " + sample)
    sys.exit()

# Make list of files
inputFiles = []
expNo = 'e' + '{:0=4d}'.format(experiment)
for run in range(run_first, run_last + 1):
    runNo = 'r' + '{:0=5d}'.format(run)
    filename = f"{data_dir}/{expNo}/*/{runNo}/{skim_dir}/cdst*.root"
    inputFiles += glob.glob(filename)

if len(inputFiles) == 0:
    B2ERROR('No cdst files found in ' + data_dir + ' for exp=' + str(experiment) +
            ' runFirst=' + str(run_first) + ' runLast=' + str(run_last) +
            ' (skim_dir=' + skim_dir + ')')
    sys.exit()

# Output folder
run_range = 'r' + '{:0=5d}'.format(run_first) + '-' + '{:0=5d}'.format(run_last)
output_dir = f"{main_output_dir}/commonT0-{sample}-{method}-{expNo}-{run_range}"

# Temporary fix for BII-5431
if not os.path.isdir(main_output_dir):
    os.makedirs(main_output_dir)
    print('New folder created: ' + main_output_dir)

# Suppress messages during processing
basf2.set_log_level(basf2.LogLevel.WARNING)


def commonT0_calibration_BF():
    ''' common T0 calibration with method BF '''

    # Create path
    main = basf2.create_path()

    # Basic modules
    main.add_module('RootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
    main.add_module('TOPChannelMasker')
    main.add_module('TOPBunchFinder', usePIDLikelihoods=True, subtractRunningOffset=False)

    # Collector module
    collector = basf2.register_module('TOPCommonT0BFCollector')

    # Algorithm
    algorithm = TOP.TOPCommonT0BFAlgorithm()

    # Define calibration
    cal = Calibration(name='TOP_commonT0Calibration', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    cal.use_central_database(globalTag)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SequentialRunByRun

    return cal


def commonT0_calibration_LL():
    ''' common T0 calibration with method LL '''

    # Create path
    main = basf2.create_path()

    # Basic modules
    main.add_module('RootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
    main.add_module('TOPChannelMasker')
    main.add_module('TOPBunchFinder', usePIDLikelihoods=True, subtractRunningOffset=False)

    # Collector module
    collector = basf2.register_module('TOPCommonT0LLCollector')
    collector.param('sample', sample)

    # Algorithm
    algorithm = TOP.TOPCommonT0LLAlgorithm()

    # Define calibration
    cal = Calibration(name='TOP_commonT0Calibration', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    cal.use_central_database(globalTag)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SequentialRunByRun

    return cal


# Add calibration to CAF
cal_fw = CAF()

if method == 'BF':
    cal = commonT0_calibration_BF()
elif method == 'LL':
    cal = commonT0_calibration_LL()
else:
    B2ERROR('Invalid method name: ' + method)
    sys.exit()

cal.backend_args = {"queue": "l"}
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
