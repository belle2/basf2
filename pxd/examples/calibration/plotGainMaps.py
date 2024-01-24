#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Creates overview plots for gain calibrations
#
# At first, you can extract the gain calibration payloads from a localdb/centraldb using the tool
#
# b2conditionsdb-extract --exp 3 --runs 0-5614 --tag Calibration_Offline_Development --output gain_payloads.root  PXDGainMapPar
#
# Secondly, execute the script as
#
# basf2 plotGainMaps.py
#
# basf2 plotGainMaps.py -- --maps


import basf2 as b2
import ROOT
from ROOT import Belle2
from array import array

import argparse
parser = argparse.ArgumentParser(description="Plot gain maps")
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
histofile = ROOT.TFile('gain_histos.root', 'RECREATE')
histofile.cd()
histofile.mkdir("maps")

# Open file with extracted payloads
rfile = ROOT.TFile("gain_payloads.root", "READ")
b2.conditions = rfile.Get("conditions")

gain_table = dict()
for sensorID in sensor_list:
    gain_table[sensorID.getID()] = list()
run_list = list()

for condition in b2.conditions:
    if condition.PXDGainMapPar_valid:
        run_list.append(condition.run)
        for sensorID in sensor_list:

            nBinsU = condition.PXDGainMapPar.getBinsU()
            nBinsV = condition.PXDGainMapPar.getBinsV()

            layer = sensorID.getLayerNumber()
            ladder = sensorID.getLadderNumber()
            sensor = sensorID.getSensorNumber()

            mean_gain = 0.0
            for guID in range(nBinsU):
                for gvID in range(nBinsV):
                    mean_gain += condition.PXDGainMapPar.getContent(sensorID.getID(), guID, gvID)

            mean_gain /= (nBinsU * nBinsV)
            gain_table[sensorID.getID()].append(mean_gain)

            if args.maps:
                name = f"Gains_{layer:d}_{ladder:d}_{sensor:d}_run_{condition.run:d}"
                title = f"Relative energy calibration Sensor={layer:d}.{ladder:d}.{sensor:d} run={condition.run:d}"
                gain_map = ROOT.TH2F(name, title, nBinsU, 0, nBinsU, nBinsV, 0, nBinsV)
                gain_map.GetXaxis().SetTitle("uBin")
                gain_map.GetYaxis().SetTitle("vBin")
                gain_map.GetZaxis().SetTitle("rel. gain factor")
                gain_map.SetStats(0)

                for guID in range(nBinsU):
                    for gvID in range(nBinsV):
                        gain = condition.PXDGainMapPar.getContent(sensorID.getID(), guID, gvID)
                        gain_map.SetBinContent(int(guID + 1), int(gvID + 1), gain)

                histofile.cd("maps")
                gain_map.Write()

histofile.cd()
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
    gr.SetName(f"graph_{sensorID.getID()}")
    gr.SetMarkerColor(ROOT.kBlue)
    gr.SetMarkerStyle(21)
    gr.SetTitle(f'Gain evolution Sensor={sensorID}')
    gr.GetXaxis().SetTitle('run number')
    gr.GetYaxis().SetTitle('gain')
    gr.Draw('AP')

    c.Update()
    c.Modified()
    c.Print(f'gains_vs_runno_{sensorID.getID()}.png')
    c.Write()


rfile.Close()
histofile.Close()
