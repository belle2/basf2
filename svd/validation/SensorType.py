#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Package for parsing XML

"""
<header>
  <contact>cervenkov@ipnp.troja.mff.cuni.cz</contact>
  <description>
    This module is part of the SVD validation suite. It retrieves
    the sensor type for a given sensorID.
  </description>
</header>
"""

import xml.etree.ElementTree as ET
from basf2 import *


def getSensorType(sensorID):
    layerNum = sensorID.getLayerNumber()
    sensorNum = sensorID.getSensorNumber()
    sensorType = ''

    tree = ET.parse('../../../../data/SVD-Components.xml')
    root = tree.getroot()

    # Parse the XML tree to get the sensor type string
    for ladder in root.iter('Ladder'):
        if int(ladder.get('layer')) == layerNum:
            for sensor in ladder.findall('Sensor'):
                if int(sensor.get('id')) == sensorNum:
                    sensorType = sensor.get('type')

    return sensorType


