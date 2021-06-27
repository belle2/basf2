#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact> SVD Software Group, svd-software@belle2.org </contact>
  <description>
    This module is used for the SVD validation.
    It gets information about ShaperDigits and RecoDigits, saving
    in a ttree in a ROOT file.
  </description>
</header>
"""

import basf2 as b2

# Some ROOT tools
import ROOT
from ROOT import Belle2
from ROOT import gROOT, addressof

# Define a ROOT struct to hold output data in the TTree
gROOT.ProcessLine('struct EventDataRecoDigit {\
    int sensor_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int sensor_type;\
    int strip_dir;\
    float recodigit_charge;\
    float recodigit_time;\
    float truehit_time;\
};')

from ROOT import EventDataRecoDigit  # noqa


class SVDValidationTTreeRecoDigit(b2.Module):
    '''class to create reco digitis ttree'''

    def __init__(self):
        """Initialize the module"""

        super(SVDValidationTTreeRecoDigit, self).__init__()
        #: output file
        self.file = ROOT.TFile('../SVDValidationTTreeRecoDigit.root', 'recreate')
        #: ttree
        self.tree = ROOT.TTree('tree', 'Event data of SVD validation events')
        #: instance of event data class
        self.data = EventDataRecoDigit()

        # Declare tree branches
        for key in EventDataRecoDigit.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, addressof(self.data, key), key + formstring)

    def event(self):
        """Take digits from SVDRecoDigits with a truehit and save needed information"""
        digits = Belle2.PyStoreArray('SVDRecoDigits')
        shaperDigits = Belle2.PyStoreArray('SVDShaperDigits')
        for digit in digits:
            # get the true hit from the related SVDShaperDigit
            digit_truehits = shaperDigits[digit.getArrayIndex()].getRelationsTo('SVDTrueHits')
            # We want only digits with exactly one associated TrueHit
            if len(digit_truehits) != 1:
                continue
            for truehit in digit_truehits:
                # Sensor identification
                sensorID = digit.getSensorID()
                self.data.sensor_id = int(sensorID)
                sensorNum = sensorID.getSensorNumber()
                self.data.sensor = sensorNum
                layerNum = sensorID.getLayerNumber()
                self.data.layer = layerNum
                if (layerNum == 3):
                    sensorType = 1  # Barrel
                else:
                    if (sensorNum == 1):
                        sensorType = 0
                    else:
                        sensorType = 1
                self.data.sensor_type = sensorType
                ladderNum = sensorID.getLadderNumber()
                self.data.ladder = ladderNum
                if digit.isUStrip():
                    self.data.strip_dir = 0
                else:
                    self.data.strip_dir = 1
                self.data.recodigit_charge = digit.getCharge()
                self.data.recodigit_time = digit.getTime()
                self.data.truehit_time = truehit.getGlobalTime()
                # Fill tree
                self.file.cd()
                self.tree.Fill()

    def terminate(self):
        """Close the output file. """
        self.file.cd()
        self.file.Write()
        self.file.Close()
