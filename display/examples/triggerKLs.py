#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Example steering file with a simple 'trigger' to show only a certain
# type of events. In this case, we only want to see events containing
# at least one K_L, which is achieved using a python module that sets
# a return code.
# If the return code is 0 (no K_L found), execution will continue in an empty
# path (i.e. nothing is done with the data), otherwise execution continues
# normally (K_L event is shown).
# The same thing can also be done using only C++, by calling
# Module::setReturnValue() in your module's event() function.

import basf2 as b2
from ROOT import Belle2
from simulation import add_simulation
from reconstruction import add_reconstruction


class PyTrigger(b2.Module):

    """Returns 1 if current event contains at least one K_L^0, 0 otherwise"""

    def initialize(self):
        """reimplementation of Module::initialize()."""

        Belle2.PyStoreObj("DisplayData").registerInDataStore()

    def event(self):
        """reimplementation of Module::event()."""

        self.return_value(0)
        mcparticles = Belle2.PyStoreArray('MCParticles')
        for p in mcparticles:
            if abs(p.getPDG()) == 130:
                b2.B2INFO('found a K_L!')
                self.return_value(1)

                # also select the object in the display
                displayData = Belle2.PyStoreObj("DisplayData")
                displayData.create()
                displayData.obj().select(p)
                break


main = b2.create_path()
eventinfosetter = main.add_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [2000])

eventinfoprinter = main.add_module('EventInfoPrinter')

evtgeninput = main.add_module('EvtGenInput')

components = ['CDC', 'KLM', 'MagneticField']
add_simulation(main, components)

########################################
# simulation is done now, so we'll put the PyTrigger module here
kltrigger = PyTrigger()
main.add_module(kltrigger)

# if PyTrigger returns 0, we'll jump into an empty path
# (skipping further modules in 'main': digitisation, tracking and display)
emptypath = b2.create_path()
kltrigger.if_false(emptypath)
########################################

add_reconstruction(main, components)

# default parameters
display = main.add_module('Display')

b2.process(main)
print(b2.statistics)
