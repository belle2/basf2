#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default CoG Time Calibration importer.
Script to Import Calibrations into a local DB
"""
import basf2
from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2
from ROOT.Belle2 import SVDCoGCalibrationFunction
from ROOT.Belle2 import SVDCoGTimeCalibrations

import os


class defaultCoGTimeCalibrationImporter(basf2.Module):

    def beginRun(self):

        iov = Belle2.IntervalOfValidity.always()
        #      iov = IntervalOfValidity(0,0,-1,-1)

        payload = Belle2.SVDCoGTimeCalibrations.t_payload()

        timeCal = SVDCoGCalibrationFunction()
        timeCal.set_bias(8., 0., 0., 0.)
        timeCal.set_scale(42., 1., 1., 1.)

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        print("setting CoG calibration for " + str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber))
                        payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, timeCal)

        Belle2.Database.Instance().storeData("SVDCoGTimeCalibrations", payload, iov)


use_local_database("localDB/database.txt", "localDB")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox", fileName="/geometry/Beast2_phase2.xml")
main.add_module("Geometry", components=['SVD'])  # , useDB = True)

main.add_module(defaultCoGTimeCalibrationImporter())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)
