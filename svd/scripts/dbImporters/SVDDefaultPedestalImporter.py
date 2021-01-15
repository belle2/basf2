#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default Pedestal Calibration importer (MC).
Script to Import Calibrations into a local DB
"""
import basf2 as b2
from ROOT import Belle2
import datetime

now = datetime.datetime.now()

# fake pedestals for MC
pedestal = 0


class defaultPedestalImporter(b2.Module):
    """strip pedestal importer"""

    def beginRun(self):
        """do everything here"""

        iov = Belle2.IntervalOfValidity.always()

        payload = Belle2.SVDPedestalCalibrations.t_payload(-1, "PedestalCalibrations_default_" +
                                                           str(now.isoformat()) + "_INFO:_fakeNULLpedestals")

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        Nstrips = 768
                        if side == 0:
                            Nstrips = 512
                            if layerNumber == 3:  # L3
                                Nstrips = 768

                        for strip in range(0, Nstrips):
                            payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), strip, pedestal)

        Belle2.Database.Instance().storeData(Belle2.SVDPedestalCalibrations.name, payload, iov)


main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")  # , fileName="/geometry/Beast2_phase2.xml")
main.add_module("Geometry", components=['SVD'])

main.add_module(defaultPedestalImporter())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
