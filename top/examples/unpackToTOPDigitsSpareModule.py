#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Unpack raw data of the spare module #1 to TOPDigits
# Usage: basf2 unpackToTOPDigitsSpareModule.py -i <input_file.sroot> -o <output_file.root>
# ---------------------------------------------------------------------------------------

import basf2 as b2

# Database
b2.conditions.override_globaltags()
b2.conditions.append_globaltag('online')

# Create path
main = b2.create_path()

# input
roinput = b2.register_module('SeqRootInput')
# roinput = b2.register_module('RootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawDetector objects (needed for PocketDAQ, can be skipped otherwise)
converter = b2.register_module('Convert2RawDet')
main.add_module(converter)

# geometry parameters
gearbox = b2.register_module('Gearbox')
gearbox.param('fileName', 'top/TOPSpareModule.xml')
main.add_module(gearbox)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer', useDB=False)

# Unpacking (format auto detection works now)
unpack = b2.register_module('TOPUnpacker')
main.add_module(unpack)

# Convert to TOPDigits
converter = b2.register_module('TOPRawDigitConverter')
converter.param('useSampleTimeCalibration', False)
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
converter.param('lookBackWindows', 28)  # depends on the run
converter.param('calibrationChannel', 0)  # if set, cal pulses will be flagged
converter.param('calpulseHeightMin', 450)  # in [ADC counts]
converter.param('calpulseHeightMax', 900)  # in [ADC counts]
converter.param('calpulseWidthMin', 2.0)  # in [ns]
converter.param('calpulseWidthMax', 6.0)  # in [ns]
main.add_module(converter)

# output
output = b2.register_module('RootOutput')
output.param('branchNames', ['TOPDigits', 'TOPRawDigits', 'TOPProductionEventDebugs',
                             'TOPProductionHitDebugs', 'TOPRawDigitsToTOPProductionHitDebugs',
                             # 'TOPRawWaveforms', 'TOPRawDigitsToTOPRawWaveforms',
                             ])
main.add_module(output)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
