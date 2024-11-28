#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <input>../TreeFitted_B0ToJPsiKs.root</input>
  <contact>Frank Meier; frank.meier@duke.edu</contact>
  <interval>nightly</interval>
</header>
"""
# Bto J/Psi Ks(Pi+Pi-) is fitted and the resolution of the vertex
# positions/E/P/pVal is plotted.

import glob

import ROOT
import sysconfig
ROOT.gROOT.ProcessLine(".include " + sysconfig.get_path("include"))  # noqa

workingFiles = glob.glob("../TreeFitted_B0ToJPsiKs.root")

chain = ROOT.TChain("B0TreeFit")
for iFile in workingFiles:
    chain.AddFile(iFile)

outputFile = ROOT.TFile("test7_TreeFitterOutput.root", "RECREATE")

RangeMicrons = 100
RangePull = 5
nBins = 50

B0_E_pull_s = ROOT.TH1F("B0_E_pull_s", 'Pull of the B^{0} energy sig', nBins, -RangePull, RangePull)
B0_p_pull_s = ROOT.TH1F("B0_p_pull_s", 'Pull of the B^{0} momentum sig', nBins, -RangePull, RangePull)

B0_vertex_pullx_s = ROOT.TH1F("B0_vertex_pullx_s", 'Pull of The B^{0} x vertex position sig', nBins, -RangePull, RangePull)
B0_vertex_pully_s = ROOT.TH1F("B0_vertex_pully_s", 'Pull of The B^{0} y vertex position sig', nBins, -RangePull, RangePull)
B0_vertex_pullz_s = ROOT.TH1F("B0_vertex_pullz_s", 'Pull of The B^{0} z vertex position sig', nBins, -RangePull, RangePull)

B0_vertex_resolutionx_s = ROOT.TH1F("B0_vertex_resolutionx_s",
                                    'meas - mc of the B^{0} x vertex sig', nBins, -RangeMicrons, RangeMicrons)
B0_vertex_resolutiony_s = ROOT.TH1F("B0_vertex_resolutiony_s",
                                    'meas - mc of the B^{0} y vertex sig', nBins, -RangeMicrons, RangeMicrons)
B0_vertex_resolutionz_s = ROOT.TH1F("B0_vertex_resolutionz_s",
                                    'meas - mc of the B^{0} z vertex sig', nBins, -RangeMicrons, RangeMicrons)
pVal_s = ROOT.TH1F("pVal_s", 'global pValue sig', nBins, -1, 1)
pVal_b = ROOT.TH1F("pVal_b", 'global pValue bkg', nBins, -1, 1)

signalCut = ROOT.TCut("(isSignal>0)")
bkgCut = ROOT.TCut("(isSignal<1)")

histOperationStrings = [
    "(E - mcE) / E_uncertainty>>B0_E_pull_s",
    "(p - mcP) / pErr>>B0_p_pull_s",
    "(x - mcDecayVertexX) / x_uncertainty>>B0_vertex_pullx_s",
    "(y - mcDecayVertexY) / y_uncertainty>>B0_vertex_pully_s",
    "(z - mcDecayVertexZ) / z_uncertainty>>B0_vertex_pullz_s",
    "(x - mcDecayVertexX)*10000>>B0_vertex_resolutionx_s",
    "(y - mcDecayVertexY)*10000>>B0_vertex_resolutiony_s",
    "(z - mcDecayVertexZ)*10000>>B0_vertex_resolutionz_s",
    "chiProb>>pVal_s",
    "chiProb>>pVal_b",
]

drawToNewHist = chain.Draw
for histOperation in histOperationStrings:
    if "_s" in histOperation:
        drawToNewHist(histOperation, signalCut)
    else:
        drawToNewHist(histOperation, bkgCut)

histsAndLabels = {
    B0_E_pull_s: "B^{0} E pull",
    B0_p_pull_s: "B^{0} momentum pull",
    B0_vertex_pullx_s: "pull of B^{0} vertex X ",
    B0_vertex_pully_s: "pull of B^{0} vertex Y ",
    B0_vertex_pullz_s: "pull of B^{0} vertex Z ",
    B0_vertex_resolutionx_s: "meas-mc of B^{0} vertex X [#mum]",
    B0_vertex_resolutiony_s: "meas-mc of B^{0} vertex Y [#mum]",
    B0_vertex_resolutionz_s: "meas-mc of B^{0} vertex Z [#mum]",
    pVal_b: "pValue",
    pVal_s: "pValue",
}

checkForGausian = "This should be a Gaussian on flat bkg and not changing."
metaOptions = "pvalue-warn=0.1"
for hist, xlabel in histsAndLabels.items():
    hist.GetXaxis().SetTitle(xlabel)
    hist.GetListOfFunctions().Add(ROOT.TNamed("Contact", "frank.meier@duke.edu"))
    hist.GetListOfFunctions().Add(ROOT.TNamed("Description", xlabel))
    hist.GetListOfFunctions().Add(ROOT.TNamed("MetaOptions", metaOptions))
    if any(string in xlabel for string in ["pull", "meas-mc"]):
        hist.GetListOfFunctions().Add(ROOT.TNamed("Check", checkForGausian))

outputFile.Write()
