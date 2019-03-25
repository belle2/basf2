#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import basf2
from basf2 import *
from ROOT import Belle2
import math as m
import ROOT as r
import os.path
import sys

##################################################################################
#
# test SVD geometry
#
# 1. add the database you want to test
# 2. > basf2 testSVDGeometry.py -i [any file with EventMetaData]
# NOTE: the input file should have the correct IoV that you want to test
#
##################################################################################


class printSVDSensors(basf2.Module):

    def beginRun(self):

        geoCache = Belle2.VXD.GeoCache.getInstance()

        print("printing all sensors of the SVD in the geometry:")
        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    print(str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber))


class printSVDLadders(basf2.Module):

    def beginRun(self):

        geoCache = Belle2.VXD.GeoCache.getInstance()

        print("printing all L3 backward sensors position of the SVD in the geometry:")
        print("format: sensorID X(cm) Y(cm) phi(rad) phi(deg)")
        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()

            if not layerNumber == 3:
                continue

            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    if not(sensorNumber == layerNumber - 1):
                        continue
                    # sensor ID string
                    sensorID = str(layerNumber) + "." + str(ladderNumber)
                    # get global coordinates:
                    local = r.TVector3(0, 0, 0)
                    glob = geoCache.getSensorInfo(sensor).pointToGlobal(local)

                    angleRad = m.atan2(glob.Y(), glob.X())
                    print(angleRad)
                    if(angleRad < 0):
                        angleRad = 2 * m.pi + angleRad
                        print("second = " + str(angleRad))

                    angleDeg = m.degrees(angleRad)

                    xStr = "{0:.2f}".format(round(glob.X(), 2))
                    yStr = "{0:.2f}".format(round(glob.Y(), 2))
                    radStr = "{0:.2f}".format(round(angleRad, 4))
                    degStr = "{0:.2f}".format(round(angleDeg, 2))
                    print(sensorID + '     ' + xStr + '     ' + yStr + '     ' + radStr + '     ' + degStr)


# add your database here
# use_database_chain()
# use_central_database("release-02-00-00")
# use_central_database("release-02-01-00")
# use_central_database("master_2018-12-24")
# use_central_database("release-03-00-02")

# Create paths
main = create_path()

main.add_module('RootInput')

main.add_module("Gearbox")
main.add_module('Geometry', useDB=True)

main.add_module(printSVDSensors())
main.add_module(printSVDLadders())

print_path(main)

# Process events
process(main)

print(statistics)
