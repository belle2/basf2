#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <contact>D. Cervenkov, cervenkov@ipnp.troja.mff.cuni.cz</contact>
  <description>
    This module is part of the SVD validation suite. It creates
    an energy loss distribution and saves it to a ROOT file.
  </description>
</header>
"""

from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2
from ROOT import gROOT, AddressOf

# Define a ROOT struct to hold output data in the TTree.
gROOT.ProcessLine('struct EnergyLossData {\
    float simhit_length;\
    float simhit_energy;\
    float simhit_dEdx;\
};'
                  )

from ROOT import EnergyLossData


class SVDEnergyLoss(Module):

    def __init__(self):
        super(SVDEnergyLoss, self).__init__()
        ## Output ROOT file.
        self.file = ROOT.TFile('SVDEnergyLossData.root', 'recreate')
        ## TTree for output data
        self.tree = ROOT.TTree('tree', 'Event data of SVD simulation')
        ## Instance of EnergyLossData class
        self.data = EnergyLossData()
        # Declare tree branches
        for key in EnergyLossData.__dict__.keys():
            if not '__' in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key), key
                                 + formstring)

    def beginRun(self):
        """ Dummy module """

    def event(self):
        # Start with truehits and use the relation to get the corresponding
        # simhits.
        truehits = Belle2.PyStoreArray('SVDTrueHits')
        for truehit in truehits:
            simhits = truehit.getRelationsTo('SVDSimHits')
            for simhit in simhits:
                # We are interested only in the primary particles, not
                # delta electrons, etc.
                particle = simhit.getRelatedFrom('MCParticles')
                if not particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle):
                    continue

                length = (simhit.getPosOut() - simhit.getPosIn()).Mag()
                # The deposited energy is the number of electrons multiplied
                # by the energy required to create an electron-hole pair
                energy = simhit.getElectrons() * Belle2.Const.ehEnergy

                self.data.simhit_length = length
                self.data.simhit_energy = energy
                self.data.simhit_dEdx = energy / length

                # A reasonable cut to see a nice Landau distribution
                if self.data.simhit_dEdx > 0.015:
                    continue

                # Fill tree
                self.file.cd()
                self.tree.Fill()

    def terminate(self):
        self.file.cd()
        self.file.Write()
        self.file.Close()


