#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Unpack raw data in Interim FE format v2.1 to TOPDigits
# Usage: basf2 unpackToTOPDigits.py -i <input_file.sroot> -o <output_file.root>
# ---------------------------------------------------------------------------------------

from basf2 import *

# Create path
main = create_path()

# input
roinput = register_module('SeqRootInput')
# roinput = register_module('RootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawDetector objects
converter = register_module('Convert2RawDet')
main.add_module(converter)

# geometry parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP needed)
geometry = register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Unpacking (format auto detection works now)
unpack = register_module('TOPUnpacker')
main.add_module(unpack)

# Add multiple hits by running feature extraction offline
featureExtractor = register_module('TOPWaveformFeatureExtractor')
main.add_module(featureExtractor)

# Convert to TOPDigits
converter = register_module('TOPRawDigitConverter')
converter.param('useSampleTimeCalibration', False)
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
converter.param('lookBackWindows', 28)  # depends on the run (28 for local runs, 220 for GCR)
converter.param('calibrationChannel', 0)  # if set, cal pulses will be flagged
converter.param('calpulseHeightMin', 450)  # in [ADC counts]
converter.param('calpulseHeightMax', 900)  # in [ADC counts]
converter.param('calpulseWidthMin', 2.0)  # in [ns]
converter.param('calpulseWidthMax', 6.0)  # in [ns]
main.add_module(converter)

# output
output = register_module('RootOutput')
output.param('branchNames', ['TOPDigits', 'TOPRawDigits', 'TOPInterimFEInfos',
                             'TOPRawDigitsToTOPInterimFEInfos',
                             # 'TOPRawWaveforms', 'TOPRawWaveformsToTOPInterimFEInfos',
                             # 'TOPRawDigitsToTOPRawWaveforms',
                             ])
main.add_module(output)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
