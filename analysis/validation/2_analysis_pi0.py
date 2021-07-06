#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <input>../GenericB_GENSIMRECtoDST.dst.root</input>
  <output>Pi0_Validation.root</output>
  <contact>Mario Merola (mario.merola@na.infn.it), Andrea Selce (selce@infn.it)</contact>
  <description>
  Check the calibration of the ECL in the MC by determining the measured pi0 invariant mass.
  </description>
</header>
"""

import basf2
import ROOT
from modularAnalysis import cutAndCopyList, inputMdst, matchMCTruth
from stdPi0s import stdPi0s
from validation_tools.metadata import create_validation_histograms
from validation_tools.metadata import validation_metadata_update
from variables import variables as vm

INPUT_FILENAME = "../GenericB_GENSIMRECtoDST.dst.root"
OUTPUT_FILENAME = "Pi0_Validation.root"

main = basf2.Path()
inputMdst('default', INPUT_FILENAME, path=main)

stdPi0s('all', path=main)
matchMCTruth('pi0:all', path=main)

cutAndCopyList('pi0:rec', 'pi0:all', 'daughter(0, E)>0.05 and daughter(1, E)>0.05', path=main)
cutAndCopyList('pi0:mc', 'pi0:all', 'mcErrors<1', path=main)

vm.addAlias('Mreco', 'M')


create_validation_histograms(
    main, OUTPUT_FILENAME, "pi0:rec",
    variables_1d=[
        (
            "Mreco", 40, 0.08, 0.18,
            "#pi^{0} reconstructed candidates, invariant mass",
            "Mario Merola <mario.merola@desy.de>; Andrea Selce <selce@infn.it>",
            r"The $pi^{0}$ invariant mass distribution with $E_{\gamma}>0.05\, \text{GeV}$",
            r"Distribution should be peaking at the nominal $\pi^{0}$ mass.",
            "M(#pi^{0}) [GeV/c^{2}]", "Candidates", "shifter"
        ),
    ],
    description=r"$\pi^0$ reconstructed mass distribution",
)


vm.addAlias('Mmc', 'M')

create_validation_histograms(
    main, OUTPUT_FILENAME, "pi0:mc",
    variables_1d=[
        (
            "Mmc", 40, 0.08, 0.18,
            "#pi^{0} MC candidates, invariant mass",
            "Mario Merola <mario.merola@desy.de>; Andrea Selce <selce@infn.it>",
            r"The $pi^{0}$ invariant mass distribution for truth matched candidates",
            r"Distribution should be peaking at the nominal $\pi^{0}$ mass.",
            "M(#pi^{0}) [GeV/c^{2}]", "Candidates", "shifter"
        ),
    ],
    description=r"$\pi^0$ MC mass distribution",
)

basf2.process(main)
print(basf2.statistics)


f = ROOT.TFile(OUTPUT_FILENAME)
Mrecohist = f.Get('Mreco')
Mmchist = f.Get('Mmc')


mass = ROOT.RooRealVar("recomass", "m_{#gamma#gamma} [GeV/c^{2}]", 0.11, 0.15)

h_pi0_reco = ROOT.RooDataHist("h_pi0_reco", "h_pi0_reco", ROOT.RooArgList(mass), Mrecohist)
h_pi0_mc = ROOT.RooDataHist("h_pi0_mc", "h_pi0_mc", ROOT.RooArgList(mass), Mmchist)


# pi0 signal PDF is a Crystal Ball (Gaussian also listed in case we want to switch)
mean = ROOT.RooRealVar("mean", "mean", 0.14, 0.11, 0.15)
sig1 = ROOT.RooRealVar("#sigma", "sig", 0.05, 0.002, 0.1)
gau1 = ROOT.RooGaussian("gau1", "gau1", mass, mean, sig1)

alphacb = ROOT.RooRealVar("alphacb", "alpha", 1.5, 0.1, 1.9)
ncb = ROOT.RooRealVar("ncb", "n", 8)  # ,2.,15)
sigcb = ROOT.RooCBShape("sigcb", "sigcb", mass, mean, sig1, alphacb, ncb)

# pi0 background PDF is a 2nd order Chebyshev
b1 = ROOT.RooRealVar("b1", "b1", -3.0021e-01, -0.8, 0.8)
a1 = ROOT.RooRealVar("a1", "a1", -3.0021e-01, -0.8, 0.8)
bList = ROOT.RooArgList(a1, b1)
bkg = ROOT.RooChebychev("bkg", "bkg", mass, bList)


nsig = ROOT.RooRealVar("nsig", "nsig", 1000, 0, 1000000)
nbkg = ROOT.RooRealVar("nbkg", "nbkg", 1000, 0, 1000000)


totalPdf = ROOT.RooAddPdf("totalpdf", "", ROOT.RooArgList(sigcb, bkg), ROOT.RooArgList(nsig, nbkg))


output = ROOT.TFile("Pi0_Validation_ntuple.root", "recreate")

# Store pi0 mass fit results to a tuple for comparison of mean and width among releases.
outputNtuple = ROOT.TNtuple(
    "pi0_mass",
    "Pi0 mass fit results",
    "mean:meanerror:width:widtherror")


canvas = ROOT.TCanvas("canvas", "pi0 mass fit", 1000, 600)
canvas.Divide(2, 1)
canvas.cd(1)

# Fit to the reco mass
totalPdf.fitTo(h_pi0_reco, ROOT.RooFit.Extended(True), ROOT.RooFit.Minos(1), ROOT.RooFit.Range(0.11, 0.15))
frame1 = ROOT.RooPlot
frame1 = mass.frame()
h_pi0_reco.plotOn(frame1, ROOT.RooFit.Name("Hist"))
frame1.SetMaximum(frame1.GetMaximum())
totalPdf.plotOn(frame1, ROOT.RooFit.Name("curve"))
totalPdf.plotOn(frame1, ROOT.RooFit.Components("sigcb"), ROOT.RooFit.LineStyle(ROOT.kDashed), ROOT.RooFit.LineColor(ROOT.kRed))
totalPdf.plotOn(frame1, ROOT.RooFit.Components("bkg"), ROOT.RooFit.LineStyle(3), ROOT.RooFit.LineColor(ROOT.kBlue))
# totalPdf.paramOn(frame1,ROOT.RooFit.Parameters(ROOT.RooArgSet(sig1,mean,nsig)),ROOT.RooFit.Format("NELU",ROOT.RooFit.AutoPrecision(2)),ROOT.RooFit.Layout(0.5,0.9,0.9))
# frame1.getAttText().SetTextSize(0.5)
frame1.SetMaximum(Mrecohist.GetMaximum() * 1.5)
frame1.GetXaxis().SetTitleOffset(1.4)
frame1.GetYaxis().SetTitleOffset(1.5)
meanval = mean.getVal()
meanerror = mean.getError()
width = sig1.getVal()
widtherror = sig1.getError()
frame1.Draw("")

outputNtuple.Fill(meanval, meanerror, width, widtherror)


canvas.cd(2)

# ---------------------------
# Fit to the truth matched mass using the same pdf of the reco mass.
# Re-initialize the fit parameters to the default values.
mean.setVal(0.14)
sig1.setVal(0.005)
nsig.setVal(1000)
nbkg.setVal(1000)
alphacb.setVal(1.5)
ncb.setVal(8.)
b1.setRange(-0.5, 0.5)
a1.setRange(-0.5, 0.5)
b1.setVal(-3.0021e-01)
a1.setVal(-3.0021e-01)


# Fit to the truth matched mass
totalPdf.fitTo(h_pi0_mc, ROOT.RooFit.Extended(True), ROOT.RooFit.Minos(1), ROOT.RooFit.Range(0.11, 0.15))
frame2 = ROOT.RooPlot
frame2 = mass.frame()
h_pi0_mc.plotOn(frame2, ROOT.RooFit.Name("Hist"))
frame2.SetMaximum(frame2.GetMaximum())
totalPdf.plotOn(frame2, ROOT.RooFit.Name("curve"))
totalPdf.plotOn(frame2, ROOT.RooFit.Components("sigcb"), ROOT.RooFit.LineStyle(ROOT.kDashed), ROOT.RooFit.LineColor(ROOT.kRed))
totalPdf.plotOn(frame2, ROOT.RooFit.Components("bkg"), ROOT.RooFit.LineStyle(3), ROOT.RooFit.LineColor(ROOT.kBlue))
# totalPdf.paramOn(frame2,ROOT.RooFit.Parameters(ROOT.RooArgSet(sig1,mean,nsig)),ROOT.RooFit.Format("NELU",ROOT.RooFit.AutoPrecision(2)),ROOT.RooFit.Layout(0.5,0.9,0.9))
# frame2.getAttText().SetTextSize(0.5)
frame2.SetMaximum(Mmchist.GetMaximum() * 1.5)
frame2.GetXaxis().SetTitleOffset(1.4)
frame2.GetYaxis().SetTitleOffset(1.5)
meanval_mc = mean.getVal()
meanerror_mc = mean.getError()
width_mc = sig1.getVal()
widtherror_mc = sig1.getError()
frame2.Draw("")

canvas.Write()

outputNtuple.Fill(meanval_mc, meanerror_mc, width_mc, widtherror_mc)

outputNtuple.Write()

validation_metadata_update(
    output,
    "pi0_mass",
    title="Pi0 mass fit results",
    contact="mario.merola@na.infn.it, selce@infn.it",
    description="Fit to the invariant mass of the reconstructed and truth matched pi0s",
    check="Consistent numerical fit results. Stable mean and width.",
    metaoptions="shifter")


output.Close()
