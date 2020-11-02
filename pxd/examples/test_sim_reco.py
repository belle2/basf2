#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction

numEvents = 2000

# first register the modules

b2.set_random_seed(1)

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = b2.register_module('EventInfoPrinter')

evtgeninput = b2.register_module('EvtGenInput')
evtgeninput.logging.log_level = b2.LogLevel.INFO

# Create paths
main = b2.create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(evtgeninput)
add_simulation(main, components=['MagneticField', 'PXD'], usePXDDataReduction=False)
add_reconstruction(main, components=['MagneticField', 'PXD'])


main.add_module('RootOutput')

# Process events
b2.process(main)

print(b2.statistics)
