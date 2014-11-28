#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
import glob

# ----------------------------------------------------------------------------------
# Example of simulation/reconstruction of generic BBbar events with beam background.
#
# This example generates BBbar events using EvtGenInput module, runs full simulation
# with BG mixing using BeamBkgMixer module, then runs full reconstruction and finaly
# writes the results to mdst file.
# ----------------------------------------------------------------------------------

set_log_level(LogLevel.ERROR)

# background (collision) files
bg = glob.glob('/sw/belle2/bkg/*.root')  # if you run at KEKCC

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
main.add_module(evtgeninput)

# Simulation
add_simulation(main, bkgfiles=bg)

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
print statistics
