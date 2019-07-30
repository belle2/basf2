#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default HotStrips Calibration importer.
Script to Import Calibrations into a local DB
"""
import basf2
from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2
from ROOT.Belle2 import SVDHotStripsCalibrations
import datetime
import os

now = datetime.datetime.now()


class defaultHotStripsImporter(basf2.Module):

    def beginRun(self):

        iov = Belle2.IntervalOfValidity.always()
        #      iov = IntervalOfValidity(0,0,-1,-1)

        payload = Belle2.SVDHotStripsCalibrations.t_payload(0, "HotStrips_default_" + str(now.isoformat()) + "_INFO:_noHotstrips")

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        Nstrips = 768
                        print("setting hot strips default value (0, good strip) for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber))

                        if side == 0 and not layerNumber == 3:  # non-L3 V side
                            Nstrips = 512

                        for strip in range(0, Nstrips):
                            payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, 0)

        Belle2.Database.Instance().storeData(Belle2.SVDHotStripsCalibrations.name, payload, iov)


use_database_chain()
use_central_database("svd_onlySVDinGeoConfiguration")
use_local_database("localDB_defaultHotStripsCalibrations/database.txt", "localDB_defaultHotStripsCalibrations")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox", fileName="/geometry/Beast2_phase2.xml")
main.add_module("Geometry", components=['SVD'])  # , useDB = True)

main.add_module(defaultHotStripsImporter())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)
