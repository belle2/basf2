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
  <description>
    This module is used for the SVD validation.
    It gets information about RecoDigits, saving
    in a ttree in a ROOT file.
  </description>
  <noexecute>SVD validation helper class</noexecute>
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
        """Take digits from SVDRecoDigits with at least one truehit and save needed information"""
        digits = Belle2.PyStoreArray('SVDRecoDigits')
        shaperDigits = Belle2.PyStoreArray('SVDShaperDigits')
        for digit in digits:
            # get the true hit from the related SVDShaperDigit
            # it works because there is a 1-to-1 correspondence between
            # ShaperDigits and RecoDigits
            digit_truehits = shaperDigits[digit.getArrayIndex()].getRelationsTo('SVDTrueHits')

            if len(digit_truehits) == 0:
                continue

            # find the trueHit with highest energy deposit (the "best" match)
            energy = 0
            bestTrueHitIndex = 0

            for i, trueHit in enumerate(digit_truehits):
                if trueHit.getEnergyDep() > energy:
                    energy = trueHit.getEnergyDep()
                    bestTrueHitIndex = i
            bestTrueHit = digit_truehits[bestTrueHitIndex]

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
            self.data.truehit_time = bestTrueHit.getGlobalTime()
            # Fill tree
            self.file.cd()
            self.tree.Fill()

    def terminate(self):
        """Close the output file. """
        self.file.cd()
        self.file.Write()
        self.file.Close()
