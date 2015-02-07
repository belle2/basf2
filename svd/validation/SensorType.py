#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <contact>cervenkov@ipnp.troja.mff.cuni.cz</contact>
  <description>
....This module is part of the SVD validation suite. It retrieves
....the sensor type for a given sensorID.
  </description>
</header>
"""

import xml.etree.ElementTree as ET
from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2
from ROOT import gROOT, AddressOf


def getSensorType(sensorID):
    layerNum = sensorID.getLayerNumber()
    sensorNum = sensorID.getSensorNumber()
    sensorType = ''

    tree = ET.parse('../../../../data/SVD-Components.xml')
    root = tree.getroot()

    for ladder in root.iter('Ladder'):
        if int(ladder.get('layer')) == layerNum:
            for sensor in ladder.findall('Sensor'):
                if int(sensor.get('id')) == sensorNum:
                    sensorType = sensor.get('type')

    return sensorType


