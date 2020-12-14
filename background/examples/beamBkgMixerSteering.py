#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import os
import sys
from simulation import add_simulation
import glob

# ----------------------------------------------------------------------------------
# Example of BeamBkgMixer steering after add_simulation
# ----------------------------------------------------------------------------------

b2.set_log_level(b2.LogLevel.INFO)

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

# generate BBbar events
evtgeninput = b2.register_module('EvtGenInput')
main.add_module(evtgeninput)

# Simulation
add_simulation(main, bkgfiles=bg, bkgOverlay=False)

# Search for BeamBkgMixer in the path,
# then set overall scale factor to 2.5 and remove cut on ECL deposited energy
for m in main.modules():
    if m.type() == "BeamBkgMixer":
        m.param('overallScaleFactor', 2.5)
        m.param('maxEdepECL', 0)
        break
else:
    b2.B2ERROR("Could not find module of type 'BeamBkgMixer'")

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
