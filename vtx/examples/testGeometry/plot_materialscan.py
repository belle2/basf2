#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

######################################################
# This script plots the polar material scan produced
# with MaterialScan.py
#
# Usage:
# python3 plot_materialscan.py --input=MaterialScan_VTX-CMOS-5layer-Discs.root
#
######################################################


import ROOT
from ROOT import gPad
import os

import argparse
ap = argparse.ArgumentParser()
ap.add_argument("--input", default="VTX-CMOS-5layer-Discs.root", help="VTX geometry variant from xml")
args = vars(ap.parse_args())


# Input files
vtx_file = args['input']

# Name of VTX variant
vtx_name = os.path.splitext(vtx_file)[0]
vtx_name = vtx_name.split('_')[-1]

# Output root file file
output = os.path.splitext(vtx_file)[0]+'_profile.root'


def add_vtxhistos(filename, hfile):
    rfile = ROOT.TFile(filename, 'READ')

    th2_pipe = rfile.Get("Spherical/BeamPipe_x0")
    th2_vtx = rfile.Get("Spherical/{:s}_x0".format(vtx_name))

    hfile.cd()
    hpipe = ROOT.TH1D("hpipe_vtx", "BeamPipe", 100, 17, 150)
    hvtx = ROOT.TH1D("hvtx_vtx", vtx_name, 100, 17, 150)

    for bin in range(1, th2_pipe.GetNbinsX()+1):
        hpipe.SetBinContent(bin, th2_pipe.GetBinContent(bin, 1))
        hvtx.SetBinContent(bin, th2_vtx.GetBinContent(bin, 1))

    hfile.Write()
    rfile.Close()

    hpipe.SetFillColor(ROOT.kRed)
    hpipe.SetMarkerStyle(21)
    hpipe.SetMarkerColor(ROOT.kRed)
    hpipe.SetLineColor(ROOT.kRed)
    hvtx.SetFillColor(ROOT.kGreen)
    hvtx.SetMarkerStyle(21)
    hvtx.SetMarkerColor(ROOT.kGreen)

    return hpipe, hvtx


# Histogram file
hfile = ROOT.TFile(output, 'RECREATE')


cst2 = ROOT.TCanvas("cst2", "stacked hists", 10, 10, 900, 700)
hs_vtx = ROOT.THStack("hs_vtx", "")
hpipe, hvtx = add_vtxhistos(vtx_file, hfile)
hs_vtx.Add(hpipe)
hs_vtx.Add(hvtx)
hs_vtx.Draw()
hs_vtx.GetXaxis().SetTitle("#theta [deg]")
hs_vtx.GetYaxis().SetTitle("X/X_{0}")
gPad.SetGrid()
gPad.Modified()
gPad.Update()

leg2 = ROOT.TLegend(0.68, 0.72, 0.98, 0.92)
leg2.AddEntry(hpipe, "BeamPipe", "f")
leg2.AddEntry(hvtx, vtx_name, "f")
leg2.Draw()
gPad.Modified()
gPad.Update()
cst2.Write()
