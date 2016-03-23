#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
# Demonstrates using PyStoreArray and PyStoreObj to
# create data store contents from Python modules.
######################################################

import os
import math
import random

import basf2
import ROOT
from ROOT import Belle2

import simulation


class SillyGeneratorModule(basf2.Module):
    """Small module to demonstrate how the registration of StoreArrays works from Python"""

    def initialize(self):
        """ Register a StoreArray on the DataStore"""
        mcParticles = Belle2.PyStoreArray(Belle2.MCParticle.Class())
        mcParticles.registerInDataStore()

    def event(self):
        """ Access and fill the registered StoreArray """
        # Alternative access by name
        mcParticles = Belle2.PyStoreArray("MCParticles")
        mcParticle = mcParticles.appendNew()

        # Fill one test muon for demonstration
        vertex = ROOT.TVector3(0, 0, 0)

        # Sample a random momentum vector of
        # gaus distributed length and uniform distibuted  direction
        phi = ROOT.gRandom.Uniform(0.0, 2.0 * math.pi)
        costheta = ROOT.gRandom.Uniform(-1.0, 1.0)
        theta = math.acos(costheta)
        r = max(0, ROOT.gRandom.Gaus(1, 0.2))
        momentum = ROOT.TVector3(0, 0, 0)
        momentum.SetMagThetaPhi(1, theta, phi)

        mcParticle.setPDG(13)
        mcParticle.setMassFromPDG()
        mcParticle.addStatus(Belle2.MCParticle.c_PrimaryParticle)
        mcParticle.addStatus(Belle2.MCParticle.c_StableInGenerator)
        mcParticle.setProductionVertex(vertex)
        mcParticle.setMomentum(momentum)
        m = mcParticle.getMass()
        mcParticle.setEnergy(math.sqrt(momentum * momentum + m * m))
        mcParticle.setDecayTime(float("inf"))


class ParticleStatisticsModule(basf2.Module):
    """Collect statistics on particles"""

    def initialize(self):
        """init"""
        #: save statistics in here
        self.tfile = ROOT.TFile("ParticleStatistics.root", "recreate")

        ntuple = Belle2.PyStoreObj(Belle2.RootMergeable("TNtuple").Class(), "ParticleMomenta", Belle2.DataStore.c_Persistent)
        ntuple.registerInDataStore()
        print("IsValid", ntuple.isValid())
        ntuple.create()
        print("IsValid", ntuple.isValid())
        ntuple.obj().assign(ROOT.TNtuple("Particles",  # ROOT name
                                         "Momentum compontents of the particles",  # ROOT title
                                         "px:py:pz"  # Var list
                                         ))

        hist = Belle2.PyStoreObj(Belle2.RootMergeable("TH1D").Class(), "AbsMomentum", Belle2.DataStore.c_Persistent)
        hist.registerInDataStore()
        print("IsValid", hist.isValid())
        hist.create()
        print("IsValid", hist.isValid())
        hist.obj().assign(ROOT.TH1D("AbsMomentum",  # ROOT name
                                    "Absolute momentum of particles",  # ROOT title
                                    20,  # n bins
                                    0,  # upper bound
                                    2  # lower bound
                                    ))

        print("IsValid", ntuple.isValid())
        print("IsValid", hist.isValid())

    def event(self):
        """actually collect info"""
        ntuple = Belle2.PyStoreObj("ParticleMomenta", Belle2.DataStore.c_Persistent)
        hist = Belle2.PyStoreObj("AbsMomentum", Belle2.DataStore.c_Persistent)

        print("IsValid", ntuple.isValid())
        print("IsValid", hist.isValid())

        mcParticles = Belle2.PyStoreArray(Belle2.MCParticle.Class())

        for mcParticle in mcParticles:
            momentum = mcParticle.getMomentum()
            ntuple.obj().get().Fill(momentum.X(), momentum.Y(), momentum.Z())
            hist.obj().get().Fill(momentum.Mag())

    def terminate(self):
        """terminate"""
        ntuple = Belle2.PyStoreObj("ParticleMomenta", Belle2.DataStore.c_Persistent)
        ntuple.obj().write(self.tfile)
        hist = Belle2.PyStoreObj("AbsMomentum", Belle2.DataStore.c_Persistent)
        hist.write(self.tfile)
        self.tfile.Close()


# ******** Register modules  **********
eventInfoSetterModule = basf2.register_module('EventInfoSetter')
eventInfoSetterModule.param(dict(
    evtNumList=[1000],
    runList=[1],
    expList=[1]
))

printCollectionsModule = basf2.register_module('PrintCollections')
sillyGeneratorModule = SillyGeneratorModule()
particleStatisticsModule = ParticleStatisticsModule()

# ******** Create paths and add modules  **********
main = basf2.create_path()
main.add_module(eventInfoSetterModule)
main.add_module(sillyGeneratorModule)
main.add_module(particleStatisticsModule)
simulation.add_simulation(main)

main.add_module(printCollectionsModule)
basf2.process(main)

# Print call statistics
print(basf2.statistics)
