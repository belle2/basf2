#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Charge Simulation Calibration importer (MC).
Script to import charge coupling constants for simulation into a local DB
"""
import basf2 as b2
from ROOT import Belle2
from ROOT.Belle2 import SVDChargeSimCal
import datetime
from basf2 import conditions as b2conditions

now = datetime.datetime.now()


# release-05 equivalent values:
# c0,1,2,3 = coupling constants
# ew = electronWeight

# L3 u/V
ew_L3_U = 0.8828
c0_L3_U = 0.9305
c1_L3_U = 0.448
c2_L3_U = 0.03
c3_L3_U = 0
ew_L3_V = 0.8529
c0_L3_V = 0.9782
c1_L3_V = 0.3713
c2_L3_V = 0.0065
c3_L3_V = 0
# forward U/V
ew_fw_U = 0.9163
c0_fw_U = 0.964
c1_fw_U = 0.4295
c2_fw_U = 0.0139
c3_fw_U = 0
ew_fw_V = 0.9486
c0_fw_V = 0.9677
c1_fw_V = 0.3552
c2_fw_V = 0.0091
c3_fw_V = 0
# backward U/V
ew_bk_U = 0.9128
c0_bk_U = 0.9665
c1_bk_U = 0.4202
c2_bk_U = 0.0125
c3_bk_U = 0
ew_bk_V = 0.9
c0_bk_V = 0.9687
c1_bk_V = 0.3615
c2_bk_V = 0.0091
c3_bk_V = 0
# origami U/v
ew_or_U = 0.9138
c0_or_U = 0.9665
c1_or_U = 0.4202
c2_or_U = 0.0125
c3_or_U = 0
ew_or_V = 0.9472
c0_or_V = 0.9687
c1_or_V = 0.3615
c2_or_V = 0.0091
c3_or_V = 0


class defaultChargeSimulationImporter(b2.Module):
    ''' default coupling constants importer'''

    def beginRun(self):
        '''begin run'''

        iov = Belle2.IntervalOfValidity.always()

        chargeSimCal = SVDChargeSimCal()

        payload = Belle2.SVDChargeSimulationCalibrations.t_payload(chargeSimCal, "ChargeSimulationCalibrations_default_" +
                                                                   str(now.isoformat()) + "_INFO:_rel05values")

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        print("setting ChargeSimulation for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber) + "." + str(side))
                        if side == 0:  # V
                            if layerNumber == 3:  # L3 V
                                c0 = c0_L3_V
                                c1 = c1_L3_V
                                c2 = c2_L3_V
                                c3 = c3_L3_V
                                ew = ew_L3_V
                            else:
                                if sensorNumber == 1:  # FW V
                                    c0 = c0_fw_V
                                    c1 = c1_fw_V
                                    c2 = c2_fw_V
                                    c3 = c3_fw_V
                                    ew = ew_fw_V
                                else:
                                    if sensorNumber == layerNumber - 1:  # BK V
                                        c0 = c0_bk_V
                                        c1 = c1_bk_V
                                        c2 = c2_bk_V
                                        c3 = c3_bk_V
                                        ew = ew_bk_V
                                    else:  # BARREL V
                                        c0 = c0_or_V
                                        c1 = c1_or_V
                                        c2 = c2_or_V
                                        c3 = c3_or_V
                                        ew = ew_or_V
                        if side == 1:  # U
                            if layerNumber == 3:  # L3 U
                                c0 = c0_L3_U
                                c1 = c1_L3_U
                                c2 = c2_L3_U
                                c3 = c3_L3_U
                                ew = ew_L3_U
                            else:
                                if sensorNumber == 1:  # FW U
                                    c0 = c0_fw_U
                                    c1 = c1_fw_U
                                    c2 = c2_fw_U
                                    c3 = c3_fw_U
                                    ew = ew_fw_U
                                else:  # BKW U
                                    if sensorNumber == layerNumber - 1:  # BK U
                                        c0 = c0_bk_U
                                        c1 = c1_bk_U
                                        c2 = c2_bk_U
                                        c3 = c3_bk_U
                                        ew = ew_bk_U
                                    else:  # BARREL U
                                        c0 = c0_or_U
                                        c1 = c1_or_U
                                        c2 = c2_or_U
                                        c3 = c3_or_U
                                        ew = ew_or_U
                        # this insert does not work
                        chargeSimCal.couplingConstant['C0'] = c0
                        chargeSimCal.couplingConstant['C1'] = c1
                        chargeSimCal.couplingConstant['C2'] = c2
                        chargeSimCal.couplingConstant['C3'] = c3
                        chargeSimCal.electronWeight = ew
                        payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, chargeSimCal)

        Belle2.Database.Instance().storeData(Belle2.SVDChargeSimulationCalibrations.name, payload, iov)


b2conditions.prepend_globaltag("svd_onlySVDinGeoConfiguration")

main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")

main.add_module(defaultChargeSimulationImporter())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)