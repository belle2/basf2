#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Plot content of dbstore_CDCAlignment...root DB payload file
"""
import sys

import ROOT
from ROOT import Belle2

ROOT.gROOT.SetBatch(1)

if len(sys.argv) < 2:
    sys.exit("No input .root file specified!")

inputroot = str(sys.argv[1])
file = ROOT.TFile(inputroot, "OPEN")
cdc = file.Get("CDCAlignment")

wires_in_layer = [
    160, 160, 160, 160, 160, 160, 160, 160,
    160, 160, 160, 160, 160, 160,
    192, 192, 192, 192, 192, 192,
    224, 224, 224, 224, 224, 224,
    256, 256, 256, 256, 256, 256,
    288, 288, 288, 288, 288, 288,
    320, 320, 320, 320, 320, 320,
    352, 352, 352, 352, 352, 352,
    384, 384, 384, 384, 384, 384]

n_wires = sum(wires_in_layer)

# CDC - Alignment of layers -----------------------------------------------------

params = [1, 2, 6, 11, 12, 16]
param_names = ['X', 'Y', 'Phi', 'dX', 'dY', 'dPhi']
param_scales = [0.02, 0.02, 0.001, 0.02, 0.02, 0.001]

histosL = []
for i in range(0, len(params)):
    histosL.append(ROOT.TGraph(56))
    histosL[-1].SetName("g{}".format(i))
    histosL[-1].SetTitle(param_names[i])

for ipar in range(0, len(params)):
    for layer in range(0, 56):
        param = params[ipar]
        val = cdc.getGlobalParam(Belle2.WireID(layer, 511).getEWire(), param)
        histosL[ipar].SetPoint(layer, layer, val)

cL = ROOT.TCanvas("cL", "CDCAlignment - Layers", 1600, 1200)
cL.Divide(3, 2)

for i, h in enumerate(histosL):
    cL.cd(i+1)
    h.SetMarkerStyle(34)
    if params[i] in [1, 2, 11, 12]:
        h.GetHistogram().SetMaximum(0.02)
        h.GetHistogram().SetMinimum(-0.02)
    else:
        h.GetHistogram().SetMaximum(0.0005)
        h.GetHistogram().SetMinimum(-0.0005)
    h.Draw("AP")

cL.SaveAs(inputroot + ".png")

# CDC - Alignment of wires -----------------------------------------------------
params = [0, 1, 2, 4, 5, 6, 21]
param_names = ['bX', 'bY', 'bZ', 'fX', 'fY', 'fZ', 'sagging']
scales = [0.03, 0.03, 0.3, 0.03, 0.03, 0.3, 200]
histosW = [ROOT.TH2F("histo_wire_{}".format(ipar), "histo_{}".format(param_names[ipar]), n_wires, 0,
                     n_wires, 2000, -scales[ipar], scales[ipar]) for ipar in range(0, len(params))]

for ipar in range(0, len(params)):
    wireindex = 0
    for layer in range(0, 56):
        for wire in range(0, wires_in_layer[layer]):
            param = params[ipar]
            val = cdc.getGlobalParam(Belle2.WireID(layer, wire).getEWire(), param)
            histosW[ipar].Fill(wireindex, val)
            wireindex += 1

cW = ROOT.TCanvas("cW", "CDCAlignment - Wires", 1600, 1200)
cW.Divide(3, 3)

for i, h in enumerate(histosW):
    cW.cd(i+1)
    h.Draw("prof")

cW.SaveAs(inputroot + "_wires.png")
