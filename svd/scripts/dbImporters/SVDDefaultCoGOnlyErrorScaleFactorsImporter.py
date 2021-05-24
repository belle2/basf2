#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default CoGOnly Error Scaling Factors importer.
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


class defaultSVDCoGOnlyErrorScaleFactorsImporter(b2.Module):
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

        cls_payload = Belle2.SVDCoGOnlyErrorScaleFactors.t_payload(
            clsParam,
            "CoGOnlyErrorScaleFactors_default_" +
            str(
                now.isoformat()) +
            "_INFO:_scaleFactors=fromSimulation")

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        print("setting CoGOnly Error Scale Factors for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber) + "." + str(side))

                        if side == 0:  # V
                            if layerNumber == 3:  # L3 V
                                clsParam.scaleError_clSize1 = 1.582
                                clsParam.scaleError_clSize2 = 0.987
                                clsParam.scaleError_clSize3 = 3.427
                            else:
                                if sensorNumber == 1:  # FW V
                                    clsParam.scaleError_clSize1 = 1.822
                                    clsParam.scaleError_clSize2 = 1.189
                                    clsParam.scaleError_clSize3 = 1.788
                                else:  # BARREL V
                                    clsParam.scaleError_clSize1 = 2.098
                                    clsParam.scaleError_clSize2 = 1.210
                                    clsParam.scaleError_clSize3 = 2.293
                        if side == 1:  # U
                            if layerNumber == 3:  # L3 U
                                clsParam.scaleError_clSize1 = 3.202
                                clsParam.scaleError_clSize2 = 0.757
                                clsParam.scaleError_clSize3 = 2.231
                            else:
                                if sensorNumber == 1:  # FW U
                                    clsParam.scaleError_clSize1 = 2.718
                                    clsParam.scaleError_clSize2 = 0.889
                                    clsParam.scaleError_clSize3 = 2.198
                                else:  # BARREL U
                                    clsParam.scaleError_clSize1 = 0.831
                                    clsParam.scaleError_clSize2 = 0.682
                                    clsParam.scaleError_clSize3 = 1.737
                        print(" size 1 = " + str(clsParam.scaleError_clSize1) + ", size 2 = " +
                              str(clsParam.scaleError_clSize2) + ", size >2 = " + str(clsParam.scaleError_clSize3))

                        cls_payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, clsParam)

        Belle2.Database.Instance().storeData(Belle2.SVDCoGOnlyErrorScaleFactors.name, cls_payload, iov)


b2conditions.prepend_globaltag('svd_onlySVDinGeoConfiguration')

main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")

main.add_module(defaultSVDCoGOnlyErrorScaleFactorsImporter())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
