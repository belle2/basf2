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

# ----------------------------------------------------------------------------
# Example of running time base calibration using simulated double pulses
# ----------------------------------------------------------------------------

# slot number to calibrate
moduleID = 5

# use realistic cal pulses (waveforms) or not
realistic = True

# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.ERROR)

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10000])
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['TOP'])
main.add_module(geometry)

# pulse generator
if realistic:
    # generator
    calpulse = b2.register_module('TOPCalPulseGenerator')
    calpulse.param('asicChannels', [0])
    calpulse.param('moduleIDs', [moduleID])
    calpulse.param('amplitude', 750.0)
    main.add_module(calpulse)

    # digitization
    topdigi = b2.register_module('TOPDigitizer')
    topdigi.param('useSampleTimeCalibration', True)
    main.add_module(topdigi)
else:
    # generator
    calpulse = b2.register_module('TOPDoublePulseGenerator')
    calpulse.param('asicChannels', [0])
    calpulse.param('moduleIDs', [moduleID])
    calpulse.param('useDatabase', True)
    calpulse.param('outputFileName', 'usedSampleTimes.root')
    main.add_module(calpulse)


# TB calibrator
calib = b2.register_module('TOPTimeBaseCalibrator')
calib.param('moduleID', moduleID)
calib.param('minTimeDiff', 40)
calib.param('maxTimeDiff', 80)
calib.param('directoryName', 'tbc')
calib.param('method', 1)
calib.param('useFallingEdge', False)
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
