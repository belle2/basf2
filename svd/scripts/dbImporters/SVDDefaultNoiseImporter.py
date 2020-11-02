#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default Noise Calibration importer (MC).
Script to Import Calibrations into a local DB
"""
import basf2 as b2
from ROOT import Belle2
import datetime
from basf2 import conditions as b2conditions

now = datetime.datetime.now()

'''
# Phase 3 - scaled with 375
noise_L3_U = 2.48
noise_L3_V = 1.81
noise_bkw_U = 2.08
noise_bkw_V = 1.81
noise_origami_U = 2.40
noise_origami_V = 1.33
noise_fwd_U = 2.00
noise_fwd_V = 1.81
'''
# Phase 3 - scaled with rescaled gain
noise_L3_U = 3.29
noise_L3_V = 2.46
noise_bkw_U = 3.12
noise_bkw_V = 2.75
noise_origami_U = 3.32
noise_origami_V = 2.28
noise_fwd_U = 3.03
noise_fwd_V = 2.76
'''
# TestBeam
noise_L3_U = 2.51
noise_L3_V = 1.73
noise_bkw_U = -1
noise_bkw_V = -1
noise_origami_U = 2.88
noise_origami_V = 1.73
noise_fwd_U = -1
noise_fwd_V = -1
'''


class defaultNoiseImporter(b2.Module):
    ''' default strip noise importer'''

    def beginRun(self):
        '''begin run'''

        iov = Belle2.IntervalOfValidity.always()

        payload = Belle2.SVDNoiseCalibrations.t_payload(-1, "NoiseCalibrations_default_" +
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
                        print("setting Noise for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber) + "." + str(side))
                        if side == 0:  # V
                            if layerNumber == 3:  # L3 V
                                noise = noise_L3_V
                            else:
                                Nstrips = 512
                                if sensorNumber == 1:  # FW V
                                    noise = noise_fwd_V
                                else:  # BKW V
                                    if sensorNumber == layerNumber - 1:  # FW V
                                        noise = noise_bkw_V
                                    else:  # BARREL V
                                        noise = noise_origami_V
                        if side == 1:  # U
                            if layerNumber == 3:  # L3 U
                                noise = noise_L3_U
                            else:
                                if sensorNumber == 1:  # FW U
                                    noise = noise_fwd_U
                                else:  # BKW U
                                    if sensorNumber == layerNumber - 1:  # FW U
                                        noise = noise_bkw_U
                                    else:  # BARREL U
                                        noise = noise_origami_U

                        print(str(Nstrips))
                        for strip in range(0, Nstrips):
                            payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), strip, noise)

        Belle2.Database.Instance().storeData(Belle2.SVDNoiseCalibrations.name, payload, iov)


b2conditions.prepend_globaltag("svd_onlySVDinGeoConfiguration")

main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")

main.add_module(defaultNoiseImporter())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
