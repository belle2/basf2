#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Steering file to produce the Time Base calibration out of a pulser run
# Called by the wrapper scripts runTBCpocket.sh and runTBClocal.sh, which
# sets the proper directories for the output.
#
# Note: check the global tag!
#
# Contributors: Marko Staric, Umberto Tamponi
#

import basf2 as b2
import sys

# read parameters

argvs = sys.argv
if len(argvs) != 5:
    print('usage: basf2', argvs[0],
          '-i <file_sroot> (pocket|local) <slot> <channel> <output_dir>')
    sys.exit()

datatype = argvs[1]      # data type (pocket, local)
slot = int(argvs[2])     # slot number (1-16)
channel = int(argvs[3])  # calibration channel (0-7)
outdir = argvs[4]        # output directory path

print('data type:', datatype, ' slot:', slot, ' calibration channel:', channel,
      ' output to:', outdir)

# Define a global tag (note: the one given bellow can be out-dated!)
b2.use_central_database('data_reprocessing_proc8')

# Suppress messages and warnings during processing
b2.set_log_level(b2.LogLevel.ERROR)

# Create path
main = b2.create_path()

# input
roinput = b2.register_module('SeqRootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawDetector objects
if datatype == 'pocket':
    print('pocket DAQ data assumed')
    converter = b2.register_module('Convert2RawDet')
    main.add_module(converter)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Unpacking (format auto detection works now)
unpack = b2.register_module('TOPUnpacker')
main.add_module(unpack)

# Add multiple hits by running feature extraction offline
featureExtractor = b2.register_module('TOPWaveformFeatureExtractor')
main.add_module(featureExtractor)

# Convert to TOPDigits
converter = b2.register_module('TOPRawDigitConverter')
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
calib = b2.register_module('TOPTimeBaseCalibrator')
calib.param('moduleID', slot)
calib.param('method', 1)  # Matrix inversion or iterative
calib.param('directoryName', outdir)
calib.logging.log_level = b2.LogLevel.INFO
main.add_module(calib)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
print(b2.statistics(b2.statistics.TERM))
