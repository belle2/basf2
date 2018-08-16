#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Creates overview plots for occupancy calibrations
#
# At first, you can extract the occupancy calibration payloads from a localdb/centraldb using the tool
#
# b2conditionsdb-extract --exp 3 --runs 126-6522 --tag Calibration_Offline_Development
#                        --output occupancyinfo_payloads.root  PXDOccupancyInfoPar
#
# Secondly, execute the script as
#
# basf2 occupancy_plot.py
#
# author: benjamin.schwenker@phys.uni-goettingen.de

import os
import sys
from basf2 import *
import ROOT
from ROOT import Belle2
from array import array

sensor_list = [Belle2.VxdID("1.1.1"), Belle2.VxdID("1.1.2"), Belle2.VxdID("2.1.1"), Belle2.VxdID("2.1.2")]

rfile = ROOT.TFile("occupancyinfo_payloads.root", "UPDATE")
conditions = rfile.Get("conditions")

occupancy_table = dict()
for sensorID in sensor_list:
    occupancy_table[sensorID.getID()] = list()
run_list = list()

for condition in conditions:
    if condition.PXDOccupancyInfoPar_valid:
        run_list.append(condition.run)
        for sensorID in sensor_list:
            occupancy = condition.PXDOccupancyInfoPar.getOccupancy(sensorID.getID())
            occupancy_table[sensorID.getID()].append(occupancy)


c = ROOT.TCanvas('occupancy_vs_runno', 'Occupancy evolution vs. run number', 200, 10, 700, 500)
c.SetGrid()

for sensorID in sensor_list:

    n = len(run_list)
    max_occupancy = 0.0
    x, y = array('d'), array('d')
    for value in occupancy_table[sensorID.getID()]:
        y.append(value)
        if value > max_occupancy:
            max_occupancy = value
    for run in run_list:
        x.append(run)

    gr = ROOT.TGraph(n, x, y)
    gr.SetLineColor(ROOT.kBlue)
    gr.SetLineWidth(4)
    gr.SetName("graph_{}".format(sensorID.getID()))
    gr.SetMarkerColor(ROOT.kBlue)
    gr.SetMarkerStyle(21)
    gr.SetTitle('Occupancy evolution Sensor={}'.format(sensorID))
    gr.GetXaxis().SetTitle('run number')
    gr.GetYaxis().SetTitle('occupancy')
    gr.GetYaxis().SetRangeUser(0.0, 1.1 * max_occupancy)
    gr.Draw('AP')

    c.Update()
    c.Modified()
    c.Print('occupancy_vs_runno_{}.png'.format(sensorID.getID()))
    c.Write()


rfile.Write()
rfile.Close()
