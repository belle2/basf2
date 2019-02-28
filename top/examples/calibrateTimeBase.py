#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import os
import math

# ----------------------------------------------------------------------------
# Example of running time base calibration using simulated double pulses
# ----------------------------------------------------------------------------

# slot number to calibrate
moduleID = 5

# use realistic cal pulses (waveforms) or not
realistic = True

# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10000])
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['TOP'])
main.add_module(geometry)

# pulse generator
if realistic:
    # generator
    calpulse = register_module('TOPCalPulseGenerator')
    calpulse.param('asicChannels', [0])
    calpulse.param('moduleIDs', [moduleID])
    calpulse.param('amplitude', 750.0)
    main.add_module(calpulse)

    # digitization
    topdigi = register_module('TOPDigitizer')
    topdigi.param('useSampleTimeCalibration', True)
    main.add_module(topdigi)
else:
    # generator
    calpulse = register_module('TOPDoublePulseGenerator')
    calpulse.param('asicChannels', [0])
    calpulse.param('moduleIDs', [moduleID])
    calpulse.param('useDatabase', True)
    calpulse.param('outputFileName', 'usedSampleTimes.root')
    main.add_module(calpulse)


# TB calibrator
calib = register_module('TOPTimeBaseCalibrator')
calib.param('moduleID', moduleID)
calib.param('minTimeDiff', 40)
calib.param('maxTimeDiff', 80)
calib.param('directoryName', 'tbc')
calib.param('method', 1)
calib.param('useFallingEdge', False)
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
