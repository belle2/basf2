#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ******** Imports ********
import os
import math

import basf2
import ROOT
from ROOT import Belle2

import simulation


class SillyGeneratorModule(basf2.Module):
    """Small module to demonstrate how the registration of StoreArrays works from Python"""

    def initialize(self):
        # Register a StoreArray on the DataStore
        mcParticles = Belle2.PyStoreArray(Belle2.MCParticle.Class())
        mcParticles.registerInDataStore()

    def event(self):
        # Access and fill the registered StoreArray
        # Alternative access by name
        mcParticles = Belle2.PyStoreArray("MCParticles")
        mcParticle = mcParticles.appendNew()

        # Fill one test muon for demonstration
        vertex = ROOT.TVector3(0, 0, 0)
        momentum = ROOT.TVector3(1, 0, 0)
        mcParticle.setPDG(13)
        mcParticle.setMassFromPDG()
        mcParticle.addStatus(Belle2.MCParticle.c_PrimaryParticle)
        mcParticle.addStatus(Belle2.MCParticle.c_StableInGenerator)
        mcParticle.setProductionVertex(vertex)
        mcParticle.setMomentum(momentum)
        m = mcParticle.getMass()
        mcParticle.setEnergy(math.sqrt(momentum * momentum + m * m))
        mcParticle.setDecayTime(float("inf"))

# ******** Register modules  **********
eventInfoSetterModule = basf2.register_module('EventInfoSetter')
eventInfoSetterModule.param(dict(
    evtNumList=[1],
    runList=[1],
    expList=[1]
))

printCollectionsModule = basf2.register_module('PrintCollections')
sillyGeneratorModule = SillyGeneratorModule()
displayModule = basf2.register_module('Display')

# ******** Create paths and add modules  **********
main = basf2.create_path()
main.add_module(eventInfoSetterModule)
main.add_module(sillyGeneratorModule)

simulation.add_simulation(main)

main.add_module(printCollectionsModule)
main.add_module(displayModule)

basf2.process(main)

# Print call statistics
print(basf2.statistics)
