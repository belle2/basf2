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

main = b2.create_path()

# add modules to paths
main.add_module('EventInfoSetter', evtNumList=[5000])
main.add_module('ProgressBar')
main.add_module('EvtGenInput')
add_simulation(main)
add_reconstruction(main)

main.add_module('AsyncDisplay', discardOldEvents=True)

b2.process(main)
print(b2.statistics)
