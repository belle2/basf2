#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import basf2
from basf2 import *
from ROOT import Belle2
import ROOT as r
import os.path
import sys

##################################################################################
#
# test SVD geometry
#
# 1. add the database you want to test
# 2. > basf2 testSVDGeometry.py -i [any file with EventMetaData]
#
##################################################################################


class printSVDgeomety(basf2.Module):

    def beginRun(self):

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    print(str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber))

# add your database here


# Create paths
main = create_path()

main.add_module('RootInput')

main.add_module("Gearbox")
main.add_module('Geometry', useDB=True)

main.add_module(printSVDgeomety())

# main.add_module('Progress')
main.add_module('ProgressBar')

print_path(main)

# Process events
process(main)

print(statistics)
