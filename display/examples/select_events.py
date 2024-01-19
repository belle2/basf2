#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Demonstrates selecting events to save in the event display:
# * create 'conditionspath' with e.g. RootOutput
# * use if_true() on the display module to activate it
#
# Once started, a widget will be shown to toggle the return value,
# if the box is checked when advancing to the next event, the event
# will be saved in saved_from_display.root.

import basf2 as b2
from simulation import add_simulation

# register necessary modules
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [5000])

main = b2.Path()

# add modules to paths
main.add_module(eventinfosetter)

main.add_module('EvtGenInput')

add_simulation(main)

# add_reconstruction(main)

display = main.add_module('Display')
conditionpath = b2.Path()
conditionpath.add_module('RootOutput', outputFileName='saved_from_display.root')
display.if_true(conditionpath)

b2.process(main)
print(b2.statistics)
