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
SVD Default Hit Time Selection importer.
"""
import basf2 as b2
from ROOT import Belle2
from ROOT.Belle2 import SVDHitTimeSelectionFunction
from basf2 import conditions as b2conditions
import datetime

# import sys

# default values
# cls hit time
clsTimeMin = -999
clsTimeFunctionID = 0  # default, t > clsTimeMin
clsTimeDeltaT = 30  # NOT USED
clsTimeNSigma = 10  # NOT USED

# time difference U-V
clsTimeDiff = 999  # default for DATA and exp 1003, 1002
# clsTimeDiff = 10  # default for exp 0

now = datetime.datetime.now()


class defaultSVDHitTimeSelectionImporter(b2.Module):
    """
    Defining the python module to do the import.
    """

    def beginRun(self):
        """
        call the functions to import the cluster parameters
        """
        iov = Belle2.IntervalOfValidity.always()

        # SpacePoint time
        hitTimeSelection = SVDHitTimeSelectionFunction()
        # set default version = 0
        hitTimeSelection.setFunctionID(clsTimeFunctionID)
        # version 0: t > tMin
        hitTimeSelection.setMinTime(clsTimeMin)
        # version 1: |t-t0|<deltaT - NOT USED
        hitTimeSelection.setDeltaTime(clsTimeDeltaT)
        # version 2: |t-t0|<nSgma*tErrTOT - NOT USED
        hitTimeSelection.setNsigma(clsTimeNSigma)
        # cluster time difference
        hitTimeSelection.setMaxUVTimeDifference(clsTimeDiff)
        payload = Belle2.SVDHitTimeSelection.t_payload(
            hitTimeSelection, "HitTimeSelection_noCuts_" + str(now.isoformat()) +
            "_INFO:_tmin=" + str(clsTimeMin) + "_tDiff=" + str(clsTimeDiff))

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        print("setting SVD Hit Time Selections for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber) + "." + str(side))

        Belle2.Database.Instance().storeData(Belle2.SVDHitTimeSelection.name, payload, iov)


b2conditions.prepend_globaltag('svd_onlySVDinGeoConfiguration')

main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")

main.add_module(defaultSVDHitTimeSelectionImporter())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
