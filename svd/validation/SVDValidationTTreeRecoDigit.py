#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact> K. Adamczyk, adamczykarol@gmail.com </contact>
  <description>
    This module is used for the SVD validation.
    It gets information about ShaperDigits and RecoDigits, saving
    in a ttree in a ROOT file.
  </description>
</header>
"""
import sys
import math

from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2
from ROOT import gROOT, AddressOf
from ROOT import PyConfig
from ROOT import TVector3

# Define a ROOT struct to hold output data in the TTree
gROOT.ProcessLine('struct EventDataRecoDigit {\
    int sensor_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int sensor_type;\
    int strip_dir;\
    float recodigit_time;\
    float truehit_time;\
};')

from ROOT import EventDataRecoDigit


class SVDValidationTTreeRecoDigit(Module):

    def __init__(self):
        """Initialize the module"""

        super(SVDValidationTTreeRecoDigit, self).__init__()
        # Output ROOT file
        self.file = ROOT.TFile('../SVDValidationTTreeRecoDigit.root', 'recreate')
        # TTrees for output data
        self.tree = ROOT.TTree('tree', 'Event data of SVD validation events')

        # Instance of the EventData class
        self.data = EventDataRecoDigit()

        # Declare tree branches
        for key in EventDataRecoDigit.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key), key + formstring)

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """Take digits from SVDRecoDigits with a truehit and save needed information"""
        digits = Belle2.PyStoreArray('SVDRecoDigits')
        for digit in digits:
            digit_truehits = digit.getRelationsTo('SVDTrueHits')
            # We want only digits with exactly one associated TrueHit
            if len(digit_truehits) != 1:
                # print("len(digit_truehits) != 1")
                continue
            for truehit in digit_truehits:
                sensorInfo = Belle2.VXD.GeoCache.get(digit.getSensorID())
                # Let's store some data
                # Sensor identification
                sensorID = digit.getSensorID()
                self.data.sensor_id = int(sensorID)
                sensorNum = sensorID.getSensorNumber()
                self.data.sensor = sensorNum
                layerNum = sensorID.getLayerNumber()
                # print("layerNum = ", layerNum)
                self.data.layer = layerNum
                if (layerNum == 3):
                    sensorType = 1  # Barrel
                else:
                    if (sensorNum == 1):
                        sensorType = 0
                    else:
                        sensorType = 1
                self.data.sensor_type = sensorType
                # print("sensorType = ", sensorType)
                ladderNum = sensorID.getLadderNumber()
                self.data.ladder = ladderNum
                # Digit information
                self.data.recodigit_time = digit.getTime()
                # TrueHit information
                self.data.truehit_time = truehit.getGlobalTime()
                # Fill tree
                self.file.cd()
                self.tree.Fill()

    def terminate(self):
        """Close the output file. """
        self.file.cd()
        self.file.Write()
        self.file.Close()
