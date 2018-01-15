#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import math

# ----------------------------------------------------------------------------
# Example of generating realistic calibration pulses with TOPCalPulseGenerator
# ----------------------------------------------------------------------------

# local database with TBC constants
reset_database()
pathTo = '/group/belle2/group/detector/TOP/calibration/combined/Combined_TBCrun417x_LocaT0run4855/'  # on KEKCC
use_local_database(pathTo + "localDB/localDB.txt", pathTo + "localDB")

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

# pulse generator
calpulse = register_module('TOPCalPulseGenerator')
calpulse.param('asicChannels', [0])
calpulse.param('moduleIDs', [1])
calpulse.param('amplitude', 750.0)
main.add_module(calpulse)

# digitization
topdigi = register_module('TOPDigitizer')
topdigi.param('useSampleTimeCalibration', True)
main.add_module(topdigi)

# output
main.add_module('RootOutput')

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
