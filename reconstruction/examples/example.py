#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from HLTTrigger import add_HLT_Y4S

main = create_path()

# specify number of events to be generated in job
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])  # we want to process 10 events
main.add_module(eventinfosetter)

# generate BBbar events
from beamparameters import add_beamparameters
add_beamparameters(main, "Y4S")

main.add_module('EvtGenInput')

# detecor simulation
add_simulation(main)
# or add_simulation(main, components) to simulate a selection of detectors

# HLT L3 simulation
main.add_module('Level3')

# reconstruction
add_reconstruction(main)
# or add_reconstruction(main, components) to run the reconstruction of a selection of detectors

# HLT physics trigger
add_HLT_Y4S(main)

# full output
main.add_module('RootOutput', outputFileName='output.root')

# mdst output
add_mdst_output(main)

main.add_module('ProgressBar')
process(main)

# Print call statistics
print(statistics)
