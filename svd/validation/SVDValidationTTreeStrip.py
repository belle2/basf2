#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>G. Caria, gcaria@student.unimelb.edu.au</contact>
  <description>
    This module is used for the SVD validation.
    It gets information about digits, saving
    in a ttree in a ROOT file.
  </description>
</header>
"""
import sys
import math
import xml.etree.ElementTree as ET

from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2  # make Belle2 namespace available
from ROOT import gROOT, AddressOf
from ROOT import PyConfig
from ROOT import TVector3

# Define a ROOT struct to hold output data in the TTree
gROOT.ProcessLine('struct EventDataStrip {\
    int sensor_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int sensor_type;\
    int strip_dir;\
    float strip_charge;\
    int strip_noise;\
    };')

from ROOT import EventDataStrip


class SVDValidationTTreeStrip(Module):

    def __init__(self):
        """Initialize the module"""

        super(SVDValidationTTreeStrip, self).__init__()
        # Output ROOT file
        self.file = ROOT.TFile('../SVDValidationTTreeStrip.root', 'recreate')
        # TTrees for output data
        self.tree = ROOT.TTree('tree', 'Event data of SVD validation events')

        # Instance of the EventDataStrip class
        self.data = EventDataStrip()

        # Declare tree branches
        for key in EventDataStrip.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key), key + formstring)

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """Find digit with a cluster and save needed information"""
        # Before starting iterating on clusters gets noise value from xml file
        # so that it doesn't have to be repeated for every digit
        tree = ET.parse(Belle2.FileSystem.findFile('svd/data/SVD-Components.xml'))
        root = tree.getroot()
        for sensor in root.findall('Sensor'):
            if sensor.get('type') == 'Layer3':
                if len(sensor.findall('Active')) == 1:
                    active = sensor.find('Active')
                    NoiseULayer3 = int(active.find('ElectronicNoiseU').text)
                    NoiseVLayer3 = int(active.find('ElectronicNoiseV').text)
        for sensorbase in root.iter('SensorBase'):
            if sensorbase.get('type') == 'Barrel':
                active = sensorbase.find('Active')
                NoiseUBarrel = int(active.find('ElectronicNoiseU').text)
                NoiseVBarrel = int(active.find('ElectronicNoiseV').text)
            elif sensorbase.get('type') == 'Slanted':
                active = sensorbase.find('Active')
                NoiseUSlanted = int(active.find('ElectronicNoiseU').text)
                NoiseVSlanted = int(active.find('ElectronicNoiseV').text)

        # Start with clusters and use the relation to get the corresponding
        # digits
        clusters = Belle2.PyStoreArray('SVDClusters')
        for cluster in clusters:

            cls_strip_ids = []

            cluster_truehits = cluster.getRelationsTo('SVDTrueHits')
            # We want only clusters with exactly one associated TrueHit
            if len(cluster_truehits) != 1:
                continue
            digits = cluster.getRelationsTo('SVDRecoDigits')

            # get all the strip IDs of this cluster
            for digit in digits:
                if digit.getCellID() not in cls_strip_ids:
                    cls_strip_ids.append(digit.getCellID())

            # get the strip charge as the highest charge among its digits
            for strip_id in cls_strip_ids:
                # print("strip_id is : " + str(strip_id))
                strip_charge = 0
                for digit in digits:
                    if strip_id != digit.getCellID():
                        continue

                    # print("this digit's charge: " + str(digit.getCharge()))

                    if(digit.getCharge() > strip_charge):
                        strip_charge = digit.getCharge()

                # Sensor identification
                sensorID = cluster.getSensorID()
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
                self.data.strip_charge = strip_charge
                # Find what noise value should be used
                # depending on layer and sensor number
                if digit.isUStrip():
                    strip_dir = 0
                    if (layerNum == 3):
                        noise = NoiseULayer3
                    else:
                        if (sensorNum == 1):
                            noise = NoiseUSlanted
                        else:
                            noise = NoiseUBarrel
                else:
                    strip_dir = 1
                    if (layerNum == 3):
                        noise = NoiseVLayer3
                    else:
                        if (sensorNum == 1):
                            noise = NoiseVSlanted
                        else:
                            noise = NoiseVBarrel
                self.data.strip_dir = strip_dir
                self.data.strip_noise = noise
                # Fill tree
                self.file.cd()
                self.tree.Fill()

    def terminate(self):
        """Close the output file. """
        self.file.cd()
        self.file.Write()
        self.file.Close()
