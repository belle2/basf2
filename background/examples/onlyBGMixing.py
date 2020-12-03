#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import os
import sys
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
import glob

# ----------------------------------------------------------------------------------
# Example of simulation/reconstruction of events with beam background only.
#
# This example runs full simulation of beam BG only events using BeamBkgMixer module,
# then runs full reconstruction and finaly writes the results to mdst file.
# ----------------------------------------------------------------------------------

b2.set_log_level(b2.LogLevel.ERROR)

if 'BELLE2_BACKGROUND_MIXING_DIR' not in os.environ:
    b2.B2ERROR('BELLE2_BACKGROUND_MIXING_DIR variable is not set - it must contain the path to BG mixing samples')
    sys.exit()

# background (collision) files
bg = glob.glob(os.environ['BELLE2_BACKGROUND_MIXING_DIR'] + '/*.root')
if len(bg) == 0:
    b2.B2ERROR('No files found in ', os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
    sys.exit()

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

# Simulation
add_simulation(main, bkgfiles=bg, bkgOverlay=False)

# Reconstruction
add_reconstruction(main)

# Mdst output
add_mdst_output(main)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
