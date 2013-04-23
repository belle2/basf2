#!/usr/bin/env python
# -*- coding: utf-8 -*-

# example steering file to produce events and show them in the display
# without stopping the simulation.
# event data is buffered in shared memory, previous events may be overwritten
# once the buffer is full.

import os
import random
from basf2 import *

# register necessary modules
evtmetagen = register_module('EvtMetaGen')

# generate one event
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [5000])

evtmetainfo = register_module('EvtMetaInfo')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('ExcludedComponents', ['ECL'])

pgun = register_module('ParticleGun')


# simulation
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# create paths
main = create_path()

# add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pgun)
main.add_module(g4sim)

cdcdigi = register_module('CDCDigitizer')
main.add_module(cdcdigi)

pxd_digi = register_module('PXDDigitizer')
main.add_module(pxd_digi)

main.add_module(register_module('PXDClusterizer'))

mctrackfinder = register_module('MCTrackFinder')
mctrackfinder.param('UsePXDHits', True)
mctrackfinder.param('UseSVDHits', True)
mctrackfinder.param('UseCDCHits', True)
main.add_module(mctrackfinder)

genfit = register_module('GenFitter')
main.add_module(genfit)

# default parameters
display = register_module('AsyncDisplay')
# make some room in the buffer when full
display.param('DiscardOldEvents', True)
main.add_module(display)

process(main)
print statistics
