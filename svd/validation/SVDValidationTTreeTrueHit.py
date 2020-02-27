#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact> SVD Software Group, svd-software@belle2.org </contact>
  <description>
    This module is used for the SVD validation.
    It gets information about truehits, saving in a ttree in a ROOT file.
  </description>
</header>
"""
import sys
import math

from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2  # make Belle2 namespace available
from ROOT import gROOT, AddressOf

# Define a ROOT struct to hold output data in the TTree
gROOT.ProcessLine('struct EventDataTrueHit {\
    int sensor_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int sensor_type;\
    float truehit_cluster;\
    };')

from ROOT import EventDataTrueHit


class SVDValidationTTreeTrueHit(Module):

    def __init__(self):
        """Initialize the module"""

        super(SVDValidationTTreeTrueHit, self).__init__()
        # Output ROOT file
        self.file = ROOT.TFile('../SVDValidationTTreeTrueHit.root', 'recreate')
        # TTrees for output data
        self.tree = ROOT.TTree('tree', 'Event data of SVD validation events')

        # Instance of the EventDataTrueHit class
        self.data = EventDataTrueHit()

        # Declare tree branches
        for key in EventDataTrueHit.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key), key + formstring)

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """Find truehit and save needed information"""

        # Start with truehits and use the relation to get the corresponding
        svd_truehits = Belle2.PyStoreArray('SVDTrueHits')
        print("\nlen(svd_truehits) = ", len(svd_truehits))
        for truehit in svd_truehits:
            self.data.truehit_cluster = -999
            clusters = truehit.getRelationsFrom('SVDClusters')
            print("len(clusters) = ", len(clusters))
            for cluster in clusters:
                # Sensor identification
                sensorID = cluster.getSensorID()
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
                #
                self.data.truehit_cluster = 1
                # Fill tree
                self.file.cd()
                self.tree.Fill()

    def terminate(self):
        """Close the output file. """
        self.file.cd()
        self.file.Write()
        self.file.Close()
