#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Creates overview plots for hotpixel calibrations
#
# At first, you can extract the hotpixel calibration payloads from a localdb/centraldb using the tool
#
# b2conditionsdb-extract --exp 3 --runs 126-6522 --tag Calibration_Offline_Development
#                        --output masked_pixel_payloads.root  PXDMaskedPixelPar
#
# Secondly, execute the script as
#
# basf2 hotpixel_plot.py
#
# author: benjamin.schwenker@phys.uni-goettingen.de

import os
import sys
from basf2 import *
import ROOT
from ROOT import Belle2
from array import array

sensor_list = [Belle2.VxdID("1.1.1"), Belle2.VxdID("1.1.2"), Belle2.VxdID("2.1.1"), Belle2.VxdID("2.1.2")]

rfile = ROOT.TFile("masked_pixel_payloads.root", "UPDATE")
conditions = rfile.Get("conditions")

hotpixel_table = dict()
for sensorID in sensor_list:
    hotpixel_table[sensorID.getID()] = list()
run_list = list()

for condition in conditions:
    if condition.PXDMaskedPixelPar_valid:
        print("Starting on run {}".format(condition.run))
        run_list.append(condition.run)

        for sensorID in sensor_list:

            nUCells = 250
            nVCells = 768

            layer = sensorID.getLayerNumber()
            ladder = sensorID.getLadderNumber()
            sensor = sensorID.getSensorNumber()

            name = "MaskedPixels_{:d}_{:d}_{:d}_run_{:d}".format(layer, ladder, sensor, condition.run)
            title = "Masked Pixels Sensor={:d}.{:d}.{:d} run={:d}".format(layer, ladder, sensor, condition.run)
            hot_map = ROOT.TH2F(name, title, nUCells, 0, nUCells, nVCells, 0, nVCells)
            hot_map.GetXaxis().SetTitle("uCell")
            hot_map.GetYaxis().SetTitle("vCell")
            hot_map.GetZaxis().SetTitle("isMasked")
            hot_map.SetStats(0)

            counter = 0.0

            for uCell in range(nUCells):
                for vCell in range(nVCells):
                    pixID = uCell * nVCells + vCell
                    isMasked = not condition.PXDMaskedPixelPar.pixelOK(sensorID.getID(), pixID)
                    if isMasked:
                        counter += 1.0
                    hot_map.SetBinContent(int(uCell + 1), int(vCell + 1), isMasked)

            hotfraction = counter / (nUCells * nVCells)
            hotpixel_table[sensorID.getID()].append(hotfraction)

            if False:
                hot_map.Write()

c = ROOT.TCanvas('hotpixels_vs_runno', 'Hotpixel evolution vs. run number', 200, 10, 700, 500)
c.SetGrid()

for sensorID in sensor_list:

    n = len(run_list)
    x, y = array('d'), array('d')
    for value in hotpixel_table[sensorID.getID()]:
        y.append(value)
    for run in run_list:
        x.append(run)

    gr = ROOT.TGraph(n, x, y)
    gr.SetLineColor(ROOT.kBlue)
    gr.SetLineWidth(4)
    gr.SetName("graph_{}".format(sensorID.getID()))
    gr.SetMarkerColor(ROOT.kBlue)
    gr.SetMarkerStyle(21)
    gr.SetTitle('Hotpixel evolution Sensor={}'.format(sensorID))
    gr.GetXaxis().SetTitle('run number')
    gr.GetYaxis().SetTitle('hotpixel fraction')
    gr.GetYaxis().SetRangeUser(0.0, 1.0)
    gr.Draw('AP')

    c.Update()
    c.Modified()
    c.Print('hotpixel_vs_runno_{}.png'.format(sensorID.getID()))
    c.Write()


rfile.Write()
rfile.Close()
