#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2

b2.set_log_level(b2.LogLevel.ERROR)

# Create path
main = b2.create_path()

# input
roinput = b2.register_module('SeqRootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawTOP
# converter = register_module('Convert2RawDet')
# main.add_module(converter)

# geometry parameters
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP needed)
geometry = b2.register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Unpacking
unpack = b2.register_module('TOPUnpacker')
unpack.param('swapBytes', True)
unpack.param('dataFormat', 0x0301)
main.add_module(unpack)

# Convert to TOPDigits
converter = b2.register_module('TOPRawDigitConverter')
converter.param('useSampleTimeCalibration', False)
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
main.add_module(converter)

# TOP's data quality module
# histo = register_module("HistoManager")
histo = b2.register_module("DqmHistoManager")
histo.param('HostName', 'localhost')
histo.param('Port', 9991)
histo.param('DumpInterval', 1000)
main.add_module(histo)

dqm = b2.register_module('TOPDataQualityOnline')
main.add_module(dqm)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
