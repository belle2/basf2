#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import basf2
from simulation import add_simulation
from reconstruction import add_reconstruction

# Use local database.
basf2.conditions.append_testing_payloads('localdb/database.txt')

# Create path.
main = basf2.create_path()

# Input.
main.add_module('RootInput', inputFileName=sys.argv[1])

# Simulation.
add_simulation(main)

# Reconstruction.
add_reconstruction(main, pruneTracks=False, add_muid_hits=True)
# Disable the time window in muid module by setting it to 1 second.
# This is necessary because the  alignment needs to be performed before
# the time calibration; if the time window is not disabled, then all
# scintillator hits are rejected.
basf2.set_module_parameters(main, 'Muid', MaxDt=1e9)

# Output.
main.add_module('RootOutput', outputFileName=sys.argv[2])

# Progress.
main.add_module('Progress')

# Processing.
basf2.process(main)

# Print call statistics
print(basf2.statistics)
