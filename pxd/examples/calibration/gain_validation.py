#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file to create an localdb containing PXDGainMapPar payloads
# in a localdb
#
# Execute as: basf2 gain_validatoin.py
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de

import os
import sys
from basf2 import *
import ROOT
from ROOT import Belle2

sensor_list = [Belle2.VxdID("1.1.1"), Belle2.VxdID("1.1.2"), Belle2.VxdID("2.1.1"), Belle2.VxdID("2.1.2")]

rfile = ROOT.TFile("GainPlots.root", "UPDATE")

for sensorID in sensor_list:
    for iDCD in range(4):
        for iSWB in range(6):

            layer = sensorID.getLayerNumber()
            ladder = sensorID.getLadderNumber()
            sensor = sensorID.getSensorNumber()

            mcHisto = rfile.Get("signal_mc_sensor_{}_{}_{}_dcd_{}_swb_{}".format(layer, ladder, sensor, iDCD, iSWB))
            if not mcHisto:
                continue

            dataHisto = rfile.Get("signal_data_sensor_{}_{}_{}_dcd_{}_swb_{}".format(layer, ladder, sensor, iDCD, iSWB))
            if not dataHisto:
                continue

            canvas = ROOT.TCanvas("canvas_{}_{}_{}_dcd_{}_swb_{}".format(layer, ladder, sensor, iDCD, iSWB))

            dataHisto.SetXTitle("cluster charge / ADU")
            dataHisto.SetLineColor(ROOT.kBlack)
            dataHisto.SetYTitle("number of clusters")

            mcHisto.SetXTitle("cluster charge / ADU")
            mcHisto.SetLineColor(ROOT.kBlue)
            mcHisto.SetYTitle("number of clusters")

            mcHisto.DrawNormalized()
            dataHisto.DrawNormalized("same")
            canvas.Update()
            canvas.Write()

rfile.Write()
rfile.Close()
