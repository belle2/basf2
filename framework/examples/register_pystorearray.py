#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

######################################################
# Demonstrates using PyStoreArray and PyStoreObj to
# create data store contents from Python modules.
######################################################

import math

import basf2
import ROOT
from ROOT import Belle2


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
    """Collect statistics on particles - also parallel processable"""

    def __init__(self):
        """set module flags"""
        super().__init__()
        self.set_property_flags(
            basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED |
            basf2.ModulePropFlags.TERMINATEINALLPROCESSES)

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

        mcParticles = Belle2.PyStoreArray(Belle2.MCParticle.Class())

        for mcParticle in mcParticles:
            momentum = mcParticle.getMomentum()
            ntuple.get().Fill(momentum.X(),
                              momentum.Y(),
                              momentum.Z())

            hist.get().Fill(momentum.Mag())

    def terminate(self):
        """terminate"""
        if (not Belle2.ProcHandler.parallelProcessingUsed() or
                Belle2.ProcHandler.isOutputProcess()):
            print("Writting objects")
            ntuple = Belle2.PyStoreObj("ParticleMomenta", Belle2.DataStore.c_Persistent)
            ntuple.write(self.tfile)

            hist = Belle2.PyStoreObj("AbsMomentum", Belle2.DataStore.c_Persistent)
            hist.write(self.tfile)

            self.tfile.Close()


def main():
    path = basf2.create_path()
    path.add_module('EventInfoSetter',
                    evtNumList=[1000],
                    runList=[1],
                    expList=[1]
                    )

    path.add_module(SillyGeneratorModule())
    path.add_module(ParticleStatisticsModule())

    basf2.process(path)

    # Print call statistics
    print(basf2.statistics)


if __name__ == "__main__":
    main()
