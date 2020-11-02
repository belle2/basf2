#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default Cluster Calibration importer.
t_min = -80,
"""
import basf2 as b2
from ROOT import Belle2
from ROOT.Belle2 import SVDClusterCuts
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

# cls cuts
clsSeedSNR = 5
clsAdjSNR = 3
clsMinSNR = 0
clsScaleErrSize1 = 1
clsScaleErrSize2 = 1
clsScaleErrSize3 = 1
now = datetime.datetime.now()


class defaultSVDClusterCalibrationImporter(b2.Module):
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
        time_payload = Belle2.SVDClusterCalibrations.t_time_payload(
            hitTimeSelection, "HitTimeSelection_default_" + str(now.isoformat()) +
            "_INFO:_tmin="+str(clsTimeMin)+"_tDiff="+str(clsTimeDiff))

        # cluster reconstruction & position error
        clsParam = SVDClusterCuts()
        clsParam.minSeedSNR = clsSeedSNR
        clsParam.minAdjSNR = clsAdjSNR
        clsParam.minClusterSNR = clsMinSNR
        clsParam.scaleError_clSize1 = clsScaleErrSize1
        clsParam.scaleError_clSize2 = clsScaleErrSize2
        clsParam.scaleError_clSize3 = clsScaleErrSize3

        cls_payload = Belle2.SVDClusterCalibrations.t_payload(
            clsParam,
            "ClusterCalibrations_default_" +
            str(
                now.isoformat()) +
            "_INFO:_seed=" +
            str(clsSeedSNR) +
            "_adj=" +
            str(clsAdjSNR) +
            "_cls=" +
            str(clsMinSNR) +
            "_scaleFactors=fromSimulation")

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        print("setting SVDCluster calibrations for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber) + "." + str(side))

                        if side == 0:  # V
                            if layerNumber == 3:  # L3 V
                                clsParam.scaleError_clSize1 = 1.638
                                clsParam.scaleError_clSize2 = 1.168
                                clsParam.scaleError_clSize3 = 0.430
                            else:
                                if sensorNumber == 1:  # FW V
                                    clsParam.scaleError_clSize1 = 1.766
                                    clsParam.scaleError_clSize2 = 1.481
                                    clsParam.scaleError_clSize3 = 0.433
                                else:  # BARREL V
                                    clsParam.scaleError_clSize1 = 2.338
                                    clsParam.scaleError_clSize2 = 1.418
                                    clsParam.scaleError_clSize3 = 0.468
                        if side == 1:  # U
                            if layerNumber == 3:  # L3 U
                                clsParam.scaleError_clSize1 = 1.352
                                clsParam.scaleError_clSize2 = 1.137
                                clsParam.scaleError_clSize3 = 0.559
                            else:
                                if sensorNumber == 1:  # FW U
                                    clsParam.scaleError_clSize1 = 1.728
                                    clsParam.scaleError_clSize2 = 1.209
                                    clsParam.scaleError_clSize3 = 0.662
                                else:  # BARREL U
                                    clsParam.scaleError_clSize1 = 1.312
                                    clsParam.scaleError_clSize2 = 0.871
                                    clsParam.scaleError_clSize3 = 0.538
                        print(" size 1 = " + str(clsParam.scaleError_clSize1) + ", size 2 = " +
                              str(clsParam.scaleError_clSize2) + ", size >2 = " + str(clsParam.scaleError_clSize3))

                        cls_payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, clsParam)

        Belle2.Database.Instance().storeData(Belle2.SVDClusterCalibrations.name, cls_payload, iov)
        Belle2.Database.Instance().storeData(Belle2.SVDClusterCalibrations.time_name, time_payload, iov)


b2conditions.prepend_globaltag('svd_onlySVDinGeoConfiguration')

main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")

main.add_module(defaultSVDClusterCalibrationImporter())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
