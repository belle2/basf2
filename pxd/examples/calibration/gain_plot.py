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


for condition in tree:

    if condition.PXDGainMapPar_valid:
        for sensorID in sensor_list:

            layer = sensorID.getLayerNumber()
            ladder = sensorID.getLadderNumber()
            sensor = sensorID.getSensorNumber()

            nBinsU = condition.PXDGainMapPar.getCorrectionsU()
            nBinsV = condition.PXDGainMapPar.getCorrectionsV()

            name = "Gains_{:d}_{:d}_{:d}_run_{:d}".format(layer, ladder, sensor, condition.run)
            title = "Relative energy calibration Sensor={:d}.{:d}.{:d} run={:d}".format(layer, ladder, sensor, condition.run)
            gain_map = ROOT.TH2F(name, title, nBinsU, 0, nBinsU, nBinsV, 0, nBinsV)
            gain_map.GetXaxis().SetTitle("gain uid")
            gain_map.GetYaxis().SetTitle("gain vid")
            gain_map.GetZaxis().SetTitle("rel. gain factor")
            gain_map.SetStats(0)

            for guID in range(nBinsU):
                for gvID in range(nBinsV):
                    gain = condition.PXDGainMapPar.getGainCorrection(sensorID.getID(), guID, gvID)
                    gain_map.SetBinContent(int(guID + 1), int(gvID + 1), gain)

            gain_map.Write()

rfile.Write()
rfile.Close()
