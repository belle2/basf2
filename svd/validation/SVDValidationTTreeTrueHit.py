#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
    <contact> SVD Software Group, svd-software@belle2.org </contact>
    <description>
    This module is used for the SVD validation.
    It gets information about truehits, saving in a ttree in a ROOT file.
    </description>
</header>
"""
import basf2 as b2

# Some ROOT tools
import ROOT
from ROOT import Belle2  # make Belle2 namespace available
from ROOT import gROOT, addressof

# Define a ROOT struct to hold output data in the TTree
gROOT.ProcessLine('struct EventDataTrueHit {\
    int sensor_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int sensor_type;\
    int strip_dir;\
    };')

from ROOT import EventDataTrueHit  # noqa


class SVDValidationTTreeTrueHit(b2.Module):
    '''class to create the true hit ttree'''

    def __init__(self):
        """Initialize the module"""

        super(SVDValidationTTreeTrueHit, self).__init__()
        #: output file
        self.file = ROOT.TFile('../SVDValidationTTreeTrueHit.root', 'recreate')
        #: output ttree
        self.tree = ROOT.TTree('tree', 'Event data of SVD validation events')
        #: instance of EventDataTrueHit class
        self.data = EventDataTrueHit()

        # Declare tree branches
        for key in EventDataTrueHit.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, addressof(self.data, key), key + formstring)

    def event(self):
        """ Start with truehits and use the relation to get the corresponding clusters """
        svdtruehits = Belle2.PyStoreArray('SVDTrueHits')
        for truehit in svdtruehits:
            clusters = truehit.getRelationsFrom('SVDClusters')
            if len(clusters) == 0:
                # Sensor identification
                sensorID = truehit.getSensorID()
                self.data.sensor_id = int(sensorID)
                sensorNum = sensorID.getSensorNumber()
                self.data.sensor = sensorNum
                layerNum = sensorID.getLayerNumber()
                self.data.layer = layerNum
                if (layerNum == 3):
                    sensorType = 1
                else:
                    if (sensorNum == 1):
                        sensorType = 0
                    else:
                        sensorType = 1
                self.data.sensor_type = sensorType
                ladderNum = sensorID.getLadderNumber()
                self.data.ladder = ladderNum
                self.data.strip_dir = -1
                # Fill tree
                self.file.cd()
                self.tree.Fill()
            else:
                for cluster in clusters:
                    # Sensor identification
                    sensorID = truehit.getSensorID()
                    self.data.sensor_id = int(sensorID)
                    sensorNum = sensorID.getSensorNumber()
                    self.data.sensor = sensorNum
                    layerNum = sensorID.getLayerNumber()
                    self.data.layer = layerNum
                    if (layerNum == 3):
                        sensorType = 1
                    else:
                        if (sensorNum == 1):
                            sensorType = 0
                        else:
                            sensorType = 1
                    self.data.sensor_type = sensorType
                    ladderNum = sensorID.getLadderNumber()
                    self.data.ladder = ladderNum
                    if cluster.isUCluster():
                        self.data.strip_dir = 0
                    else:
                        self.data.strip_dir = 1
                    # Fill tree
                    self.file.cd()
                    self.tree.Fill()

    def terminate(self):
        """Close the output file. """
        self.file.cd()
        self.file.Write()
        self.file.Close()
