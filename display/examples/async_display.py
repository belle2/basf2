#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# example steering file to produce events and show them in the display
# without stopping the simulation.
# event data is buffered in shared memory, previous events may be overwritten
# once the buffer is full.

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [5000])

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')


# EvtGen to provide generic BB events
evtgeninput = register_module('EvtGenInput')


# simulation
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# create paths
main = create_path()

# add modules to paths
main.add_module(eventinfosetter)
main.add_module('ProgressBar')

main.add_module(evtgeninput)
main.add_module(gearbox)
main.add_module(geometry)


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

# reconstruction
add_reconstruction(main, components)

# default parameters
display = register_module('AsyncDisplay')
# make some room in the buffer when full
display.param('discardOldEvents', True)
main.add_module(display)

process(main)
print(statistics)
