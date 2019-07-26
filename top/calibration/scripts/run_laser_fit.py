#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ------------------------------------------------------------------------
# Basic module to study the time resolution using the laser light
#
# Contributors: Umberto Tamponi (tamponi@to.infn.it)
#
# usage: basf2 studyLaserResolution.py dbaddress (path|none) type (local|pocket|root)
#                                      output_name.root path_to_sroot run1 run2 ... runN
#        The run number accepts wildcards
# ------------------------------------------------------------------------


from basf2 import *
import sys
import glob
from caf.framework import Calibration, CAF
from caf import backends
from ROOT import Belle2
from ROOT.Belle2 import TOP


# Suppress messages during processing
set_log_level(LogLevel.WARNING)

inputFileNames = [f for f in glob.glob(
    "/ghi/fs01/belle2/bdata/group/detector/TOP/2019-05/data_sroot_global/top.0008.00414.HLT1*.sroot")]

print(inputFileNames)

# Create path
main = create_path()

roinput = register_module('SeqRootInput')
main.add_module(roinput)

converter = register_module('Convert2RawDet')
main.add_module(converter)


# geometry parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP needed)
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)


unpack = register_module('TOPUnpacker')
main.add_module(unpack)
converter = register_module('TOPRawDigitConverter')
converter.param('useSampleTimeCalibration', True)
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
converter.param('calpulseHeightMin', 320)  # do not specify the calpulse channel
converter.param('calpulseHeightMax', 680)  # do not specify the calpulse channel
converter.param('calpulseWidthMin', 1.5)  # do not specify the calpulse channel
converter.param('calpulseWidthMax', 2.2)  # do not specify the calpulse channel
converter.param('calibrationChannel', 0)  # do not specify the calpulse channel
converter.param('lookBackWindows', 28)  # in number of windows
main.add_module(converter)


laser_collector = register_module('TOPLaserCalibratorCollector')
laser_collector.param('useReferencePulse', True)
laser_collector.param('storeMCTruth', False)

algorithm = TOP.TOPLocalCalFitter()
algorithm.isMC = False
algorithm.m_TTSData = "/home/belle/tamponi/LaserMCData/TTSParametrizations.root"
algorithm.m_laserCorrections = "/home/belle/tamponi/LaserMCData/laserMCFit.root"


cal = Calibration(name='TOPLocalCalFitter', collector=laser_collector, algorithms=algorithm, input_files=inputFileNames)
cal.use_central_database('data_reprocessing_prompt')
cal.pre_collector_path = main
# How many files to use for each subjob
cal.max_files_per_collector_job = 1
# What queue to submit to for this calibration
cal.backend_args = {"queue": "s"}

cal_fw = CAF()
cal_fw.add_calibration(cal)

cal_fw.output_dir = 'testLaserCal_exp8run414_noChannelT0'

cal_fw.backend = backends.LSF()
cal_fw.run()
