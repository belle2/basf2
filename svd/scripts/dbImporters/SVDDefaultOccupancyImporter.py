#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default Occupancy Calibration importer (MC).
Script to Import Calibrations into a local DB
"""
import basf2 as b2
from ROOT import Belle2
import datetime

now = datetime.datetime.now()

# Phase 3 - exp 10
occupancy_L3 = 0.003
occupancy_allOtherLayers = 0.002


class defaultOccupancyImporter(b2.Module):
    ''' default importer of strip occupancy'''

    def beginRun(self):
        '''begin run'''

        iov = Belle2.IntervalOfValidity.always()

        payload = Belle2.SVDOccupancyCalibrations.t_payload(-1, "OccupancyCalibrations_default_" +
                                                            str(now.isoformat()) + "_INFO:_testJamesBranch")

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        Nstrips = 768
                        print("setting Occupancy for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber) + "." + str(side))
                        if side == 0:
                            if layerNumber == 3:  # L3
                                occupancy = occupancy_L3
                            else:
                                Nstrips = 512
                                occupancy = occupancy_allOtherLayers
                        elif side == 1:  # U
                            if layerNumber == 3:  # L3 U
                                occupancy = occupancy_L3
                            else:
                                occupancy = occupancy_allOtherLayers
                        else:
                            print("WARNING: sensors end!...")

                        print(str(Nstrips))

                        for strip in range(0, Nstrips):
                            payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), strip, occupancy)

        Belle2.Database.Instance().storeData(Belle2.SVDOccupancyCalibrations.name, payload, iov)


main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")

main.add_module(defaultOccupancyImporter())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
