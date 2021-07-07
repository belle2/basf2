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
# Unpack raw data in Interim FE format v2.1 to TOPDigits using time base calibration
# Usage: basf2 unpackToTOPDigitsWithTBC.py -i <input_file.sroot> -o <output_file.root>
#
# Name and location of local DB can be changed if needed
# ---------------------------------------------------------------------------------------

import basf2 as b2

# Create path
main = b2.create_path()

# input
roinput = b2.register_module('SeqRootInput')
# roinput = register_module('RootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawDetector objects
converter = b2.register_module('Convert2RawDet')
main.add_module(converter)

# geometry parameters
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP needed)
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Unpacking (format auto detection works now)
unpack = b2.register_module('TOPUnpacker')
main.add_module(unpack)

# Add multiple hits by running feature extraction offline
featureExtractor = b2.register_module('TOPWaveformFeatureExtractor')
main.add_module(featureExtractor)

# Convert to TOPDigits
converter = b2.register_module('TOPRawDigitConverter')
converter.param('useSampleTimeCalibration', True)  # enable calibration
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
converter.param('calibrationChannel', 0)  # if set, cal pulses will be flagged
converter.param('calpulseHeightMin', 450)  # in [ADC counts]
converter.param('calpulseHeightMax', 900)  # in [ADC counts]
converter.param('calpulseWidthMin', 2.0)  # in [ns]
converter.param('calpulseWidthMax', 6.0)  # in [ns]
main.add_module(converter)

# output
output = b2.register_module('RootOutput')
output.param('branchNames', ['TOPDigits', 'TOPRawDigits', 'TOPInterimFEInfos',
                             'TOPRawDigitsToTOPInterimFEInfos',
                             # 'TOPRawWaveforms', 'TOPRawWaveformsToTOPInterimFEInfos',
                             # 'TOPRawDigitsToTOPRawWaveforms',
                             ])
main.add_module(output)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
