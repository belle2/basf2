#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# CAF calibration script: channel T0 with laser
# data type: local runs with laser
#
# usage: basf2 run_channelT0_laser_calibration.py expNo run_1 run_2 ... run_n
# ---------------------------------------------------------------------------------------

import basf2
import sys
import os
import glob
from caf import backends
from caf.framework import Calibration, CAF
from caf.strategies import SingleIOV
from ROOT.Belle2 import TOP
from basf2 import B2ERROR

# ----- those parameters need to be adjusted before running -----------------------
#
globalTags = ['online']  # highest priority first
localDBs = []  # highest priority first, local DB's have higher priority than global tags
data_dir = '/ghi/fs01/belle2/bdata/group/detector/TOP/2019-*/data_sroot_global/'
main_output_dir = 'top_calibration'
look_back = 30  # look-back window setting (set to 0 if look-back setting available in DB)
calpulse_min_time = 50.0  # calibration pulse selection in time: minimal time [ns]
calpulse_max_time = 120.0  # calibration pulse selection in time: maximal time [ns]
tts_file = '/group/belle2/group/detector/TOP/calibration/MCreferences/TTSParametrizations.root'
laser_mc_fit = '/group/belle2/group/detector/TOP/calibration/MCreferences/laserMCFit.root'
fit_mode = 'calibration'  # can be either monitoring, MC or calibration
sroot_format = False  # on True data in sroot format, on False data in root format
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
expNo = 'e' + f'{experiment:04d}'
for run in run_numbers:
    expRun = f'{experiment:04d}' + '.' + f'{run:05d}'
    if sroot_format:
        filename = f"{data_dir}/top.{expRun}.*.sroot"
    else:
        filename = f"{data_dir}/top.{expRun}.*.root"
    inputFiles += glob.glob(filename)
if len(inputFiles) == 0:
    runs = "".join([str(r) + "," for r in run_numbers])[:-1]
    B2ERROR(f'No root files found in {data_dir} for exp={str(experiment)} runs={runs}')
    sys.exit()

# Check the existence of additional input files
if not os.path.isfile(tts_file):
    B2ERROR(f"File {tts_file} not found")
    sys.exit()
if not os.path.isfile(laser_mc_fit):
    B2ERROR(f"File {laser_mc_fit} not found")
    sys.exit()

# Output folder name
run_range = 'r' + f'{run_first:05d}' + '-' + f'{run_last:05d}'
output_dir = f"{main_output_dir}/channelT0-local-{expNo}-{run_range}"

# Suppress messages during processing
# basf2.set_log_level(basf2.LogLevel.WARNING)


def channelT0_calibration(sroot=False):
    '''
    calibration of channel T0 with laser data
    :param sroot: True if input files are in sroot format, False if in root format
    '''

    #   create path
    main = basf2.create_path()

    #   basic modules
    if sroot:
        main.add_module('SeqRootInput')
    else:
        main.add_module('RootInput')
    main.add_module('TOPGeometryParInitializer')
    main.add_module('TOPUnpacker')
    main.add_module('TOPRawDigitConverter',
                    useSampleTimeCalibration=True,
                    useAsicShiftCalibration=True,
                    useChannelT0Calibration=False,
                    useModuleT0Calibration=False,
                    useCommonT0Calibration=False,
                    calpulseHeightMin=200,
                    calpulseHeightMax=680,
                    calpulseWidthMin=1.5,
                    calpulseWidthMax=2.2,
                    calpulseTimeMin=calpulse_min_time,
                    calpulseTimeMax=calpulse_max_time,
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
    algorithm.setFitMode(fit_mode)
    algorithm.setTTSFileName(tts_file)
    algorithm.setFitConstraintsFileName(laser_mc_fit)

    #   define calibration
    cal = Calibration(name='TOP_channelT0', collector=collector,
                      algorithms=algorithm, input_files=inputFiles)
    for globalTag in reversed(globalTags):
        cal.use_central_database(globalTag)
    for localDB in reversed(localDBs):
        cal.use_local_database(localDB)
    cal.pre_collector_path = main
    cal.max_files_per_collector_job = 1
    cal.strategies = SingleIOV  # merge all runs together to gain statistics
    return cal


# Define calibrations
cal = channelT0_calibration(sroot_format)
cal.backend_args = {"queue": "l"}

# Add calibrations to CAF
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
