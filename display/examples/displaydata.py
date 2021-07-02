#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This example demonstrates showing user-defined things using DisplayData

import basf2 as b2

from ROOT import Belle2
from ROOT import TVector3
import ROOT


class DisplayDataTest(b2.Module):
    """Test DisplayData"""

    def initialize(self):
        """reimplementation of Module::initialize()."""

        Belle2.PyStoreObj("DisplayData").registerInDataStore()

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

        displayData.obj().addArrow("z=-2m", TVector3(100, 100, -200), TVector3(0, 0, -200), ROOT.kGray)

        # highlight the first MCParticle
        mcparticles = Belle2.PyStoreArray('MCParticles')
        part = mcparticles[0]
        if part is not None:
            displayData.obj().select(part)


# register necessary modules
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [500])


# create geometry
gearbox = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')
geometry.param('components', ['CDC', 'MagneticField'])

particlegun = b2.register_module('ParticleGun')
particlegun.param('nTracks', 4)


# simulation
g4sim = b2.register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = b2.LogLevel.ERROR

# create paths
main = b2.create_path()

# add modules to paths
main.add_module(eventinfosetter)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
# main.add_module(g4sim)

main.add_module(DisplayDataTest())

# default parameters
display = b2.register_module('Display')
display.param('showAllPrimaries', True)
main.add_module(display)

b2.process(main)
print(b2.statistics)
