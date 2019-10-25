#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# CAF calibration script: module T0
# data type: cdst (bhabha or dimuon)
#
# usage: basf2 run_moduleT0_calibration.py expNo runFirst runLast [sample]
#        sample = bhabha/dimuon (D = bhabha)
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
#
# ---------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 4:
    print("usage: basf2", argvs[0], "experiment runFirst runLast [sample]")
    print("       sample = bhabha/dimuon (D = bhabha)")
    sys.exit()
experiment = int(argvs[1])
run_first = int(argvs[2])
run_last = int(argvs[3])
sample = default_sample

if len(argvs) > 4:
    sample = argvs[4]

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
output_dir = f"{main_output_dir}/moduleT0-{sample}-{expNo}-{run_range}"

# Temporary fix for BII-5431
if not os.path.isdir(main_output_dir):
    os.makedirs(main_output_dir)
    print('New folder created: ' + main_output_dir)

# Suppress messages during processing
basf2.set_log_level(basf2.LogLevel.WARNING)


def moduleT0_calibration_DeltaT():
    ''' rough module T0 calibration with method DeltaT '''

    # Create path
    main = basf2.create_path()

    # Basic modules
    main.add_module('RootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
    main.add_module('TOPChannelMasker')
    main.add_module('TOPBunchFinder', usePIDLikelihoods=True, subtractRunningOffset=False)

    # Collector module
    collector = basf2.register_module('TOPModuleT0DeltaTCollector')

    # Algorithm
    algorithm = TOP.TOPModuleT0DeltaTAlgorithm()

    # Define calibration
    cal = Calibration(name='TOP_moduleT0_rough', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    cal.use_central_database(globalTag)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SingleIOV

    return cal


def moduleT0_calibration_LL():
    ''' final module T0 calibration with method LL '''

    # Create path
    main = basf2.create_path()

    # Basic modules
    main.add_module('RootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPTimeRecalibrator', subtractBunchTime=False)
    main.add_module('TOPChannelMasker')
    main.add_module('TOPBunchFinder', usePIDLikelihoods=True, subtractRunningOffset=False)

    # Collector module
    collector = basf2.register_module('TOPModuleT0LLCollector')
    collector.param('sample', sample)

    # Algorithm
    algorithm = TOP.TOPModuleT0LLAlgorithm()

    # Define calibration
    cal = Calibration(name='TOP_moduleT0_final', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    cal.use_central_database(globalTag)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SingleIOV

    return cal


# Define calibrations
cal1 = moduleT0_calibration_DeltaT()
cal2 = moduleT0_calibration_LL()
cal1.backend_args = {"queue": "l"}
cal2.backend_args = {"queue": "l"}
cal2.depends_on(cal1)

# Add calibrations to CAF
cal_fw = CAF()
cal_fw.add_calibration(cal1)
cal_fw.add_calibration(cal2)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
