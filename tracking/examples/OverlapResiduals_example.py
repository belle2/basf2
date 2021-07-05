#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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


# Register the necessary modules

# =====================================================================================================
# Generate specific cosmic events
cosmics = basf2.register_module('Cosmics')
cosmics.param('level', 1)
cosmics.param('ipRequirement', 1)
cosmics.param('cylindricalR', 16.0)
# Setting the random seed for particle generation
basf2.set_random_seed(0)
# Create Event information
eventinfosetter = basf2.register_module('EventInfoSetter')
# Show progress of processing
progress = basf2.register_module('ProgressBar')
# Load parameters
gearbox = basf2.register_module('Gearbox')
# Create geometry
geometry = basf2.register_module('Geometry', excludedComponents=['MagneticField'])
# Manages created histograms
histos = basf2.register_module('HistoManager')
# The main module: studies hits in overlapping sensors of a same VXD layer to monitor the VXD alignment
VXDResiduals = basf2.register_module('OverlapResiduals')
# ======================================================================================================

eventinfosetter.param({'evtNumList': [100], 'runList': [1]})

# Run the modules
main = basf2.create_path()
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(histos)
# Performs generation
main.add_module(cosmics)
# Performs simulation
add_simulation(main)
# Performs reconstruction
add_reconstruction(main, pruneTracks=False)
# Runs module OverlapResiduals
main.add_module(VXDResiduals)
main.add_module(progress)

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
