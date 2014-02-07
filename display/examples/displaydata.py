#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This example demonstrates showing user-defined things using DisplayData

import os
import random
from basf2 import *

# avoid race conditions beetween pyroot and GUI thread
from ROOT import PyConfig
PyConfig.StartGuiThread = False

from ROOT import Belle2
from ROOT import TVector3
import ROOT


class DisplayDataTest(Module):
    """Test DisplayData"""

    def initialize(self):
        """reimplementation of Module::initialize()."""

        Belle2.PyStoreObj("DisplayData").registerAsPersistent()

    def event(self):
        """reimplementation of Module::event()."""

        displayData = Belle2.PyStoreObj("DisplayData")
        displayData.create()

        # you can put a label at arbitrary points
        displayData.obj().addLabel("Origin", TVector3(0, 0, 0))

        # or simple points (grouped into sets)
        for i in range(10):
            displayData.obj().addPoint("set 1", TVector3(15, 15, 10 * i))
        for i in range(10):
            displayData.obj().addPoint("set 2", TVector3(10 * i, 0, 50))

        # you can add arrows
        pos = TVector3(0, 0, 100)
        displayData.obj().addArrow("x", pos, pos + TVector3(50, 0, 0))
        displayData.obj().addArrow("y", pos, pos + TVector3(0, 50, 0))
        displayData.obj().addArrow("z", pos, pos + TVector3(0, 0, 50))

        displayData.obj().addArrow("z=-2m", TVector3(100, 100, -200),
                TVector3(0, 0, -200), ROOT.kGray)

        # highlight the first MCParticle
        mcparticles = Belle2.PyStoreArray('MCParticles')
        part = mcparticles[0]
        if part != None:
            displayData.obj().select(part)


# register necessary modules
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [500])


# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('components', ['CDC', 'MagneticField'])

particlegun = register_module('ParticleGun')
particlegun.param('nTracks', 4)


# simulation
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# create paths
main = create_path()

# add modules to paths
main.add_module(eventinfosetter)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
#main.add_module(g4sim)

main.add_module(DisplayDataTest())

# default parameters
display = register_module('Display')
display.param('showAllPrimaries', True)
main.add_module(display)

process(main)
print statistics
