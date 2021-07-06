#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Creates two histograms and adds them to the 'Histogram' tab in the display
# Doubleclicking them will show them in the current canvas (added on
# the right side of the screen) and keep them updated each event.

import basf2 as b2

from ROOT import Belle2
from ROOT import TH1F
from ROOT import TH3F


class GenerateHist(b2.Module):
    """Generate some histograms to pass to the display."""

    #: histogram for SimHit energy deposition
    edep_hist = TH1F("CDC_edep", "Energy deposition from CDCSimHits", 100, 0.0, 10e-6)
    #: histogram for SimHit positions
    pos_hist = TH3F("CDC_pos", "Positions of CDCSimHits",
                    100, -150.0, 150.0,
                    100, -150.0, 150.0,
                    100, -150.0, 150.0)

    def initialize(self):
        """reimplementation of Module::initialize()."""

        Belle2.PyStoreObj("DisplayData").registerInDataStore()

    def event(self):
        """reimplementation of Module::event()."""

        displayData = Belle2.PyStoreObj("DisplayData")
        displayData.create()

        cdcsimhits = Belle2.PyStoreArray("CDCSimHits")
        for hit in cdcsimhits:
            self.edep_hist.Fill(hit.getEnergyDep())
            p = hit.getPosWire()
            self.pos_hist.Fill(p.X(), p.Y(), p.Z())

        displayData.obj().addHistogram("CDC_edep", self.edep_hist)
        displayData.obj().addHistogram("CDC_pos", self.pos_hist)


# register necessary modules
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [500])


# create geometry
gearbox = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')
geometry.param('components', ['CDC', 'MagneticField'])

particlegun = b2.register_module('ParticleGun')

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
main.add_module(g4sim)

main.add_module(GenerateHist())

# default parameters
display = b2.register_module('Display')
main.add_module(display)

b2.process(main)
print(b2.statistics)
