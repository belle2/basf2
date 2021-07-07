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
# Example of generating double calibration pulses with TOPDoublePulseGenerator
# ----------------------------------------------------------------------------

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])
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
calpulse = b2.register_module('TOPDoublePulseGenerator')
calpulse.param('asicChannels', [0])
calpulse.param('moduleIDs', [1])
# intervals = [math.exp(-float(i)/99.0)+1 for i in range(128)]
# intervals = intervals + intervals
# calpulse.param('sampleTimeIntervals', intervals)
calpulse.param('useDatabase', True)
calpulse.param('outputFileName', 'usedSampleTimes.root')
main.add_module(calpulse)

# output
main.add_module('RootOutput')

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
