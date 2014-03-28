#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output

# ----------------------------------------------------------------------------------
# Example of simulation/reconstruction of generic BBbar events with beam background
# ----------------------------------------------------------------------------------

set_log_level(LogLevel.ERROR)

# background (collision) files
# Note: exactly one list entry per BG type, order not important
#   wildcards can be used if there are several files of the same BG type
#   example: 'Coulomb_HER_*.root' if there are several files for Coulomb_HER
#   do not use glob.glob to get the list of files: it can result in higher background!

dir = '/sw/belle2/bkg/'  # change the directory name if you don't run on KEKCC
bkgFiles = [
    dir + 'Coulomb_HER_100us.root',
    dir + 'Coulomb_LER_100us.root',
    dir + 'RBB_HER_100us.root',
    dir + 'RBB_LER_100us.root',
    dir + 'Touschek_HER_100us.root',
    dir + 'Touschek_LER_100us.root',
    ]

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
main.add_module(evtgeninput)

# Simulation
add_simulation(main, bkgfiles=bkgFiles)

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
