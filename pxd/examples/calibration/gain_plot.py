#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Creates overview plots for gain calibrations
#
# At first, you can extract the gain calibration payloads from a localdb/centraldb using the tool
#
# b2conditionsdb-extract --exp 3 --runs 2-2 --tag localdb/database.txt --output gain_payloads.root  PXDGainMapPar
#
# Secondly, execute the script as
#
# basf2 gain_plot.py
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de

import os
import sys
from basf2 import *
import ROOT
from ROOT import Belle2

sensor_list = [Belle2.VxdID("1.1.1"), Belle2.VxdID("1.1.2"), Belle2.VxdID("2.1.1"), Belle2.VxdID("2.1.2")]

rfile = ROOT.TFile("gain_payloads.root", "UPDATE")
tree = rfile.Get("conditions")


# Baseline values from PXDDigitizer
ADCUnit = 130.0
Gq = 0.6


for condition in tree:

    if condition.PXDGainMapPar_valid:
        for sensorID in sensor_list:

            layer = sensorID.getLayerNumber()
            ladder = sensorID.getLadderNumber()
            sensor = sensorID.getSensorNumber()

            name = "Gains_{:d}_{:d}_{:d}_run_{:d}".format(layer, ladder, sensor, condition.run)
            title = "Relative energy calibration Sensor={:d}.{:d}.{:d} run={:d}".format(layer, ladder, sensor, condition.run)
            gain_map = ROOT.TH2F(name, title, 4, 1 - 0.5, 5 - 0.5, 6, 1 - 0.5, 7 - 0.5)
            gain_map.GetXaxis().SetTitle("DCD")
            gain_map.GetYaxis().SetTitle("SWB")
            gain_map.GetZaxis().SetTitle("rel. gain factor")
            gain_map.SetStats(0)

            name = "AbsGains_{:d}_{:d}_{:d}_run_{:d}".format(layer, ladder, sensor, condition.run)
            title = "Energy calibration Sensor={:d}.{:d}.{:d} run={:d}".format(layer, ladder, sensor, condition.run)
            abs_gain_map = ROOT.TH2F(name, title, 4, 1 - 0.5, 5 - 0.5, 6, 1 - 0.5, 7 - 0.5)
            abs_gain_map.GetXaxis().SetTitle("DCD")
            abs_gain_map.GetYaxis().SetTitle("SWB")
            abs_gain_map.GetZaxis().SetTitle("gain factor [eV/ADU]")
            abs_gain_map.SetStats(0)

            for chipID in range(24):
                iDCD = chipID / 6 + 1
                iSWB = chipID % 6 + 1
                gain = condition.PXDGainMapPar.getGainCorrection(sensorID.getID(), chipID)
                gain_map.SetBinContent(int(iDCD), int(iSWB), gain)
                abs_gain_map.SetBinContent(int(iDCD), int(iSWB), 3.65 * ADCUnit / Gq / gain)

            gain_map.Write()
            abs_gain_map.Write()

rfile.Write()
rfile.Close()
