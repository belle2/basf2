#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <input>TreeFitted_B0ToJPsiKs.root</input>
  <output>test_TreeFitterOutput.root</output>
  <contact>Jo-Frederik Krohn; jo-frederik.krohn@desy.de</contact>
  <description> B->J/Psi Ks(Pi+Pi-) is fitted and the resolution of the vertex positions/E/P/pVal is plotted. </description>
  <interval>nightly</interval>
</header>
"""

import ROOT
import sysconfig
ROOT.gROOT.ProcessLine(".include " + sysconfig.get_path("include"))
from basf2 import *

import glob

workingFiles = glob.glob(str("../TreeFitted_B0ToJPsiKs.root"))

chain = ROOT.TChain(str("B0TreeFit"))
for iFile in workingFiles:
    chain.AddFile(iFile)

outputFile = ROOT.TFile("test7_TreeFitterOutput.root", "RECREATE")

RangeMicrons = 100
RangePull = 5
nBins = 50

B0_E_pull_s = ROOT.TH1F("B0_E_pull_s", 'Pull of the B^{0} energy sig', nBins, -RangePull, RangePull)
B0_p_pull_s = ROOT.TH1F("B0_p_pull_s", 'Pull of the B^{0} momentum sig', nBins, -RangePull, RangePull)

B0_vertex_pullx_s = ROOT.TH1F("B0_vertex_pullx_s", 'Pull of The B^{0} x vertex postion sig', nBins, -RangePull, RangePull)
B0_vertex_pully_s = ROOT.TH1F("B0_vertex_pully_s", 'Pull of The B^{0} y vertex postion sig', nBins, -RangePull, RangePull)
B0_vertex_pullz_s = ROOT.TH1F("B0_vertex_pullz_s", 'Pull of The B^{0} z vertex postion sig', nBins, -RangePull, RangePull)

B0_vertex_resolutionx_s = ROOT.TH1F("B0_vertex_resolutionx_s",
                                    'meas - mc of the B^{0} x vertex sig', nBins, -RangeMicrons, RangeMicrons)
B0_vertex_resolutiony_s = ROOT.TH1F("B0_vertex_resolutiony_s",
                                    'meas - mc of the B^{0} y vertex sig', nBins, -RangeMicrons, RangeMicrons)
B0_vertex_resolutionz_s = ROOT.TH1F("B0_vertex_resolutionz_s",
                                    'meas - mc of the B^{0} z vertex sig', nBins, -RangeMicrons, RangeMicrons)
pVal_s = ROOT.TH1F("pVal_s", 'global pValue sig', nBins, -1, 1)
pVal_b = ROOT.TH1F("pVal_b", 'global pValue bkg', nBins, -1, 1)

histsAndLabels = {
    B0_E_pull_s: "B^{0} E pull",
    B0_p_pull_s: "B^{0} momentum pull",
    B0_vertex_pullx_s: "pull of B^{0} vertex X ",
    B0_vertex_pully_s: "pull of B^{0} vertex Y ",
    B0_vertex_pullz_s: "pull of B^{0} vertex Z ",
    B0_vertex_resolutionx_s: "meas-mc of B^{0} vertex X [\mum]",
    B0_vertex_resolutiony_s: "meas-mc of B^{0} vertex Y [\mum]",
    B0_vertex_resolutionz_s: "meas-mc of B^{0} vertex Z [\mum]",
    pVal_b: "pValue",
    pVal_s: "pValue",
}

checkForGausian = "This should be a Gausian on flat bkg and not changing."
metaOptions = "pvalue-warn=0.1"
for hist, xlabel in histsAndLabels.items():
    hist.GetXaxis().SetTitle(xlabel)
    hist.GetListOfFunctions().Add(ROOT.TNamed("Contact", "jkrohn@student.unimelb.edu.au"))
    hist.GetListOfFunctions().Add(ROOT.TNamed("Description", xlabel))
    hist.GetListOfFunctions().Add(ROOT.TNamed("MetaOptions", metaOptions))
    if any(string in xlabel for string in ["pull", "meas-mc"]):
        hist.GetListOfFunctions().Add(ROOT.TNamed("Check", checkForGausian))

signalCut = ROOT.TCut("(isSignal>0)")
bkgCut = ROOT.TCut("(isSignal<1)")

histOperationStrings = [
    "(E - mcE) / E_uncertainty>>B0_E_pull_s",
    "(p - mcP) / pErr>>B0_p_pull_s",
    "(x - mcDX) / x_uncertainty>>B0_vertex_pullx_s",
    "(y - mcDY) / y_uncertainty>>B0_vertex_pully_s",
    "(z - mcDZ) / z_uncertainty>>B0_vertex_pullz_s",
    "(x - mcDX)*10000>>B0_vertex_resolutionx_s",
    "(y - mcDY)*10000>>B0_vertex_resolutiony_s",
    "(z - mcDZ)*10000>>B0_vertex_resolutionz_s",
    "chiProb>>pVal_s",
    "chiProb>>pVal_b",
]

drawToNewHist = chain.Draw
for histOperation in histOperationStrings:
    if "_s" in histOperation:
        drawToNewHist(histOperation, signalCut)
    else:
        drawToNewHist(histOperation, bkgCut)

outputFile.Write()
