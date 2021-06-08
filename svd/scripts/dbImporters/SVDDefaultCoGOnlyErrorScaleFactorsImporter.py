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
clsScaleErrSize4 = 1
clsScaleErrSize5 = 1
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
        clsParam.scaleError_clSize4 = clsScaleErrSize4
        clsParam.scaleError_clSize5 = clsScaleErrSize5

        cls_payload = Belle2.SVDCoGOnlyErrorScaleFactors.t_payload(
            clsParam,
            "CoGOnlyErrorScaleFactors_default_" +
            str(
                now.isoformat()) +
            "_INFO:_scaleFactors=1")

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
