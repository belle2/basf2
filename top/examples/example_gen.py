#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation

main = create_path()

# specify number of events to be generated in job
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('evtNumList', [10])  # we want to process 10 events
evtmetagen.param('runList', [1])  # from run number 1
evtmetagen.param('expList', [1])  # and experiment number 1
main.add_module(evtmetagen)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
evtgeninput.param('userDECFile', 'top/examples/B2Kpi.dec')
main.add_module(evtgeninput)

# detecor simulation
add_simulation(main)

# output
output = register_module('RootOutput')
output.param('outputFileName', 'B2Kpi_gen.root')
main.add_module(output)

process(main)

# Print call statistics
print statistics
