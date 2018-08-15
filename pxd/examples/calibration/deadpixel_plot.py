#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Creates overview plots for deadpixel calibrations
#
# At first, you can extract the deadpixel calibration payloads from a localdb/centraldb using the tool
#
# b2conditionsdb-extract --exp 3 --runs 126-6522 --tag Calibration_Offline_Development
#                         --output dead_pixel_payloads.root  PXDDeadPixelPar
#
# Secondly, execute the script as
#
# basf2 deadpixel_plot.py
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de

import os
import sys
from basf2 import *
import ROOT
from ROOT import Belle2
from array import array

sensor_list = [Belle2.VxdID("1.1.1"), Belle2.VxdID("1.1.2"), Belle2.VxdID("2.1.1"), Belle2.VxdID("2.1.2")]

rfile = ROOT.TFile("dead_pixel_payloads.root", "UPDATE")
conditions = rfile.Get("conditions")

deadpixel_table = dict()
for sensorID in sensor_list:
    deadpixel_table[sensorID.getID()] = list()
run_list = list()

for condition in conditions:
    if condition.PXDDeadPixelPar_valid:
        print("Starting on run {}".format(condition.run))
        run_list.append(condition.run)

        for sensorID in sensor_list:

            nUCells = 250
            nVCells = 768

            layer = sensorID.getLayerNumber()
            ladder = sensorID.getLadderNumber()
            sensor = sensorID.getSensorNumber()

            name = "DeadPixels_{:d}_{:d}_{:d}_run_{:d}".format(layer, ladder, sensor, condition.run)
            title = "Dead Pixels Sensor={:d}.{:d}.{:d} run={:d}".format(layer, ladder, sensor, condition.run)
            dead_map = ROOT.TH2F(name, title, nUCells, 0, nUCells, nVCells, 0, nVCells)
            dead_map.GetXaxis().SetTitle("uCell")
            dead_map.GetYaxis().SetTitle("vCell")
            dead_map.GetZaxis().SetTitle("isDead")
            dead_map.SetStats(0)

            counter = 0.0

            for uCell in range(nUCells):
                for vCell in range(nVCells):
                    pixID = uCell * nVCells + vCell
                    isDeadSinglePixel = condition.PXDDeadPixelPar.isDeadSinglePixel(sensorID.getID(), pixID)
                    isDeadRow = condition.PXDDeadPixelPar.isDeadRow(sensorID.getID(), vCell)
                    isDeadDrain = condition.PXDDeadPixelPar.isDeadDrain(sensorID.getID(), uCell * 4 + vCell % 4)
                    isDead = isDeadSinglePixel or isDeadRow or isDeadDrain

                    if isDead:
                        counter += 1.0
                    dead_map.SetBinContent(int(uCell + 1), int(vCell + 1), isDead)

            deadfraction = counter / (nUCells * nVCells)
            deadpixel_table[sensorID.getID()].append(deadfraction)

            if False:
                dead_map.Write()


c = ROOT.TCanvas('dead_vs_runno', 'Deadpixel evolution vs. run number', 200, 10, 700, 500)
c.SetGrid()

for sensorID in sensor_list:

    n = len(run_list)
    x, y = array('d'), array('d')
    for value in deadpixel_table[sensorID.getID()]:
        y.append(value)
    for run in run_list:
        x.append(run)

    gr = ROOT.TGraph(n, x, y)
    gr.SetLineColor(ROOT.kBlue)
    gr.SetLineWidth(4)
    gr.SetName("graph_{}".format(sensorID.getID()))
    gr.SetMarkerColor(ROOT.kBlue)
    gr.SetMarkerStyle(21)
    gr.SetTitle('Deadpixel evolution Sensor={}'.format(sensorID))
    gr.GetXaxis().SetTitle('run number')
    gr.GetYaxis().SetTitle('dead fraction')
    gr.GetYaxis().SetRangeUser(0.0, 1.0)
    gr.Draw('AP')

    c.Update()
    c.Modified()
    c.Print('deadpixels_vs_runno_{}.png'.format(sensorID.getID()))
    c.Write()


rfile.Write()
rfile.Close()
