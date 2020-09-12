#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>G. Casarosa giulia.casarosa@desy.de</contact>
  <description>
    This module is used for the SVD validation.
    It gets information about SpacePoints, saving
    in a ttree in a ROOT file.
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
from ROOT import PyConfig
from ROOT import TVector3

# Define a ROOT struct to hold output data in the TTree
gROOT.ProcessLine('struct EventDataSpacePoint {\
    int sensor_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int sensor_type;\
    float time_u;\
    float time_v;\
    };')

from ROOT import EventDataSpacePoint  # noqa


class SVDValidationTTreeSpacePoint(Module):
    '''class to create spacepoint ttree'''

    def __init__(self):
        """Initialize the module"""

        super(SVDValidationTTreeSpacePoint, self).__init__()
        self.file = ROOT.TFile('../SVDValidationTTreeSpacePoint.root', 'recreate')
        '''Output ROOT file'''
        self.tree = ROOT.TTree('tree', 'Event data of SVD validation events')
        '''TTrees for output data'''
        self.data = EventDataSpacePoint()
        '''Instance of the EventDataSpacePoint class'''

        # Declare tree branches
        for key in EventDataSpacePoint.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key), key + formstring)

    def event(self):
        """Find digit with a cluster and save needed information"""

        # Start with SpacePoints and use the relation to get the corresponding
        # digits
        spacepoints = Belle2.PyStoreArray('SVDSpacePoints')

        for sp in spacepoints:

            # Sensor identification
            sensorID = sp.getVxdID()
            self.data.sensor_id = int(sensorID)
            sensorNum = sensorID.getSensorNumber()
            self.data.sensor = sensorNum
            layerNum = sensorID.getLayerNumber()

            # look at SP on SVD only
            sp_type = sp.getType()
            if sp_type != 1:
                continue

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

            # space point information
            timeU = sp.TimeU()
            timeV = sp.TimeV()
            self.data.time_u = timeU
            self.data.time_v = timeV

            # Fill tree
            self.file.cd()
            self.tree.Fill()

    def terminate(self):
        """Close the output file. """
        self.file.cd()
        self.file.Write()
        self.file.Close()
