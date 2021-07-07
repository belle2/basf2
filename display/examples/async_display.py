#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# example steering file to produce events and show them in the display
# without stopping the simulation.
# event data is buffered in shared memory, previous events may be overwritten
# once the buffer is full.

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction

# register necessary modules
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [5000])

main = b2.create_path()

# add modules to paths
main.add_module(eventinfosetter)
main.add_module('ProgressBar')

main.add_module('EvtGenInput')

# (no ECL because of unsupported solids)
components = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    'EKLM',
    'BKLM',
]
add_simulation(main, components)

# reconstruction
add_reconstruction(main, components)

# default parameters
display = b2.register_module('AsyncDisplay')
# make some room in the buffer when full
display.param('discardOldEvents', True)
main.add_module(display)

b2.process(main)
print(b2.statistics)
