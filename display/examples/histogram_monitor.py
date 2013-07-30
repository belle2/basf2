#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Creates two histograms and adds them to the 'Histogram' tab in the display
# Doubleclicking them will show them in the current canvas (added on
# the right side of the screen) and keep them updated each event.

import os
import random
from basf2 import *

# avoid race conditions beetween pyroot and GUI thread
from ROOT import PyConfig
PyConfig.StartGuiThread = False

from ROOT import Belle2
from ROOT import TH1F
from ROOT import TH3F
from ROOT import TVector3


class GenerateHist(Module):
    """Generate some histograms to pass to the display."""

    ## histogram for SimHit energy deposition
    edep_hist = TH1F("CDC_edep", "Energy deposition from CDCSimHits",
        100, 0.0, 10e-6)
    ## histogram for SimHit positions
    pos_hist = TH3F("CDC_pos", "Positions of CDCSimHits",
        100, -150.0, 150.0,
        100, -150.0, 150.0,
        100, -150.0, 150.0)

    def __init__(self):
        """constructor."""

        super(GenerateHist, self).__init__()

    def initialize(self):
        """reimplementation of Module::initialize()."""

        Belle2.PyStoreObj("DisplayData").registerAsPersistent()

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
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('evtNumList', [500])


# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('Components', ['CDC', 'MagneticField'])

particlegun = register_module('ParticleGun')

# simulation
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# create paths
main = create_path()

# add modules to paths
main.add_module(evtmetagen)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(g4sim)

main.add_module(GenerateHist())

# default parameters
display = register_module('Display')
main.add_module(display)

process(main)
print statistics
