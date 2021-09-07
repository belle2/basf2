#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Simple script to create some particles in a list with a python module
"""

# Go away doxygen, no one likes you
# @cond

import basf2
from ROOT import Belle2
from ROOT.Math import PxPyPzEVector


class MyParticleCreator(basf2.Module):
    def initialize(self):
        self.listB0 = Belle2.ParticleListHelper("B0:all")
        self.listPi0 = Belle2.ParticleListHelper("pi0:all")
        print(self.listB0.isSelfConjugated(), self.listPi0.isSelfConjugated())

    def event(self):
        self.listB0.create()
        self.listPi0.create()
        for i in range(5):
            self.listB0.addParticle(PxPyPzEVector(i, 0, 0, i), i % 2 == 0)
            self.listPi0.addParticle(PxPyPzEVector(i, 0, 0, i), i % 2 == 0)


basf2.set_random_seed("something important")
path = basf2.create_path()
path.add_module("EventInfoSetter", evtNumList=[2])
path.add_module(MyParticleCreator())
path.add_module("ParticlePrinter", listName="B0:all")
path.add_module("ParticlePrinter", listName="pi0:all")
basf2.process(path)

# @endcond
