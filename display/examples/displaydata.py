#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This example demonstrates showing user-defined things using DisplayData

import basf2 as b2

from ROOT import Belle2
from ROOT.Math import XYZVector
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
        displayData.obj().addLabel("Origin", XYZVector(0, 0, 0))

        # or simple points (grouped into sets)
        for i in range(10):
            displayData.obj().addPoint("set 1", XYZVector(15, 15, 10 * i))
        for i in range(10):
            displayData.obj().addPoint("set 2", XYZVector(10 * i, 0, 50))

        # you can add arrows
        pos = XYZVector(0, 0, 100)
        displayData.obj().addArrow("x", pos, pos + XYZVector(50, 0, 0))
        displayData.obj().addArrow("y", pos, pos + XYZVector(0, 50, 0))
        displayData.obj().addArrow("z", pos, pos + XYZVector(0, 0, 50))

        displayData.obj().addArrow("z=-2m", XYZVector(100, 100, -200), XYZVector(0, 0, -200), ROOT.kGray)

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
