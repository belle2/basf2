#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default FADC Masked Strips Calibration importer.
Script to Import Calibrations into a local DB
"""
import basf2
from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2
from ROOT.Belle2 import SVDFADCMaskedStrips
import datetime
import os

now = datetime.datetime.now()


class defaultFADCMaskedStripsImporter(basf2.Module):

    def beginRun(self):

        iov = Belle2.IntervalOfValidity.always()

        payload = Belle2.SVDFADCMaskedStrips.t_payload(
            0, "FADCMaskedStrips_default_" + str(now.isoformat()) + "_INFO:_noFADCMaskedstrips")

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        Nstrips = 768
                        print("setting masked strips default value (0, not masked) for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber))
                        if side == 0 and not layerNumber == 3:  # non-L3 V side
                            Nstrips = 512

                        for strip in range(0, Nstrips):
                            payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), strip, 0)

                            # mask something, uncomment the following 2 lines:
                            # if(layerNumber == 3 and ladderNumber == 2 and sensorNumber == 2 and side == 0):
                            #    payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), strip, 1)

        Belle2.Database.Instance().storeData(Belle2.SVDFADCMaskedStrips.name, payload, iov)


use_database_chain()
use_central_database("svd_onlySVDinGeoConfiguration")
use_local_database("localDB_defaultFADCMaskedStrips/database.txt", "localDB_defaultFADCMaskedStrips")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")

main.add_module(defaultFADCMaskedStripsImporter())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)
