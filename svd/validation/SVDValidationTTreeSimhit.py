#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <description>
    This module is used for the SVD validation.
    It gets information about truehits and  clusters, saving
    in a ttree in a ROOT file.
  </description>
  <noexecute>SVD validation helper class</noexecute>
</header>
"""

import basf2 as b2

# Some ROOT tools
import ROOT
from ROOT import Belle2  # make Belle2 namespace available
from ROOT import gROOT, addressof

# Define a ROOT struct to hold output data in the TTree
gROOT.ProcessLine('struct EventDataSimhit {\
    int sensor_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int sensor_type;\
    float simhit_length;\
    float simhit_energy;\
    float simhit_dEdx;\
    };')

from ROOT import EventDataSimhit  # noqa


class SVDValidationTTreeSimhit(b2.Module):
    '''class to create sim hit ttree'''

    def __init__(self):
        """Initialize the module"""

        super().__init__()
        #: output file
        self.file = ROOT.TFile('../SVDValidationTTreeSimhit.root', 'recreate')
        #: output ttree
        self.tree = ROOT.TTree('tree', 'Event data of SVD validation events')
        #: instance of eventDataSimhit
        self.data = EventDataSimhit()

        # Declare tree branches
        for key in EventDataSimhit.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, addressof(self.data, key), key + formstring)

    def event(self):
        """Find simhits with a truehit and save needed information"""

        # Start with truehits and use the relation to get the corresponding
        # simhits
        svd_truehits = Belle2.PyStoreArray('SVDTrueHits')
        for truehit in svd_truehits:
            simhits = truehit.getRelationsTo('SVDSimHits')
            for simhit in simhits:
                # Sensor identification
                sensorID = simhit.getSensorID()
                self.data.sensor_id = int(sensorID)
                sensorNum = sensorID.getSensorNumber()
                self.data.sensor = sensorNum
                layerNum = sensorID.getLayerNumber()
                self.data.layer = layerNum
                ladderNum = sensorID.getLadderNumber()
                self.data.ladder = ladderNum
                if (layerNum == 3):
                    sensorType = 1
                else:
                    if (sensorNum == 1):
                        sensorType = 0
                    else:
                        sensorType = 1
                self.data.sensor_type = sensorType
                # We are interested only in the primary particles, not
                # delta electrons, etc.
                particle = simhit.getRelatedFrom('MCParticles')
                if not particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle):
                    continue

                length = (simhit.getPosOut() - simhit.getPosIn()).R()
                # The deposited energy is the number of electrons multiplied
                # by the energy required to create an electron-hole pair
                energy = simhit.getElectrons() * Belle2.Const.ehEnergy
                self.data.simhit_length = length
                self.data.simhit_energy = energy
                if (length == 0):
                    continue
                self.data.simhit_dEdx = energy / length
                # A reasonable cut to see a nice Landau distribution
                # remove this cut to be sensitive to delta rays
                # if self.data.simhit_dEdx > 0.015:
                #    continue
                # Fill tree
                self.file.cd()
                self.tree.Fill()

    def terminate(self):
        """Close the output file. """
        self.file.cd()
        self.file.Write()
        self.file.Close()
