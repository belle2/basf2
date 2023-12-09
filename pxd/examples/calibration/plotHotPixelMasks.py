#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Creates overview plots for hotpixel calibrations
#
# At first, you can extract the hotpixel calibration payloads from a localdb/centraldb using the tool
#
# b2conditionsdb-extract --exp 3 --runs 0-5614 --tag Calibration_Offline_Development --output hot_payloads.root PXDMaskedPixelPar
#
# Secondly, execute the script as
#
# basf2 plotHotPixelMasks.py
#
# basf2 plotHotPixelMasks.py -- --maps


import basf2 as b2
import ROOT
from ROOT import Belle2
from array import array

import argparse
parser = argparse.ArgumentParser(description="Plot hotpixel maps")
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
histofile = ROOT.TFile('hotpixel_histos.root', 'RECREATE')
histofile.cd()
histofile.mkdir("maps")

# Open file with extracted payloads
rfile = ROOT.TFile("hot_payloads.root", "READ")
b2.conditions = rfile.Get("conditions")

hotpixel_table = dict()
for sensorID in sensor_list:
    hotpixel_table[sensorID.getID()] = list()
run_list = list()

for condition in b2.conditions:
    if condition.PXDMaskedPixelPar_valid:
        print(f"Starting on run {condition.run}")
        run_list.append(condition.run)

        for sensorID in sensor_list:

            nUCells = 250
            nVCells = 768

            hotpixelmap = condition.PXDMaskedPixelPar.getMaskedPixelMap()

            layer = sensorID.getLayerNumber()
            ladder = sensorID.getLadderNumber()
            sensor = sensorID.getSensorNumber()

            counter = len(hotpixelmap[sensorID.getID()])

            if args.maps:
                name = f"MaskedPixels_{layer:d}_{ladder:d}_{sensor:d}_run_{condition.run:d}"
                title = f"Masked Pixels Sensor={layer:d}.{ladder:d}.{sensor:d} run={condition.run:d}"
                hot_map = ROOT.TH2F(name, title, nUCells, 0, nUCells, nVCells, 0, nVCells)
                hot_map.GetXaxis().SetTitle("uCell")
                hot_map.GetYaxis().SetTitle("vCell")
                hot_map.GetZaxis().SetTitle("isMasked")
                hot_map.SetStats(0)

                for uCell in range(nUCells):
                    for vCell in range(nVCells):
                        pixID = uCell * nVCells + vCell
                        isMasked = not condition.PXDMaskedPixelPar.pixelOK(sensorID.getID(), pixID)
                        hot_map.SetBinContent(int(uCell + 1), int(vCell + 1), isMasked)

                histofile.cd("maps")
                hot_map.Write()

            hotfraction = counter / (nUCells * nVCells)
            hotpixel_table[sensorID.getID()].append(hotfraction)


histofile.cd()
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
    gr.SetName(f"graph_{sensorID.getID()}")
    gr.SetMarkerColor(ROOT.kBlue)
    gr.SetMarkerStyle(21)
    gr.SetTitle(f'Hotpixel evolution Sensor={sensorID}')
    gr.GetXaxis().SetTitle('run number')
    gr.GetYaxis().SetTitle('hotpixel fraction')
    gr.GetYaxis().SetRangeUser(0.0, 1.0)
    gr.Draw('AP')

    c.Update()
    c.Modified()
    c.Print(f'hotpixel_vs_runno_{sensorID.getID()}.png')
    c.Write()


rfile.Close()
histofile.Close()
