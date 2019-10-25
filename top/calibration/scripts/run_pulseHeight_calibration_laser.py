#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# CAF calibration script: pulse height distributions and threshold efficiencies
# data type: local runs with laser
#
# usage: basf2 run_pulseHeight_calibration_laser.py expNo run_1 run_2 ... run_n
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
globalTag = 'data_reprocessing_prompt_rel4_patchb'
data_dir = '/ghi/fs01/belle2/bdata/group/detector/TOP/2019-*/data_sroot_global/'
main_output_dir = 'top_calibration'
look_back = 28  # look-back window setting (set to 0 if look-back setting available in DB)
t_min = -50.0  # [ns] time window low of laser signal
t_max = 0.0    # [ns] time window high of laser signal
#
# ---------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 3:
    print("usage: basf2", argvs[0], "experiment run_1 run_2 ... run_n")
    sys.exit()
experiment = int(argvs[1])
run_numbers = sorted([int(r) for r in argvs[2:]])
run_first = run_numbers[0]
run_last = run_numbers[-1]

# Make list of files
inputFiles = []
expNo = 'e' + '{:0=4d}'.format(experiment)
for run in run_numbers:
    expRun = '{:0=4d}'.format(experiment) + '.' + '{:0=5d}'.format(run)
    filename = f"{data_dir}/top.{expRun}.*.sroot"
    inputFiles += glob.glob(filename)
if len(inputFiles) == 0:
    runs = "".join([str(r) + "," for r in run_numbers])[:-1]
    B2ERROR(f'No sroot files found in {data_dir} for exp={str(experiment)} runs={runs}')
    sys.exit()

# Output folder
run_range = 'r' + '{:0=5d}'.format(run_first) + '-' + '{:0=5d}'.format(run_last)
output_dir = f"{main_output_dir}/pulseHeight-laser-{expNo}-{run_range}"

# Temporary fix for BII-5431
if not os.path.isdir(main_output_dir):
    os.makedirs(main_output_dir)
    print('New folder created: ' + main_output_dir)

# Suppress messages during processing
basf2.set_log_level(basf2.LogLevel.WARNING)


def pulseHeight_calibration():
    ''' pulse-height parametrizations and threshold efficiencies '''

    # Create path
    main = basf2.create_path()

    # Basic modules: Input, converters, geometry, unpacker
    main.add_module('SeqRootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPUnpacker')
    main.add_module('TOPRawDigitConverter', lookBackWindows=look_back)

    # Collector module
    collector = basf2.register_module('TOPPulseHeightCollector')
    collector.param('timeWindow', [t_min, t_max])

    # Algorithm
    algorithm = TOP.TOPPulseHeightAlgorithm()

    # Define calibration
    cal = Calibration(name='TOP_pulseHeightCalibration', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    cal.use_central_database(globalTag)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SingleIOV

    return cal


# Add calibration to CAF
cal_fw = CAF()
cal = pulseHeight_calibration()
cal.backend_args = {"queue": "l"}
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
