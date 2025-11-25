#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------
# Simulation of cosmic data
# ---------------------------------------------------------------------------------

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_cosmics_reconstruction

# Suppress messages and warnings during processing:
# b2.set_log_level(b2.LogLevel.ERROR)

# Create path
main = b2.create_path()

# Event info setter
main.add_module('EventInfoSetter')

# Cosmic generator
main.add_module('Cosmics')

# Detector simulation
add_simulation(main)

# reconstruction
add_cosmics_reconstruction(main, components=['CDC'], merge_tracks=False)

# output
output = b2.register_module('RootOutput')
output.param('branchNames', ['TOPDigits', 'ExtHits', 'Tracks', 'TrackFitResults',
                             'MCParticles', 'TOPBarHits', 'EventT0'])
main.add_module(output)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
