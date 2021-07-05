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
SVD Default CoGOnly Position Error Formulas importer.
"""
import basf2 as b2
from ROOT import Belle2
from ROOT.Belle2 import SVDPositionErrorFunction
from basf2 import conditions as b2conditions
import datetime

# import sys

# default values
# read the svd/dbobjects/SVDPositionErrorFunction class
# now replicates the same formulas as in release-05

now = datetime.datetime.now()


class defaultSVDCoGOnlyPositionErrorImporter(b2.Module):
    """
    Defining the python module to do the import.
    """

    def beginRun(self):
        """
        call the functions to import the cluster parameters
        """
        iov = Belle2.IntervalOfValidity.always()

        # cluster position error
        clsParam = SVDPositionErrorFunction()

        cls_payload = Belle2.SVDCoGOnlyPositionError.t_payload(
            clsParam,
            "CoGOnlyPositionError_default_" +
            str(
                now.isoformat()) +
            "_INFO:_formulas=rel05")

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        print("setting CoGOnly Position Error for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber) + "." + str(side))
                        cls_payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, clsParam)

        Belle2.Database.Instance().storeData(Belle2.SVDCoGOnlyPositionError.name, cls_payload, iov)


b2conditions.prepend_globaltag('svd_onlySVDinGeoConfiguration')

main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")

main.add_module(defaultSVDCoGOnlyPositionErrorImporter())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
