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

if 'BELLE2_BACKGROUND_DIR' not in os.environ:
    B2ERROR('BELLE2_BACKGROUND_DIR variable is not set - it must contain the path to BG overlay samples')
    sys.exit()

# background overlay files
bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
if len(bg) == 0:
    B2ERROR('No files found in ', os.environ['BELLE2_BACKGROUND_DIR'])
    sys.exit()

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

# Simulation
add_simulation(main, bkgfiles=bg, bkgOverlay=True)

# set debug level for overlay executor module
for m in main.modules():
    if m.type() == "BGOverlayExecutor":
        m.logging.log_level = LogLevel.DEBUG  # comment or remove to turn off
        m.logging.debug_level = 100
        break

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
