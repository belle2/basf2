#!/usr/bin/env python
# -*- coding: utf-8 -*-

# example steering file to generate B \bar B events and show the result,
# including fitted tracks.

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [5])

eventinfoprinter = register_module('EventInfoPrinter')

# create geometry
gearbox = register_module('Gearbox')

# EvtGen to provide generic BB events
evtgeninput = register_module('EvtGenInput')

evtgeninput.param('boost2LAB', True)


# create paths
main = create_path()

# add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)

main.add_module(gearbox)
main.add_module(evtgeninput)

add_simulation(main)

# reconstruction
add_reconstruction(main)

# default parameters
display = register_module('Display')
main.add_module(display)

process(main)
print statistics
