#!/usr/bin/env python
# -*- coding: utf-8 -*-

# example steering file to produce events and show them in the display
# without stopping the simulation.
# event data is buffered in shared memory, previous events may be overwritten
# once the buffer is full.

from basf2 import *

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [5000])

evtmetainfo = register_module('EventInfoPrinter')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# EvtGen to provide generic BB events
evtgeninput = register_module('EvtGenInput')

evtgeninput.param('boost2LAB', True)

# simulation
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# create paths
main = create_path()

# add modules to paths
main.add_module(eventinfosetter)
main.add_module(evtmetainfo)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
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
display.param('discardOldEvents', True)
main.add_module(display)

process(main)
print statistics
