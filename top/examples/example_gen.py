#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

main = create_path()

# specify number of events to be generated in job
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [10])  # we want to process 10 events
evtmetagen.param('RunList', [1])  # from run number 1
evtmetagen.param('ExpList', [1])  # and experiment number 1
main.add_module(evtmetagen)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
evtgeninput.param('userDECFile', 'B2Kpi.dec')
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

# output
output = register_module('RootOutput')
output.param('outputFileName', 'output.root')
main.add_module(output)

process(main)

# Print call statistics
print statistics
