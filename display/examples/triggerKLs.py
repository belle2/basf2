#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Example steering file with a simple 'trigger' to show only a certain
# type of events. In this case, we only want to see events containing
# at least one K_L, which is achieved using a python module that sets
# a return code.
# If the return code is 0 (no K_L found), execution will continue in an empty
# path (i.e. nothing is done with the data), otherwise execution continues
# normally (K_L event is shown).
# The same thing can also be done using only C++, by calling
# Module::setReturnValue() in your module's event() function.

import os
import random
from basf2 import *
from ROOT import Belle2


class PyTrigger(Module):

    """Returns 1 if current event contains at least one K_L^0, 0 otherwise"""

    def __init__(self):
        """constructor."""

        super(PyTrigger, self).__init__()
        self.setName('PyTrigger')

    def event(self):
        """reimplementation of Module::event()."""

        mcparticles = Belle2.PyStoreArray('MCParticles')
        returnvalue = 0
        for p in mcparticles:
            if p.getPDG() == 130:
                B2INFO("found a K_L!")
                returnvalue = 1
                break
        self.return_value(returnvalue)


# register necessary modules
evtmetagen = register_module('EvtMetaGen')

# generate one event
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [20])

evtmetainfo = register_module('EvtMetaInfo')

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
main.add_module(evtmetagen)
main.add_module(evtmetainfo)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(g4sim)

########################################
# simulation is done now, so we'll put the PyTrigger module here
kltrigger = PyTrigger()
main.add_module(kltrigger)

# if PyTrigger returns 0, we'll jump into an empty path
# (skipping further modules in 'main': digitisation, tracking and display)
emptypath = create_path()
kltrigger.condition('<1', emptypath)
########################################


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
display = register_module('Display')
main.add_module(display)

process(main)
