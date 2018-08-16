#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Creates overview plots for gain calibrations
#
# At first, you can extract the gain calibration payloads from a localdb/centraldb using the tool
#
# b2conditionsdb-extract --exp 3 --runs 126-6522 --tag Calibration_Offline_Development --output gain_payloads.root  PXDGainMapPar
#
# Secondly, execute the script as
#
# basf2 gain_plot.py
#
# author: benjamin.schwenker@phys.uni-goettingen.de

import os
import sys
from basf2 import *
import ROOT
from ROOT import Belle2
import numpy as np
from array import array

sensor_list = [Belle2.VxdID("1.1.1"), Belle2.VxdID("1.1.2"), Belle2.VxdID("2.1.1"), Belle2.VxdID("2.1.2")]

rfile = ROOT.TFile("gain_payloads.root", "UPDATE")
conditions = rfile.Get("conditions")

gain_table = dict()
for sensorID in sensor_list:
    gain_table[sensorID.getID()] = list()
run_list = list()

for condition in conditions:
    if condition.PXDGainMapPar_valid:
        run_list.append(condition.run)
        for sensorID in sensor_list:

            nBinsU = condition.PXDGainMapPar.getBinsU()
            nBinsV = condition.PXDGainMapPar.getBinsV()

            layer = sensorID.getLayerNumber()
            ladder = sensorID.getLadderNumber()
            sensor = sensorID.getSensorNumber()

            name = "Gains_{:d}_{:d}_{:d}_run_{:d}".format(layer, ladder, sensor, condition.run)
            title = "Relative energy calibration Sensor={:d}.{:d}.{:d} run={:d}".format(layer, ladder, sensor, condition.run)
            gain_map = ROOT.TH2F(name, title, nBinsU, 0, nBinsU, nBinsV, 0, nBinsV)
            gain_map.GetXaxis().SetTitle("uBin")
            gain_map.GetYaxis().SetTitle("vBin")
            gain_map.GetZaxis().SetTitle("rel. gain factor")
            gain_map.SetStats(0)

            gain_list = []
            for guID in range(nBinsU):
                for gvID in range(nBinsV):
                    gain = condition.PXDGainMapPar.getContent(sensorID.getID(), guID, gvID)
                    gain_list.append(gain)
                    gain_map.SetBinContent(int(guID + 1), int(gvID + 1), gain)

            mean_gain = np.mean(np.asarray(gain_list))
            gain_table[sensorID.getID()].append(mean_gain)

            if False:
                gain_map.Write()

c = ROOT.TCanvas('gain_vs_runno', 'Gain evolution vs. run number', 200, 10, 700, 500)
c.SetGrid()

for sensorID in sensor_list:

    n = len(run_list)
    x, y = array('d'), array('d')
    for value in gain_table[sensorID.getID()]:
        y.append(value)
    for run in run_list:
        x.append(run)

    gr = ROOT.TGraph(n, x, y)
    gr.SetLineColor(ROOT.kBlue)
    gr.SetLineWidth(4)
    gr.SetName("graph_{}".format(sensorID.getID()))
    gr.SetMarkerColor(ROOT.kBlue)
    gr.SetMarkerStyle(21)
    gr.SetTitle('Gain evolution Sensor={}'.format(sensorID))
    gr.GetXaxis().SetTitle('run number')
    gr.GetYaxis().SetTitle('gain')
    gr.Draw('AP')

    c.Update()
    c.Modified()
    c.Print('gains_vs_runno_{}.png'.format(sensorID.getID()))
    c.Write()


rfile.Write()
rfile.Close()
