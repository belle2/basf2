#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default OldDefault Error Scaling Factors importer.
"""
import basf2 as b2
from ROOT import Belle2
from ROOT.Belle2 import SVDPosErrScaleFactors
from basf2 import conditions as b2conditions
import datetime

# import sys

# default values
clsScaleErrSize1 = 1
clsScaleErrSize2 = 1
clsScaleErrSize3 = 1
now = datetime.datetime.now()


class defaultSVDOldDefaultErrorScaleFactorsImporter(b2.Module):
    """
    Defining the python module to do the import.
    """

    def beginRun(self):
        """
        call the functions to import the cluster parameters
        """
        iov = Belle2.IntervalOfValidity.always()

        # cluster position error
        clsParam = SVDPosErrScaleFactors()
        clsParam.scaleError_clSize1 = clsScaleErrSize1
        clsParam.scaleError_clSize2 = clsScaleErrSize2
        clsParam.scaleError_clSize3 = clsScaleErrSize3

        cls_payload = Belle2.SVDOldDefaultErrorScaleFactors.t_payload(
            clsParam,
            "OldDefaultErrorScaleFactors_default_" +
            str(
                now.isoformat()) +
            "_INFO:_scaleFactors=fromSimulation")
#            "_INFO:_scaleFactors=1")

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        print("setting OldDefault Error Scale Factors for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber) + "." + str(side))

                        if side == 0:  # V
                            if layerNumber == 3:  # L3 V
                                clsParam.scaleError_clSize1 = 1.664
                                clsParam.scaleError_clSize2 = 0.977
                                clsParam.scaleError_clSize3 = 0.446
                            else:
                                if sensorNumber == 1:  # FW V
                                    clsParam.scaleError_clSize1 = 1.859
                                    clsParam.scaleError_clSize2 = 1.185
                                    clsParam.scaleError_clSize3 = 0.459
                                else:  # BARREL V
                                    clsParam.scaleError_clSize1 = 2.082
                                    clsParam.scaleError_clSize2 = 1.218
                                    clsParam.scaleError_clSize3 = 0.510
                        if side == 1:  # U
                            if layerNumber == 3:  # L3 U
                                clsParam.scaleError_clSize1 = 1.301
                                clsParam.scaleError_clSize2 = 0.711
                                clsParam.scaleError_clSize3 = 0.619
                            else:
                                if sensorNumber == 1:  # FW U
                                    clsParam.scaleError_clSize1 = 2.196
                                    clsParam.scaleError_clSize2 = 0.889
                                    clsParam.scaleError_clSize3 = 0.700
                                else:  # BARREL U
                                    clsParam.scaleError_clSize1 = 0.813
                                    clsParam.scaleError_clSize2 = 0.693
                                    clsParam.scaleError_clSize3 = 0.630

                        print(" size 1 = " + str(clsParam.scaleError_clSize1) + ", size 2 = " +
                              str(clsParam.scaleError_clSize2) + ", size >2 = " + str(clsParam.scaleError_clSize3))

                        cls_payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, clsParam)

        Belle2.Database.Instance().storeData(Belle2.SVDOldDefaultErrorScaleFactors.name, cls_payload, iov)


b2conditions.prepend_globaltag('svd_onlySVDinGeoConfiguration')

main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")

main.add_module(defaultSVDOldDefaultErrorScaleFactorsImporter())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
