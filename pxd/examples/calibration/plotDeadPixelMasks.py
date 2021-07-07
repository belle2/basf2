#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Creates overview plots for deadpixel calibrations
#
# At first, you can extract the deadpixel calibration payloads from a localdb/centraldb using the tool
#
# b2conditionsdb-extract --exp 3 --runs 0-5614 --tag Calibration_Offline_Development --output dead_payloads.root PXDDeadPixelPar
#
# Secondly, execute the script as
#
# basf2 plotDeadPixelMasks.py
#
# basf2 plotDeadPixelMasks.py -- --maps


import basf2 as b2
import ROOT
from ROOT import Belle2
from array import array

import argparse
parser = argparse.ArgumentParser(description="Plot dead pixel maps")
parser.add_argument('--maps', dest='maps', action="store_true", help='Create maps from payloads. This can be slow!')
args = parser.parse_args()


sensor_list = [
    Belle2.VxdID("1.1.1"), Belle2.VxdID("1.1.2"),
    Belle2.VxdID("1.2.1"), Belle2.VxdID("1.2.2"),
    Belle2.VxdID("1.3.1"), Belle2.VxdID("1.3.2"),
    Belle2.VxdID("1.4.1"), Belle2.VxdID("1.4.2"),
    Belle2.VxdID("1.5.1"), Belle2.VxdID("1.5.2"),
    Belle2.VxdID("1.6.1"), Belle2.VxdID("1.6.2"),
    Belle2.VxdID("1.7.1"), Belle2.VxdID("1.7.2"),
    Belle2.VxdID("1.8.1"), Belle2.VxdID("1.8.2"),
    Belle2.VxdID("2.4.1"), Belle2.VxdID("2.4.2"),
    Belle2.VxdID("2.5.1"), Belle2.VxdID("2.5.2")]

# Create output file wíth histos and plots
histofile = ROOT.TFile('deadpixel_histos.root', 'RECREATE')
histofile.cd()
histofile.mkdir("maps")

# Open file with extracted payloads
rfile = ROOT.TFile("dead_payloads.root", "READ")
b2.conditions = rfile.Get("conditions")

deadpixel_table = dict()
for sensorID in sensor_list:
    deadpixel_table[sensorID.getID()] = list()
run_list = list()

for condition in b2.conditions:
    if condition.PXDDeadPixelPar_valid:
        print("Starting on run {}".format(condition.run))
        run_list.append(condition.run)

        for sensorID in sensor_list:

            nUCells = 250
            nVCells = 768

            deadsensormap = condition.PXDDeadPixelPar.getDeadSensorMap()
            deaddrainmap = condition.PXDDeadPixelPar.getDeadDrainMap()
            deadrowmap = condition.PXDDeadPixelPar.getDeadRowMap()
            deadsinglesmap = condition.PXDDeadPixelPar.getDeadSinglePixelMap()

            layer = sensorID.getLayerNumber()
            ladder = sensorID.getLadderNumber()
            sensor = sensorID.getSensorNumber()

            # Every dead drain counts for 192 dead pixels
            counter = len(deaddrainmap[sensorID.getID()]) * 192

            # Every dead row counts for 250 dead pixels
            # This can lead to double counting of dead pixel from dead drains
            # The double counting can be avoided using --maps option.
            counter += len(deadrowmap[sensorID.getID()]) * 250

            # Every dead row counts for 250 dead pixels
            counter += len(deadsinglesmap[sensorID.getID()])

            if args.maps:
                counter = 0

                name = "DeadPixels_{:d}_{:d}_{:d}_run_{:d}".format(layer, ladder, sensor, condition.run)
                title = "Dead Pixels Sensor={:d}.{:d}.{:d} run={:d}".format(layer, ladder, sensor, condition.run)
                dead_map = ROOT.TH2F(name, title, nUCells, 0, nUCells, nVCells, 0, nVCells)
                dead_map.GetXaxis().SetTitle("uCell")
                dead_map.GetYaxis().SetTitle("vCell")
                dead_map.GetZaxis().SetTitle("isDead")
                dead_map.SetStats(0)

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

                histofile.cd("maps")
                dead_map.Write()

            deadfraction = counter / (nUCells * nVCells)
            deadpixel_table[sensorID.getID()].append(deadfraction)


histofile.cd()
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


rfile.Close()
histofile.Close()
