#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2
from ROOT import gROOT, AddressOf

# Define a ROOT struct to hold output data in the TTree.
gROOT.ProcessLine(
'struct EventDataSimHit {\
    unsigned long exp;\
    unsigned long run;\
    unsigned long evt;\
    int vxd_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int simhit_index;\
    int truehit_index;\
    int pixel_type;\
    int simhit_PDGcode;\
    float simhit_PosInX;\
    float simhit_PosInY;\
    float simhit_PosInZ;\
    float simhit_PosOutX;\
    float simhit_PosOutY;\
    float simhit_PosOutZ;\
    float simhit_Length;\
    float simhit_MomInX;\
    float simhit_MomInY;\
    float simhit_MomInZ;\
    float simhit_Theta;\
    float simhit_EnergyDep;\
    float simhit_GlobalTime;\
};')

from ROOT import EventDataSimHit


class PXDValidationTTreeSimHit(Module):

    """
    A simple module to check the simulation of PXDTrueHits with Geant4 steps.
    This module writes its output to a ROOT tree.
    Primary goal is supporting of validation plots
    """

    def __init__(self):
        """Initialize the module"""

        super(PXDValidationTTreeSimHit, self).__init__()
        # # Output ROOT file.
        self.file = ROOT.TFile('PXDValidationTTreeSimHitOutput.root',
            'recreate')
        # # TTree for output data
        self.tree = ROOT.TTree('tree', 'Event data of PXD simulation')
        # # Instance of EventData class
        self.data = EventDataSimHit()
        # Declare tree branches
        for key in EventDataSimHit.__dict__.keys():
            if not '__' in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key),
                                 key + formstring)

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """Find simhits with a truehit and save needed information."""

        # Start with truehits and use the relation to get the corresponding
        # simhits.
        pxd_truehits = Belle2.PyStoreArray('PXDTrueHits')
        for truehit in pxd_truehits:
            pxd_simhits = truehit.getRelationsTo('PXDSimHits')
            for simhit in pxd_simhits:
                # Now let's store some data
                # Event identification
                self.data.exp = \
                    Belle2.PyStoreObj('EventMetaData').obj().getExperiment()
                self.data.run = \
                    Belle2.PyStoreObj('EventMetaData').obj().getRun()
                self.data.evt = \
                    Belle2.PyStoreObj('EventMetaData').obj().getEvent()
                # Sesnor identification
                vxd_id = simhit.getSensorID()
                self.data.vxd_id = vxd_id.getID()
                self.data.layer = vxd_id.getLayerNumber()
                self.data.ladder = vxd_id.getLadderNumber()
                self.data.sensor = vxd_id.getSensorNumber()
                sensor_info = Belle2.VXD.GeoCache.get(vxd_id)
                VPitch = sensor_info.getVPitch(simhit.getPosIn().Y())
                self.data.pixel_type = (vxd_id.getLayerNumber() - 1) * 2
                if vxd_id.getLayerNumber() == 1:
                    if VPitch > 0.0058:
                        self.data.pixel_type += 1
                if vxd_id.getLayerNumber() == 2:
                    if VPitch > 0.0080:
                        self.data.pixel_type += 1
                # Hit identification
                self.data.simhit_index = simhit.getArrayIndex()
                self.data.truehit_index = truehit.getArrayIndex()
                # SimHit information
                self.data.simhit_PDGcode = simhit.getPDGcode()
                self.data.simhit_PosInX = simhit.getPosIn().X()
                self.data.simhit_PosInY = simhit.getPosIn().Y()
                self.data.simhit_PosInZ = simhit.getPosIn().Z()
                self.data.simhit_PosOutX = simhit.getPosOut().X()
                self.data.simhit_PosOutY = simhit.getPosOut().Y()
                self.data.simhit_PosOutZ = simhit.getPosOut().Z()
                self.data.simhit_Length = \
                    (simhit.getPosOut() - simhit.getPosIn()).Mag()
                self.data.simhit_MomInX = simhit.getMomIn().X()
                self.data.simhit_MomInY = simhit.getMomIn().Y()
                self.data.simhit_MomInZ = simhit.getMomIn().Z()
                self.data.simhit_Theta = simhit.getTheta()
                self.data.simhit_EnergyDep = simhit.getEnergyDep()
                self.data.simhit_GlobalTime = simhit.getGlobalTime()
                # Fill tree
                self.file.cd()
                self.tree.Fill()

    def terminate(self):
        """ Close the output file."""

        self.file.cd()
        self.file.Write()
        self.file.Close()
