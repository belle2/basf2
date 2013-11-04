#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

main = create_path()

# specify number of events to be generated in job
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])  # we want to process 10 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
main.add_module(evtgeninput)

# detecor simulation
components = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    'BKLM',
    'ECL',
    ]
add_simulation(main, components)
# or add_simulation(main) to simulate all detectors

# reconstruction
add_reconstruction(main, components)
# or add_reconstruction(main) to run the reconstruction of all detectors

# output
output = register_module('RootOutput')
output.param('outputFileName', 'output.root')
main.add_module(output)

process(main)

# Print call statistics
print statistics
