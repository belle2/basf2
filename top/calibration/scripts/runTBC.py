#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Steering file to produce the Time Base calibration out of a pulser run
# Called by the wrapper scripts runTBCpocket.sh and runTBClocal.sh, which
# sets the proper directories for the output.
#
# Contributors: Marko Staric, Umberto Tamponi
#

from basf2 import *
import sys

# read parameters

argvs = sys.argv
if len(argvs) is not 5:
    print('usage: basf2', argvs[0],
          '-i <file_sroot> (pocket|local) <slot> <channel> <output_dir>')
    sys.exit()

datatype = argvs[1]      # data type (pocket, local)
slot = int(argvs[2])     # slot number (1-16)
channel = int(argvs[3])  # calibration channel (0-7)
outdir = argvs[4]        # output directory path

print('data type:', datatype, ' slot:', slot, ' calibration channel:', channel,
      ' output to:', outdir)

# Suppress messages and warnings during processing
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# input
roinput = register_module('SeqRootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawDetector objects
if datatype == 'pocket':
    print('pocket DAQ data assumed')
    converter = register_module('Convert2RawDet')
    main.add_module(converter)

# geometry parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP needed)
geometry = register_module('Geometry')
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
converter.param('calibrationChannel', channel)  # if set, cal pulses will be flagged
converter.param('calpulseHeightMin', 450)  # in [ADC counts]
converter.param('calpulseHeightMax', 900)  # in [ADC counts]
converter.param('calpulseWidthMin', 2.0)  # in [ns]
converter.param('calpulseWidthMax', 8.0)  # in [ns]
converter.param('lookBackWindows', 29)  # in number of windows
main.add_module(converter)

# TB calibrator
calib = register_module('TOPTimeBaseCalibrator')
calib.param('moduleID', slot)
calib.param('method', 1)  # Matrix inversion or iterative
calib.param('directoryName', outdir)
calib.logging.log_level = LogLevel.INFO
main.add_module(calib)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
print(statistics(statistics.TERM))
