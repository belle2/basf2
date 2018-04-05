#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys

set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# input
roinput = register_module('SeqRootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawTOP
# converter = register_module('Convert2RawDet')
# main.add_module(converter)

# geometry parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP needed)
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Unpacking (format auto detection should work now)
unpack = register_module('TOPUnpacker')
# unpack.param('swapBytes', True)
# unpack.param('dataFormat', 0x0301)
main.add_module(unpack)

# Convert to TOPDigits
converter = register_module('TOPRawDigitConverter')
converter.param('useSampleTimeCalibration', False)
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
main.add_module(converter)

# TOP's data quality module
histo = register_module("HistoManager")
# histo = register_module("DqmHistoManager")
# histo.param('HostName', 'localhost')
# histo.param('Port', 9991)
# histo.param('DumpInterval', 1000)
main.add_module(histo)

dqm = register_module('TOPDataQualityOnline')
main.add_module(dqm)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
