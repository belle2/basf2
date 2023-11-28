#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#########################################################################
#
# Example steering file that shows how to run the module OverlapResiduals
#
# in the specific case of MC cosmic events.
#
#
# (Generation is based on generation/examples/Cosmics.py)
#
#########################################################################

import basf2
from simulation import add_simulation
from reconstruction import add_reconstruction

# Setting the random seed for particle generation
basf2.set_random_seed(0)

# Run the modules
main = basf2.create_path()
# Create Event information
main.add_module('EventInfoSetter', evtNumList=[100], runList=[1])
main.add_module('Gearbox')
main.add_module('Geometry')
main.add_module('HistoManager')
# Performs generation of specific cosmic events
main.add_module('Cosmics', level=1, ipRequirement=1, cylindricalR=16.0)
# Performs simulation
add_simulation(main)
# Performs reconstruction
add_reconstruction(main, pruneTracks=False)
# The main module: studies hits in overlapping sensors of a same VXD layer to monitor the VXD alignment
main.add_module('OverlapResiduals')
main.add_module('ProgressBar')

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
