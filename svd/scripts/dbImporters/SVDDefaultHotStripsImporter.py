#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
SVD Default HotStrips Calibration importer.
Script to Import Calibrations into a local DB
"""
import basf2 as b2
from ROOT import Belle2
import datetime
from basf2 import conditions as b2conditions

now = datetime.datetime.now()


class defaultHotStripsImporter(b2.Module):
    '''default importer for hot strips'''

    def beginRun(self):
        '''begin run'''

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


b2conditions.prepend_globaltag("svd_onlySVDinGeoConfiguration")

main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")

main.add_module(defaultHotStripsImporter())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
