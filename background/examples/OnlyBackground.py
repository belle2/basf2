#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#         !!!This script is not meant as a standalone example,
#                it is called by the RofCheck script!!!
#
# This steering file fills simulation events with background using
# the background mixer. The file expects the number of events and a list
# of ROF files on the command line, so that the call should look like
# basf2 background/examples/OnlyBackground.py 20 /data/rofs/rof*.root
#
# Example steering file - 2013 Belle II Collaboration
##############################################################################

import sys
from basf2 import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# ============================================================================
# Register the necessary modules to perform a simulation
#
# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [int(sys.argv[1])], 'runList': [1]})
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry (funny, geometry initializes SensitiveDetector classes,
# which in turn create SimHits arrays.
geometry = register_module('Geometry')
# Add some background to events
bgmixer = register_module('MixBkg')
bgmixer.param('BackgroundFiles', sys.argv[2:len(sys.argv)])
bgmixer.param('AnalysisMode', False)
bgmixer.set_log_level(LogLevel.INFO)
# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'OnlyBackgroundOutput.root')

# ============================================================================
# Construct processing path

main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(bgmixer)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics
