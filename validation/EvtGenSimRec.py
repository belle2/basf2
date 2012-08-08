#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [100])
evtmetagen.param('RunList', [1])
evtmetagen.param('ExpList', [1])
main.add_module(evtmetagen)

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
    ]
add_simulation(main, components)

# reconstruction
add_reconstruction(main, components)

# output
simpleoutput = register_module('SimpleOutput')
simpleoutput.param('outputFileName', '../EvtGenSimRec.root')
main.add_module(simpleoutput)

process(main)

# Print call statistics
print statistics
