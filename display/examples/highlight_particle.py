#!/usr/bin/env python
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

        Belle2.PyStoreObj("DisplayData").registerAsPersistent()

    def event(self):
        """reimplementation of Module::event()."""

        displayData = Belle2.PyStoreObj("DisplayData")
        displayData.create()

        particles = Belle2.PyStoreArray('Particles')
        for p in particles:
            if abs(p.getPDGCode()) == 413:
                B2WARNING("highlighting D*+ candidate")
                #displayData.obj().select(p)
                daughters = p.getFinalStateDaughters()
                for d in daughters:
                    # selecting the MCParticle also gets the tracks
                    mcp = d.getRelated('MCParticles')
                    displayData.obj().select(mcp)
                break  # only one


main = create_path()

main.add_module(register_module('RootInput'))
main.add_module(register_module('ParticleLoader'))
main.add_module(register_module('Gearbox'))
main.add_module(register_module('Geometry'))


selectParticle('K-', -321, [], path=main)
selectParticle('pi+', 211, [], path=main)
selectParticle('g', 22, [], path=main)

makeParticle('pi0', 111, ['g', 'g'], 0.110, 0.150, path=main)

applyCuts('pi0', ['p.1 0.1:', 'p.2 0.1:'], path=main)

makeParticle('D0', 421, ['K-', 'pi+', 'pi0'], 1.700, 2.000, path=main)
makeParticle('D*+', 413, ['D0', 'pi+'], 1.900, 2.100, path=main)

applyCuts('D*+', ['M.1 1.81:1.91', 'Q :0.02', 'p_CMS 2.0:'], path=main)

main.add_module(HighlighterModule())

display = register_module('Display')
main.add_module(display)

process(main)
