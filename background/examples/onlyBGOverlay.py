#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
import glob
import sys

# ---------------------------------------------------------------------------------
# Example of simulation/reconstruction of events with beam background only.
#
# This example runs full simulation of beam BG only events using BGOverlayExecutor,
# then runs full reconstruction and finaly writes the results to mdst file.
# ---------------------------------------------------------------------------------

set_log_level(LogLevel.ERROR)

# Define a file with measured BG for overlay
bg = 'BGforOverlay.root'
if not os.path.exists(bg):
    print(bg + ' not found')
    print('You can prepare the sample by: basf2 background/examples/makeBGOverlayFile.py')
    sys.exit(1)

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})
main.add_module(eventinfosetter)

# Overlay input (before process forking!)
bginput = register_module('BGOverlayInput')
bginput.param('inputFileNames', [bg])
main.add_module(bginput)

# Simulation
add_simulation(main)

# BG Overlay (after digitizers)
overlay = register_module('BGOverlayExecutor')
overlay.logging.log_level = LogLevel.DEBUG  # comment or remove to turn off
overlay.logging.debug_level = 100
main.add_module(overlay)

# Reconstruction
add_reconstruction(main)

# Mdst output
add_mdst_output(main)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
