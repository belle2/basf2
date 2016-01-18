#!/usr/bin/env python
# -*- coding: utf-8 -*-

#############################################################
# This steering file generates, simulates, and reconstructs
# a sample of 50 events and then runs the CombinedPIDPerformance
# module, which generates TEfficiency objects for performance
# monitoring.
#
# Usage: basf2 combinedPID_evtgen.py
#
# Input: ccbar.dex
# Output: CombinedPIDPerformance.root
#
# Example steering file - 2011 Belle II Collaboration
#############################################################

from basf2 import *
from simulation import *
from reconstruction import *
from beamparameters import add_beamparameters
from ROOT import Belle2

# create path
main = create_path()

# define number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [50])
main.add_module(eventinfosetter)
add_beamparameters(main, "Y4S")

# generate ccbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('userDECFile', 'ccbar.dec')
evtgeninput.param('ParentParticle', 'vpho')
main.add_module(evtgeninput)

# run the simulation
add_simulation(main)

# run the reconstruction
add_reconstruction(main)

# run a module to generate histograms to test pid performance
pidperformance = register_module('CombinedPIDPerformance')
pidperformance.param('outputFileName', 'CombinedPIDPerformance.root')
main.add_module(pidperformance)

# show progress of processing
main.add_module(register_module('ProgressBar'))

# process events and print call statistics
process(main)
print(statistics)
