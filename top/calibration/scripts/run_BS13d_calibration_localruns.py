#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# CAF calibration script: BS13d carrier shifts
# data type: local runs (laser or singe-pulse or double-pulse)
#
# usage: basf2 run_BS13d_calibration_localruns.py expNo runFirst runLast
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

# ----- those parameters need to be adjusted before running -----------------------
#
globalTag = 'data_reprocessing_prompt_rel4_patch'
data_dir = '/ghi/fs01/belle2/bdata/group/detector/TOP/2019-*/data_sroot_global/'
main_output_dir = 'top_calibration'
maxFiles = 1  # maximum number of input files per run (0 or negative means all)
look_back = 28  # look-back window setting (set to 0 if look-back setting available in DB)
#
# ---------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 4:
    print("usage: basf2", argvs[0], "experiment runFirst runLast")
    sys.exit()
experiment = int(argvs[1])
run_first = int(argvs[2])
run_last = int(argvs[3])

# Make list of files
inputFiles = []
expNo = 'e' + '{:0=4d}'.format(experiment)
for run in range(run_first, run_last + 1):
    expRun = '{:0=4d}'.format(experiment) + '.' + '{:0=5d}'.format(run)
    filename = f"{data_dir}/top.{expRun}.*.sroot"
    files = glob.glob(filename)
    if maxFiles > 0:
        for i in range(min(len(files), maxFiles)):
            inputFiles.append(files[i])
    else:
        inputFiles += files
if len(inputFiles) == 0:
    B2ERROR('No sroot files found in ' + data_dir + ' for exp=' + str(experiment) +
            ' runFirst=' + str(run_first) + ' runLast=' + str(run_last))
    sys.exit()

# Output folder
run_range = 'r' + '{:0=5d}'.format(run_first) + '-' + '{:0=5d}'.format(run_last)
output_dir = f"{main_output_dir}/BS13d-local-{expNo}-{run_range}"

# Temporary fix for BII-5431
if not os.path.isdir(main_output_dir):
    os.makedirs(main_output_dir)
    print('New folder created: ' + main_output_dir)

# Suppress messages during processing
basf2.set_log_level(basf2.LogLevel.WARNING)

# Create path
main = basf2.create_path()

# Basic modules: Input, converters, geometry, unpacker
main.add_module('SeqRootInput')
main.add_module('TOPGeometryParInitializer')
main.add_module('TOPUnpacker')
main.add_module('TOPRawDigitConverter', lookBackWindows=look_back,
                useAsicShiftCalibration=False, useChannelT0Calibration=False)

# Collector module
collector = basf2.register_module('TOPAsicShiftsBS13dCollector')

# Algorithm
algorithm = TOP.TOPAsicShiftsBS13dAlgorithm()

# Define calibration
cal = Calibration(name='TOP_BS13dCalibration', collector=collector,
                  algorithms=algorithm, input_files=inputFiles)
cal.use_central_database(globalTag)
cal.pre_collector_path = main
cal.max_files_per_collector_job = 1
cal.backend_args = {"queue": "l"}
cal.strategies = SequentialRunByRun

# Add calibration to CAF
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
