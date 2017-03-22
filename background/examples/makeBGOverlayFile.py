#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
import glob
import sys

# -----------------------------------------------------------------------------------
# This example is for preparing a root file for BG overlay using simulated BG samples
# Output file: BGforOverlay.root
# Number of events can be overwritten with -n
# -----------------------------------------------------------------------------------

set_log_level(LogLevel.ERROR)

if 'BELLE2_BACKGROUND_DIR' not in os.environ:
    print('BELLE2_BACKGROUND_DIR variable is not set - it must contain the path to BG samples')
    sys.exit()

bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
if len(bg) == 0:
    print('No files found in ', os.environ['BELLE2_BACKGROUND_DIR'])
    sys.exit()

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})
main.add_module(eventinfosetter)

# Simulation
add_simulation(main, bkgfiles=bg)

# Output: digits and clusters only
output = register_module('RootOutput')
output.param('outputFileName', 'BGforOverlay.root')
output.param('branchNames', ['PXDDigits', 'SVDDigits', 'CDCHits', 'TOPDigits',
                             'ARICHDigits', 'ECLDigits', 'BKLMDigits', 'EKLMDigits'])
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
