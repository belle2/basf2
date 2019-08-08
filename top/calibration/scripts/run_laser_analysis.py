#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ------------------------------------------------------------------------
# master script to run the anaysis of laser data. To be used also for channelT0 calibration.
#
#
# Contributors: Umberto Tamponi (tamponi@to.infn.it)
# ------------------------------------------------------------------------

import basf2
import sys
import glob
from caf import backends
from caf.framework import Calibration, CAF
from ROOT import Belle2
from ROOT.Belle2 import TOP

# Suppress messages during processing
basf2.set_log_level(basf2.LogLevel.WARNING)


if len(sys.argv) < 3:
    print("You must provide one experiment number and as many run number as you like")
    print(f"Usage :  basf2 {sys.argv[0]} exp_num run_1 run_2 run_3 ... run_n")

parsed_exp_number = sys.argv[1]
parsed_run_numbers = sys.argv[2:]

exp_number = '{:04d}'.format(int(parsed_exp_number))
run_numbers = ['{:05d}'.format(int(r)) for r in parsed_run_numbers]


inputFileNames = []
for run in run_numbers:
    inputFileNames += [f for f in glob.glob(
        f"/ghi/fs01/belle2/bdata/group/detector/TOP/*/data_sroot_global/top.{exp_number}.{run}.HLT*.sroot")]

print(" ")
print("File List:")
print(inputFileNames)


# Create path
main = basf2.create_path()

# Basic modules: Input, converters, geometry, unpacker
main.add_module('RootInput')
main.add_module('Convert2RawDet')
main.add_module('Gearbox')
main.add_module('Geometry', components=['TOP'])
main.add_module('TOPUnpacker')


basf2.add_module('TOPRawDigitConverter',
                 useSampleTimeCalibration=True,
                 useChannelT0Calibration=False,
                 useModuleT0Calibration=False,
                 useCommonT0Calibration=False,
                 calpulserHeightMin=320,
                 calpulserHeightMax=680,
                 calpulserWidthMin=1.5,
                 calpulserWidthtMax=2.2,
                 calibrationChannel=0,
                 lookBackWindows=28)


laser_collector = basf2.register_module('TOPLaserCalibratorCollector')
laser_collector.param('useReferencePulse', True)
laser_collector.param('storeMCTruth', False)
laser_collector.param('refChannel', 0)  # Do not change this unless this channel is bad
laser_collector.param('refSlot', 4)  # Do not change this unless this slot is bad


algorithm = TOP.TOPLocalCalFitter()
algorithm.isMC = False
algorithm.m_TTSData = "/group/belle2/group/detector/TOP/calibration/MCreferences/TTSParametrizations.root"
algorithm.m_laserCorrections = "/group/belle2/group/detector/TOP/calibration/MCreferences/laserMCFit.root"


cal = Calibration(name='TOPLocalCalFitter', collector=laser_collector, algorithms=algorithm, input_files=inputFileNames)
cal.use_central_database('data_reprocessing_prompt')
cal.pre_collector_path = main

# How many files to use for each subjob
cal.max_files_per_collector_job = 1

# What queue to submit to for this calibration
cal.backend_args = {"queue": "l"}

cal_fw = CAF()
cal_fw.add_calibration(cal)


cal_fw.output_dir = f'laserMonitor_exp_{exp_number}_runs_{"".join([r+"_" for r in run_numbers])}fit.root'

cal_fw.backend = backends.LSF()
cal_fw.run()
