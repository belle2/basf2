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
SVD Default Clustering Parameters importer.
"""
import basf2 as b2
from ROOT import Belle2
from ROOT.Belle2 import SVDClusterCuts
from basf2 import conditions as b2conditions
import datetime

# import sys

# default values
# cls cuts
clsSeedSNR = 5
clsAdjSNR = 3
clsMinSNR = 0
now = datetime.datetime.now()


class defaultSVDClusteringImporter(b2.Module):
    """
    Defining the python module to do the import.
    """

    def beginRun(self):
        """
        call the functions to import the cluster parameters
        """
        iov = Belle2.IntervalOfValidity.always()

        # cluster reconstruction & position error
        clsParam = SVDClusterCuts()
        clsParam.minSeedSNR = clsSeedSNR
        clsParam.minAdjSNR = clsAdjSNR
        clsParam.minClusterSNR = clsMinSNR

        payload = Belle2.SVDClustering.t_payload(
            clsParam,
            "Clustering_default_" +
            str(
                now.isoformat()) +
            "_INFO:_seed=" +
            str(clsSeedSNR) +
            "_adj=" +
            str(clsAdjSNR) +
            "_cls=" +
            str(clsMinSNR))

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        print("setting SVD Clustering parameters for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber) + "." + str(side))

                        payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, clsParam)

        Belle2.Database.Instance().storeData(Belle2.SVDClustering.name, payload, iov)


b2conditions.prepend_globaltag('svd_onlySVDinGeoConfiguration')

main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")

main.add_module(defaultSVDClusteringImporter())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
