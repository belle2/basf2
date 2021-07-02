#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default CoG Time Calibration importer.
Script to Import Calibrations into a local DB
"""
import basf2 as b2
from ROOT import Belle2
import datetime

now = datetime.datetime.now()


class defaultCrossTalkStripsImporter(b2.Module):
    '''default cross talk strips importer'''

    def beginRun(self):
        '''begin run'''

        iov = Belle2.IntervalOfValidity.always()

        payload = Belle2.SVDCrossTalkStripsCalibrations.t_payload(
            0, "CrossTalkStrips_default_" + str(now.isoformat()) + "_INFO:_noCrossTalkstrips")
        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        print("setting crossTalk strips default value (0, good strip) for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber))
                        payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, 0)

        Belle2.Database.Instance().storeData(Belle2.SVDCrossTalkStripsCalibrations.name, payload, iov)


main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")
main.add_module(defaultCrossTalkStripsImporter())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
