#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
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

set_log_level(LogLevel.ERROR)

# background (collision) files
#   wildcards can be used if there are several files of the same BG type
#   example: 'Coulomb_HER_*.root' if there are several files for Coulomb_HER

dir = '/sw/belle2/bkg/'  # change the directory name if you don't run on KEKCC
bg = [
    dir + 'Coulomb_HER_100us.root',
    dir + 'Coulomb_LER_100us.root',
    dir + 'RBB_HER_100us.root',
    dir + 'RBB_LER_100us.root',
    dir + 'Touschek_HER_100us.root',
    dir + 'Touschek_LER_100us.root',
    ]
  # change the file names if differ

# alternatively glob.glob can be used to get the list of files:
# (the directory must include only BG files!)
# bg = glob.glob(dir + '/*.root')

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

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
