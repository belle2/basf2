#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default Occupancy Calibration importer (MC).
Script to Import Calibrations into a local DB
"""
import basf2
from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2
from ROOT.Belle2 import SVDOccupancyCalibrations
import datetime
import os

now = datetime.datetime.now()

'''
# Phase 3 - scaled with 375
occupancy_L3_U = 2.48
occupancy_L3_V = 1.81
occupancy_bkw_U = 2.08
occupancy_bkw_V = 1.81
occupancy_origami_U = 2.40
occupancy_origami_V = 1.33
occupancy_fwd_U = 2.00
occupancy_fwd_V = 1.81
'''
# Phase 3 - scaled with rescaled gain
occupancy_L3 = 0.003
occupancy_allOtherLayers = 0.002


class defaultOccupancyImporter(basf2.Module):

    def beginRun(self):

        iov = Belle2.IntervalOfValidity.always()

        payload = Belle2.SVDOccupancyCalibrations.t_payload(-1, "OccupancyCalibrations_default_" +
                                                            str(now.isoformat()) + "_INFO:_fromPhase3calibrations")

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


use_local_database("localDB_occupancy/database.txt", "localDB_occupancy")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")  # , fileName="/geometry/Beast2_phase2.xml")
main.add_module("Geometry", components=['SVD'])

main.add_module(defaultOccupancyImporter())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)
