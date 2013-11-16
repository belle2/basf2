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
'struct EventDataDigit {\
    unsigned long exp;\
    unsigned long run;\
    unsigned long evt;\
    int vxd_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int cluster_index;\
    int pixel_type;\
    float digit_u;\
    float digit_v;\
    int digit_uID;\
    int digit_vID;\
    float digit_charge;\
};')

from ROOT import EventDataDigit


class PXDValidationTTreeDigit(Module):

    """
    A simple module to check the simulation of PXDTrueHits with Geant4 steps.
    This module writes its output to a ROOT tree.
    Primary goal is supporting of validation plots
    """

    def __init__(self):
        """Initialize the module"""

        super(PXDValidationTTreeDigit, self).__init__()
        # # Output ROOT file.
        self.file = \
            ROOT.TFile('PXDValidationTTreeDigitOutput.root', 'recreate')
        # # TTree for output data
        self.tree = ROOT.TTree('tree', 'Event data of PXD simulation')
        # # Instance of EventData class
        self.data = EventDataDigit()
        # Declare tree branches
        for key in EventDataDigit.__dict__.keys():
            if not '__' in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key),
                                 key + formstring)

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """Find digits with a clusters and save needed information."""

        # Start with clusters and use the relation to get the corresponding
        # digits and truehits.
        pxd_clusters = Belle2.PyStoreArray('PXDClusters')
        for cluster in pxd_clusters:
            cluster_truehits = cluster.getRelationsTo("PXDTrueHits")

            # Here we ask only for clusters with exactly one TrueHit.
            if len(cluster_truehits) != 1:
                continue

            cluster_digits = cluster.getRelationsTo("PXDDigits")
            for digit in cluster_digits:
                # Now let's store some data
                # Event identification
                self.data.exp = Belle2.PyStoreObj('EventMetaData').\
                    obj().getExperiment()
                self.data.run = Belle2.PyStoreObj('EventMetaData').\
                    obj().getRun()
                self.data.evt = Belle2.PyStoreObj('EventMetaData').\
                    obj().getEvent()
                # Sesnor identification
                vxd_id = digit.getSensorID()
                self.data.vxd_id = vxd_id.getID()
                self.data.layer = vxd_id.getLayerNumber()
                self.data.ladder = vxd_id.getLadderNumber()
                self.data.sensor = vxd_id.getSensorNumber()
#                if vxd_id.getLayerNumber() == 1:
#                    continue
#                if vxd_id.getSensorNumber() == 2:
#                    continue
                #self.data.cluster_index = digit.getArrayIndex()
                self.data.cluster_index = cluster.getArrayIndex()
                #self.data.cluster_index = truehit.getArrayIndex()
                # Get sensor geometry information
                sensor_info = Belle2.VXD.GeoCache.get(vxd_id)
                thickness = sensor_info.getThickness()
                UPitch = sensor_info.getUPitch()
                VPitch = sensor_info.getVPitch(digit.getVCellPosition())
                self.data.pixel_type = (vxd_id.getLayerNumber() - 1) * 2
                if vxd_id.getLayerNumber() == 1:
                    if VPitch > 0.0058:
                        self.data.pixel_type += 1
                if vxd_id.getLayerNumber() == 2:
                    if VPitch > 0.0080:
                        self.data.pixel_type += 1
                # Digit information
                self.data.digit_u = digit.getUCellPosition()
                self.data.digit_v = digit.getVCellPosition()
                self.data.digit_uID = digit.getUCellID()
                self.data.digit_vID = digit.getVCellID()
                self.data.digit_charge = digit.getCharge()
                # Fill tree
                self.file.cd()
                self.tree.Fill()

    def terminate(self):
        """ Close the output file."""

        self.file.cd()
        self.file.Write()
        self.file.Close()
