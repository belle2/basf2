#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default PulseShape Calibration importer (MC).
Script to Import Calibrations into a local DB
"""
import basf2
from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2
from ROOT.Belle2 import SVDStripCalAmp
from ROOT.Belle2 import SVDTriggerBinDependentConstants

import os

timeShift = 0
timeShiftTBDep = 0
peakTime = 75
pulseWidth = 130

# gain here is actually ADUequivalent (e-/ADC)
# we set the INVERSE of it in the payload, so that the payload actually contains the gain
'''
# Phase 3 - old
gain_L3_U = 375
gain_L3_V = 375
gain_bkw_U = 375
gain_bkw_V = 375
gain_origami_U = 375
gain_origami_V = 375
gain_fwd_U = 375
gain_fwd_V = 375
'''
# Phase 3 - matching data
gain_L3_U = 282.5
gain_L3_V = 275.5
gain_bkw_U = 250.1
gain_bkw_V = 246.6
gain_origami_U = 271.4
gain_origami_V = 218.8
gain_fwd_U = 247.4
gain_fwd_V = 245.7


class defaultPulseShapeImporter(basf2.Module):

    def beginRun(self):

        iov = Belle2.IntervalOfValidity.always()

        # gain, peakTime,
        calAmp_payload = Belle2.SVDPulseShapeCalibrations.t_calAmp_payload()
        tmp_calAmp = SVDStripCalAmp()
        tmp_calAmp.gain = -1  # set after the loop on sensors
        tmp_calAmp.peakTime = peakTime
        tmp_calAmp.pulseWidth = pulseWidth
        # time shift correction (old correction: set to 0)
        time_payload = Belle2.SVDPulseShapeCalibrations.t_time_payload()

        # trigger-bin dependent time shift correction (old correction: set to 0)
        bin_payload = Belle2.SVDPulseShapeCalibrations.t_bin_payload()
        tmp_bin = SVDTriggerBinDependentConstants()
        tmp_bin.bin0 = timeShiftTBDep
        tmp_bin.bin1 = timeShiftTBDep
        tmp_bin.bin2 = timeShiftTBDep
        tmp_bin.bin3 = timeShiftTBDep

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        Nstrips = 768
                        print("setting PulseShape for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber) + "." + str(side))
                        if side == 0:  # V
                            if layerNumber == 3:  # L3 V
                                gain = gain_L3_V
                            else:
                                Nstrips = 512
                                if sensorNumber == 1:  # FW V
                                    gain = gain_fwd_V
                                else:  # BKW V
                                    if sensorNumber == layerNumber - 1:  # FW V
                                        gain = gain_bkw_V
                                    else:  # BARREL V
                                        gain = gain_origami_V
                        if side == 1:  # U
                            if layerNumber == 3:  # L3 U
                                gain = gain_L3_U
                            else:
                                if sensorNumber == 1:  # FW U
                                    gain = gain_fwd_U
                                else:  # BKW U
                                    if sensorNumber == layerNumber - 1:  # FW U
                                        gain = gain_bkw_U
                                    else:  # BARREL U
                                        gain = gain_origami_U

                        tmp_calAmp.gain = 1 / gain
                        print(str(Nstrips))
                        for strip in range(0, Nstrips):
                            print("setting Gain for strip " + str(strip) + " to " + str(tmp_calAmp.gain))

                            time_payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), strip, timeShift)
                            calAmp_payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), strip, tmp_calAmp)
                            bin_payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), strip, tmp_bin)

        Belle2.Database.Instance().storeData(Belle2.SVDPulseShapeCalibrations.time_name, time_payload, iov)
        Belle2.Database.Instance().storeData(Belle2.SVDPulseShapeCalibrations.calAmp_name, calAmp_payload, iov)
        Belle2.Database.Instance().storeData(Belle2.SVDPulseShapeCalibrations.bin_name, bin_payload, iov)


use_local_database("localDB/database.txt", "localDB")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")  # , fileName="/geometry/Beast2_phase2.xml")
main.add_module("Geometry", components=['SVD'])

main.add_module(defaultPulseShapeImporter())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)
