#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# CAF calibration script: channel T0 with laser (including pre-calibration of BS13d)
# data type: local runs with laser
#
# usage: basf2 run_channelT0_laser_calibration.py expNo run_1 run_2 ... run_n
#
# authors: U. Tamponi, M. Staric
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
look_back = 28  # look-back window setting (set to 0 if look-back setting available in DB)
tts_file = '/group/belle2/group/detector/TOP/calibration/MCreferences/TTSParametrizations.root'
laser_mc_fit = '/group/belle2/group/detector/TOP/calibration/MCreferences/laserMCFit.root'
fit_mode = 'calibration'  # can be either monitoring, MC or calibration
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

# Check the existence of additional input files
if not os.path.isfile(tts_file):
    B2ERROR(f"File {tts_file} not found")
    sys.exit()
if not os.path.isfile(laser_mc_fit):
    B2ERROR(f"File {laser_mc_fit} not found")
    sys.exit()

# Output folder
run_range = 'r' + '{:0=5d}'.format(run_first) + '-' + '{:0=5d}'.format(run_last)
output_dir = f"{main_output_dir}/channelT0-local-{expNo}-{run_range}"

# Temporary fix for BII-5431
if not os.path.isdir(main_output_dir):
    os.makedirs(main_output_dir)
    print('New folder created: ' + main_output_dir)

# Suppress messages during processing
basf2.set_log_level(basf2.LogLevel.WARNING)


def BS13d_calibration():
    ''' calibration of carrier shifts of BS13d with laser data '''

    #   create path
    main = basf2.create_path()

    #   add basic modules
    main.add_module('SeqRootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPUnpacker')
    main.add_module('TOPRawDigitConverter', lookBackWindows=look_back,
                    useAsicShiftCalibration=False, useChannelT0Calibration=False)

    #   collector module
    collector = basf2.register_module('TOPAsicShiftsBS13dCollector')

    #   algorithm
    algorithm = TOP.TOPAsicShiftsBS13dAlgorithm()

    #   define calibration
    cal = Calibration(name='TOP_BS13dCalibration', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    cal.use_central_database(globalTag)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.backend_args = {"queue": "l"}
    cal.strategies = SequentialRunByRun  # in case of power-cycle between the runs
    return cal


def channelT0_calibration():
    ''' calibration of channel T0 with laser data '''

    #   create path
    main = basf2.create_path()

    #   basic modules
    main.add_module('SeqRootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPUnpacker')
    main.add_module('TOPRawDigitConverter',
                    useSampleTimeCalibration=True,
                    useAsicShiftCalibration=True,
                    useChannelT0Calibration=False,
                    useModuleT0Calibration=False,
                    useCommonT0Calibration=False,
                    calpulseHeightMin=320,
                    calpulseHeightMax=680,
                    calpulseWidthMin=1.5,
                    calpulseWidthMax=2.2,
                    calibrationChannel=0,
                    lookBackWindows=look_back)

    #    collector module
    collector = basf2.register_module('TOPLaserCalibratorCollector')
    collector.param('useReferencePulse', True)
    collector.param('storeMCTruth', False)
    collector.param('refChannel', 0)  # Do not change this unless this channel is bad
    collector.param('refSlot', 4)  # Do not change this unless this slot is bad
    # collector.param('pulserDeltaT', 37.5)  # In some runs taken in fall 2019 the delay has been increased

    #    algorithm
    algorithm = TOP.TOPLocalCalFitter()
    algorithm.setFitMode(fitMode)
    algorithm.setTTSFileName(tts_file)
    algorithm.setFitConstraintsFileName(laser_mc_fit)

    #   define calibration
    cal = Calibration(name='TOP_channelT0', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    cal.use_central_database(globalTag)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.backend_args = {"queue": "l"}
    cal.strategies = SingleIOV  # merge all runs together to gain statistics
    return cal


# Add calibrations to CAF
cal_fw = CAF()
cal1 = BS13d_calibration()
cal2 = channelT0_calibration()
cal2.depends_on(cal1)
cal_fw.add_calibration(cal1)
cal_fw.add_calibration(cal2)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
