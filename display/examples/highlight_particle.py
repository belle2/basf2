#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# given a DST file as input, this example reconstructs D*+ candidates
# and highlights all final state particles assigned to the candidate
# in the display.

import os
import random
from basf2 import *

from ROOT import Belle2
from modularAnalysis import *


class HighlighterModule(Module):
    """Select all things related to a Particle"""

    def initialize(self):
        """reimplementation of Module::initialize()."""

        Belle2.PyStoreObj("DisplayData").registerInDataStore()

    def event(self):
        """reimplementation of Module::event()."""

        displayData = Belle2.PyStoreObj("DisplayData")
        displayData.create()

        particles = Belle2.PyStoreArray('Particles')
        for p in particles:
            if abs(p.getPDGCode()) == 413:
                B2WARNING("highlighting D*+ candidate")
                # displayData.obj().select(p)
                daughters = p.getFinalStateDaughters()
                for d in daughters:
                    # selecting the MCParticle also gets the tracks
                    mcp = d.getRelated('MCParticles')
                    displayData.obj().select(mcp)
                break  # only one


main = create_path()

main.add_module('RootInput')
main.add_module('Gearbox')
geometry = main.add_module('Geometry')
geometry.param('excludedComponents', ['ECL'])


fillParticleList('K-', 'kaonID > 0.1', path=main)
fillParticleList('pi+', 'pionID > 0.1', path=main)
fillParticleList('gamma', '', path=main)

reconstructDecay('pi0 -> gamma gamma', '0.110 < M < 0.150', path=main)

reconstructDecay('D0 -> K- pi+', '1.7 < M < 2.0', path=main)
reconstructDecay('D*+ -> D0 pi+', '1.9 < M < 2.1', path=main)


main.add_module(HighlighterModule())

main.add_module('Display')

process(main)
