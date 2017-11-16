#!/usr/bin/env python
# -*- coding: utf-8 -*-

################################################################################
#
# This validation script performs a fit of DeltaT, DeltaTErr, Dz for B0_sig and
# Deltaz for B0_tag. The signal channel here is B0->JPsiKs.
# The DeltaT and DeltaZ distributions are fitted with 3 Gaussian functions.
# DeltaTErr is fitted with a CBShape function and two Gaussians.
#
# Usage:
#   basf2 B2JpsiKs_mu_DeltaTResValidation.py workingRootNtupleFiles treeName
#
# Contributors: L. Li Gioi, F. Abudinen (June 2017)
#
################################################################################


import ROOT
import sysconfig
ROOT.gROOT.ProcessLine(".include " + sysconfig.get_path("include"))
from basf2 import *
from flavorTagger import *
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
mpl.rcParams['text.usetex'] = True
mpl.rcParams['text.latex.unicode'] = True
import matplotlib.pyplot as plt
import pylab
import sys
import math
import random
import array
from operator import itemgetter

if len(sys.argv) != 4:
    sys.exit(
        "Must provide 3 arguments: [input_sim_file] or ['input_sim_file*'] wildcards, [treeName] " +
        "and [VXD_requirement (PXD or SVD)]")

PATH = "."

workingFile = sys.argv[1]
workingFiles = glob.glob(str(workingFile))

limDeltaT = 5
limDeltaTErr = 3.0
limZSig = 0.03
limZTag = 0.03

treeName = str(sys.argv[2])

# No PXD hit equired: PXD0. At least one PXD (SVD) hit for one of the muon tracks: PXD1 (SVD1).
# Hit required for both muon tracks: PXD2 (SVD2)"
VXDReqs = []
if str(sys.argv[3]) == "PXD":
    VXDReqs = ["PXD0", "PXD2"]
elif str(sys.argv[3]) == "SVD":
    VXDReqs = ["SVD0", "SVD2"]
else:
    B2FATAL('Not available VXD requirement " + str(sys.argv[3]) + ". Available are "PXD" and "SVD".')


fitResults = []
numberOfEntries = []

for VXDReq in VXDReqs:
    iResult = []

    tdat = ROOT.TChain(treeName)

    for iFile in workingFiles:
        tdat.AddFile(iFile)

    evt_no = ROOT.RooRealVar("evt_no", "evt_no", 0., -10., 10000000.)

    B0_DeltaT = ROOT.RooRealVar("B0_DeltaT", "B0_DeltaT", 0.)
    deltaTErr = ROOT.RooRealVar("B0_DeltaTErr", "B0_DeltaTErr", 0, limDeltaTErr, "ps")
    B0_TruthDeltaT = ROOT.RooRealVar("B0_TruthDeltaT", "B0_TruthDeltaT", 0.)

    B0_mcPDG = ROOT.RooRealVar("B0_mcPDG", "B0_mcPDG", 0., -1000., 1000.)
    B0_mcTagPDG = ROOT.RooRealVar("B0_mcTagPDG", "B0_mcTagPDG", 0., -1000., 1000.)
    B0_qrMC = ROOT.RooRealVar("B0_qrMC", "B0_qrMC", 0., -100, 100)
    B0_mcErrors = ROOT.RooRealVar("B0_mcErrors", "B0_mcErrors", 0., 0., 512.)
    B0_isSignal = ROOT.RooRealVar("B0_isSignal", "B0_isSignal", 0., 0., 512.)

    B0_TagVx = ROOT.RooRealVar("B0_TagVx", "B0_TagVx", 0., -100, 100, "cm")
    B0_TruthTagVx = ROOT.RooRealVar("B0_TruthTagVx", "B0_TruthTagVx", 0., -100, 100, "cm")
    B0_TagVy = ROOT.RooRealVar("B0_TagVy", "B0_TagVy", 0., -100, 100, "cm")
    B0_TruthTagVy = ROOT.RooRealVar("B0_TruthTagVy", "B0_TruthTagVy", 0., -100, 100, "cm")
    B0_TagVz = ROOT.RooRealVar("B0_TagVz", "B0_TagVz", 0., -100, 100, "cm")
    B0_TruthTagVz = ROOT.RooRealVar("B0_TruthTagVz", "B0_TruthTagVz", 0., -100, 100, "cm")

    B0_X = ROOT.RooRealVar("B0_X", "B0_X", 0., -100, 100, "cm")
    B0_TruthX = ROOT.RooRealVar("B0_TruthX", "B0_TruthX", 0., -100, 100, "cm")
    B0_Y = ROOT.RooRealVar("B0_Y", "B0_Y", 0., -100, 100, "cm")
    B0_TruthY = ROOT.RooRealVar("B0_TruthY", "B0_TruthY", 0., -100, 100, "cm")
    B0_Z = ROOT.RooRealVar("B0_Z", "B0_Z", 0., -100, 100, "cm")
    B0_TruthZ = ROOT.RooRealVar("B0_TruthZ", "B0_TruthZ", 0., -100, 100, "cm")

    B0_Jpsi_mu0_nPXDHits = ROOT.RooRealVar("B0_Jpsi_mu0_nPXDHits", "B0_Jpsi_mu0_nPXDHits", 1., -10., 100.)
    B0_Jpsi_mu1_nPXDHits = ROOT.RooRealVar("B0_Jpsi_mu1_nPXDHits", "B0_Jpsi_mu1_nPXDHits", 1., -10., 100.)
    B0_Jpsi_mu0_nSVDHits = ROOT.RooRealVar("B0_Jpsi_mu0_nSVDHits", "B0_Jpsi_mu0_nSVDHits", 1., -10., 100.)
    B0_Jpsi_mu1_nSVDHits = ROOT.RooRealVar("B0_Jpsi_mu1_nSVDHits", "B0_Jpsi_mu1_nSVDHits", 1., -10., 100.)

    # B0_Jpsi_mu0_firstSVDLayer = ROOT.RooRealVar("B0_Jpsi_mu0__firstSVDLayer", "B0_pi0_e1__firstSVDLayer", 1., -10., 100.)
    # B0_Jpsi_mu1_firstSVDLayer = ROOT.RooRealVar("B0_Jpsi_mu1__firstSVDLayer", "B0_pi0_e0__firstSVDLayer", 1., -10., 100.)

    DT = ROOT.RooRealVar("DT", "DT", 0., -limDeltaT, limDeltaT, "ps")
    DSigZ = ROOT.RooRealVar("DSigZ", "DSigZ", 0., -limZSig, limZSig, "cm")
    # DSigZ = ROOT.RooFormulaVar("DSigZ", "DSigZ", "@@0-@@1", ROOT.RooArgList(B0_Z, B0_TruthZ))
    DTagZ = ROOT.RooRealVar("DSigZ", "DSigZ", 0., -limZTag, limZTag, "cm")
    # DTagZ = ROOT.RooFormulaVar("DTagZ", "DTagZ", "@@0-@@1", ROOT.RooArgList(B0_TagVz, B0_TruthTagVz))

    cut = "B0_isSignal == 1 "  # + "&& abs(B0_DeltaTErr)< " + str(limDeltaTErr) + " "

    if VXDReq == 'PXD1':
        cut = cut + "&& (B0_Jpsi_mu0_nPXDHits> 0 || B0_Jpsi_mu1_nPXDHits> 0) "
    if VXDReq == 'PXD2':
        cut = cut + "&& B0_Jpsi_mu0_nPXDHits> 0 && B0_Jpsi_mu1_nPXDHits> 0 "

    if VXDReq == 'SVD1':
        cut = cut + "&& (B0_Jpsi_mu0_nSVDHits> 0 || B0_Jpsi_mu1_nSVDHits> 0) "
    if VXDReq == 'SVD2':
        cut = cut + "&& B0_Jpsi_mu0_nSVDHits> 0 && B0_Jpsi_mu1_nSVDHits> 0 "

    argSet = ROOT.RooArgSet(
        # B0_mcTagPDG,
        B0_DeltaT,
        B0_TruthDeltaT,
        # B0_mcErrors,
        # evt_no,
        B0_TagVz,
        B0_TruthTagVz,
        B0_Z,
        B0_TruthZ)

    argSet.add(B0_Jpsi_mu0_nPXDHits)
    argSet.add(B0_Jpsi_mu1_nPXDHits)
    argSet.add(B0_Jpsi_mu0_nSVDHits)
    argSet.add(B0_Jpsi_mu1_nSVDHits)

    argSet.add(B0_isSignal)

    # argSet.add(B0_X)
    # argSet.add(B0_TruthX)
    # argSet.add(B0_Y)
    # argSet.add(B0_TruthY)

    # argSet.add(B0_TagVx)
    # argSet.add(B0_TruthTagVx)
    # argSet.add(B0_TagVy)
    # argSet.add(B0_TruthTagVy)

    # argSet.add(deltaTErr)
    # argSet.add(B0_qrMC)

    data = ROOT.RooDataSet(
        "data",
        "data",
        tdat,
        argSet,
        cut)

    if VXDReq == 'PXD1' or VXDReq == 'PXD2':
        fitDataDTErr = ROOT.RooDataSet("data", "data", tdat, ROOT.RooArgSet(
            B0_isSignal, B0_Jpsi_mu0_nPXDHits, B0_Jpsi_mu1_nPXDHits, deltaTErr), cut)
    elif VXDReq == 'SVD1' or VXDReq == 'SVD2':
        fitDataDTErr = ROOT.RooDataSet("data", "data", tdat, ROOT.RooArgSet(
            B0_isSignal, B0_Jpsi_mu0_nSVDHits, B0_Jpsi_mu1_nSVDHits, deltaTErr), cut)
    else:
        fitDataDTErr = ROOT.RooDataSet("data", "data", tdat, ROOT.RooArgSet(B0_isSignal, deltaTErr), cut)

    # fitData.append(data)

    fitDataDT = ROOT.RooDataSet("fitDataDT", "fitDataDT", ROOT.RooArgSet(DT))
    fitDataSigZ = ROOT.RooDataSet("fitDataSigZ", "fitDataSigZ", ROOT.RooArgSet(DSigZ))
    fitDataTagZ = ROOT.RooDataSet("fitDataTagZ", "fitDataTagZ", ROOT.RooArgSet(DTagZ))

    for i in range(data.numEntries()):

        row = data.get(i)

        tDT = row.getRealValue("B0_DeltaT", 0, ROOT.kTRUE) - row.getRealValue("B0_TruthDeltaT", 0, ROOT.kTRUE)
        if abs(tDT) < limDeltaT:
            DT.setVal(tDT)
            fitDataDT.add(ROOT.RooArgSet(DT))

        tDSigZ = row.getRealValue("B0_Z", 0, ROOT.kTRUE) - row.getRealValue("B0_TruthZ", 0, ROOT.kTRUE)

        if abs(tDSigZ) < limZSig:
            DSigZ.setVal(tDSigZ)
            fitDataSigZ.add(ROOT.RooArgSet(DSigZ))

        tDTagZ = row.getRealValue("B0_TagVz", 0, ROOT.kTRUE) - row.getRealValue("B0_TruthTagVz", 0, ROOT.kTRUE)

        if abs(tDTagZ) < limZTag:
            DTagZ.setVal(tDTagZ)
            fitDataTagZ.add(ROOT.RooArgSet(DTagZ))

    fitDataDTErr.Print()
    fitDataDT.Print()
    fitDataSigZ.Print()
    fitDataTagZ.Print()
    numberOfEntries.append(data.numEntries())

# Fit and plot of the DeltaT Error and DeltaTRECO - DeltaTMC

    Mu1 = ROOT.RooRealVar("Mu1", "Mu1", 0., -limDeltaT, limDeltaT)
    Mu2 = ROOT.RooRealVar("Mu2", "Mu2", 0., -limDeltaT, limDeltaT)
    Mu3 = ROOT.RooRealVar("Mu3", "Mu3", 0., -limDeltaT, limDeltaT)
    Sigma1 = ROOT.RooRealVar("Sigma1", "Sigma1", 1.88046e+00, 0., limDeltaT)
    Sigma2 = ROOT.RooRealVar("Sigma2", "Sigma2", 3.40331e+00, 0., limDeltaT)
    Sigma3 = ROOT.RooRealVar("Sigma3", "Sigma3", 8.23171e-01, 0., limDeltaT)
    frac1 = ROOT.RooRealVar("frac1", "frac1", 5.48703e-01, 0.0, 1.)
    frac2 = ROOT.RooRealVar("frac2", "frac2", 2.60604e-01, 0.0, 1.)

    g1 = ROOT.RooGaussModel("g1", "g1", DT, Mu1, Sigma1)
    g2 = ROOT.RooGaussModel("g2", "g2", DT, Mu2, Sigma2)
    g3 = ROOT.RooGaussModel("g3", "g3", DT, Mu3, Sigma3)

    argset1 = ROOT.RooArgSet(g1)
    argset2 = ROOT.RooArgSet(g2)
    argset3 = ROOT.RooArgSet(g3)

    model = ROOT.RooAddModel("model", "model", ROOT.RooArgList(g1, g2, g3), ROOT.RooArgList(frac1, frac2))

    DT.setRange("fitRange", -limDeltaT, limDeltaT)

    fitRes = model.fitTo(
        fitDataDT,
        ROOT.RooFit.Minos(ROOT.kFALSE), ROOT.RooFit.Extended(ROOT.kFALSE),
        ROOT.RooFit.NumCPU(8), ROOT.RooFit.Save())

    fitRes.Print("v")

    deltaTErr.Print("v")

    resFrame = DT.frame()
    fitDataDT.plotOn(resFrame)

    meanCBS = ROOT.RooRealVar("meanCBS", "meanCBS", 5.37602e-01, 0.5, 1, "ps")
    sigmaCBS = ROOT.RooRealVar("sigmaCBS", "sigmaCBS", 8.16334e-02, 0, 0.1, "ps")
    alphaCBS = ROOT.RooRealVar("alphaCBS", "alphaCBS", -4.85571e-01, -1, 0, "")
    nCBS = ROOT.RooRealVar("nCBS", "nCBS", 1.86325e+00, 0.5, 3, "")
    dtErrCBS = ROOT.RooCBShape("dtErrGen", "dtErrGen", deltaTErr, meanCBS, sigmaCBS, alphaCBS, nCBS)

    MuErr1 = ROOT.RooRealVar("MuErr1", "MuErr1", 4.12399e-01, 0.3, 0.6, "ps")
    SigmaErr1 = ROOT.RooRealVar("SigmaErr1", "SigmaErr1", 5.41152e-02, 0. - 1, 0.07, "ps")
    gErr1 = ROOT.RooGaussModel("gErr1", "gErr1", deltaTErr, MuErr1, SigmaErr1)
    fracErr1 = ROOT.RooRealVar("fracErr1", "fracErr1", 7.50810e-01, 0.0, 1.)

    MuErr2 = ROOT.RooRealVar("MuErr2", "MuErr2", 3.26658e-01, 0.2, 0.4, "ps")
    SigmaErr2 = ROOT.RooRealVar("SigmaErr2", "SigmaErr2", 3.66794e-02, 0.01, 0.08, "ps")
    gErr2 = ROOT.RooGaussModel("gErr2", "gErr2", deltaTErr, MuErr2, SigmaErr2)
    fracErr2 = ROOT.RooRealVar("fracErr2", "fracErr2", 1.82254e-01, 0.0, 1.)

    modelTErr = ROOT.RooAddModel(
        "modelErr", "modelErr", ROOT.RooArgList(
            dtErrCBS, gErr1, gErr2), ROOT.RooArgList(
            fracErr1, fracErr2))

    if VXDReq == 'PXD0' or VXDReq == 'PXD1' or VXDReq == 'PXD2':
        CBSFitRes = modelTErr.fitTo(
            fitDataDTErr,
            ROOT.RooFit.Minos(ROOT.kFALSE), ROOT.RooFit.Extended(ROOT.kFALSE),
            ROOT.RooFit.NumCPU(8), ROOT.RooFit.Save())

    dtErrCBS.Print()

    argset1TErr = ROOT.RooArgSet(dtErrCBS)
    argset2TErr = ROOT.RooArgSet(gErr1)
    argset3TErr = ROOT.RooArgSet(gErr2)

    resFrameDtErr = deltaTErr.frame()
    fitDataDTErr.plotOn(resFrameDtErr)
    modelTErr.plotOn(resFrameDtErr)
    modelTErr.plotOn(
        resFrameDtErr,
        ROOT.RooFit.Components(argset1TErr),
        ROOT.RooFit.LineColor(ROOT.kRed + 2),
        ROOT.RooFit.LineWidth(4))
    modelTErr.plotOn(
        resFrameDtErr,
        ROOT.RooFit.Components(argset2TErr),
        ROOT.RooFit.LineColor(ROOT.kGreen + 3),
        ROOT.RooFit.LineWidth(4))
    modelTErr.plotOn(
        resFrameDtErr,
        ROOT.RooFit.Components(argset3TErr),
        ROOT.RooFit.LineColor(ROOT.kMagenta + 2),
        ROOT.RooFit.LineWidth(4))

    f1 = frac1.getVal()
    f2 = frac2.getVal()
    f3 = 1 - f1 - f2

    argsetList = []
    argsetList.append([argset1, f1])
    argsetList.append([argset2, f2])
    argsetList.append([argset3, f3])

    argsetList = sorted(argsetList, key=itemgetter(1))

    model.plotOn(resFrame, ROOT.RooFit.LineWidth(3))
    model.plotOn(resFrame, ROOT.RooFit.Components(argsetList[2][0]), ROOT.RooFit.LineColor(ROOT.kRed + 2), ROOT.RooFit.LineWidth(4))
    model.plotOn(
        resFrame, ROOT.RooFit.Components(
            argsetList[1][0]), ROOT.RooFit.LineColor(
            ROOT.kGreen + 3), ROOT.RooFit.LineWidth(4))
    model.plotOn(
        resFrame, ROOT.RooFit.Components(
            argsetList[0][0]), ROOT.RooFit.LineColor(
            ROOT.kMagenta + 2), ROOT.RooFit.LineWidth(4))

    resFrame.SetTitle("")
    sXtitle = "#Deltat - Gen. #Delta t / ps"
    resFrame.GetXaxis().SetTitle(sXtitle)
    resFrame.GetXaxis().SetTitleSize(0.05)
    resFrame.GetXaxis().SetLabelSize(0.045)
    resFrame.GetYaxis().SetTitleSize(0.05)
    resFrame.GetYaxis().SetTitleOffset(1.5)
    resFrame.GetYaxis().SetLabelSize(0.045)

    shift = Mu1.getVal() * f1 + Mu2.getVal() * f2 + Mu3.getVal() * f3
    resolution = Sigma1.getVal() * f1 + Sigma2.getVal() * f2 + Sigma3.getVal() * f3

    shiftErr = math.sqrt((Mu1.getError() * f1)**2 + (Mu2.getError() * f2)**2 + (Mu3.getError() * f3)**2)
    resolutionErr = math.sqrt((Sigma1.getError() * f1)**2 + (Sigma2.getError() * f2)**2 + (Sigma3.getError() * f3)**2)

    if shiftErr < 0.01:
        shiftErr = 0.01

    if resolutionErr < 0.01:
        resolutionErr = 0.01

    Numbr = '{:d}'.format(int((f1 + f2) * fitDataDT.numEntries()))

    c1 = ROOT.TCanvas("c1", "c1", 1400, 1100)
    c1.cd()
    Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
    Pad.SetLeftMargin(0.15)
    Pad.SetBottomMargin(0.15)
    Pad.Draw()
    Pad.cd()
    resFrame.Draw()
    l = ROOT.TLegend(0.59, 0.6, 0.9, 0.9)
    # l.AddEntry(0, 'Entries' + '{:>11}'.format(Numbr))
    l.AddEntry(0, '#splitline{#mu_{#Delta t} =' + '{: 4.2f}'.format(shift) + '}{    #pm ' + '{:4.2f}'.format(shiftErr) + ' ps}')
    l.AddEntry(0, '#splitline{#sigma_{#Delta t} =' + '{: 4.2f}'.format(resolution) + '}{    #pm ' +
               '{:4.2f}'.format(resolutionErr) + ' ps}')
    l.SetTextSize(0.054)
    l.SetFillColorAlpha(ROOT.kWhite, 0)
    l.Draw()
    Pad.Update()
    nPlot = PATH + "/test6_CPVResDeltaT" + VXDReq + ".pdf"
    c1.SaveAs(nPlot)
    c1.Clear()

    iResult.append(['mu = ' + '{: 4.2f}'.format(shift) + ' +- ' + '{:4.2f}'.format(shiftErr) + ' ps',
                    'sigma =' + '{: 4.2f}'.format(resolution) + ' +- ' + '{:4.2f}'.format(resolutionErr) + ' ps'])

    resFrameDtErr.SetTitle("")
    sXtitleLandau = "#sigma_{#Deltat} / ps"
    resFrameDtErr.GetXaxis().SetTitle(sXtitleLandau)
    resFrameDtErr.GetXaxis().SetTitleSize(0.05)
    resFrameDtErr.GetXaxis().SetLabelSize(0.045)
    resFrameDtErr.GetYaxis().SetTitleSize(0.05)
    resFrameDtErr.GetYaxis().SetTitleOffset(1.5)
    resFrameDtErr.GetYaxis().SetLabelSize(0.045)

    c1 = ROOT.TCanvas("c1", "c1", 1400, 1100)
    c1.cd()
    Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
    Pad.SetLeftMargin(0.15)
    Pad.SetBottomMargin(0.15)
    Pad.Draw()
    Pad.cd()
    resFrameDtErr.Draw()
    l = ROOT.TLegend(0.59, 0.6, 0.9, 0.9)
    # l.AddEntry(0, 'Entries' + '{:>11}'.format(Numbr))
    l.AddEntry(0, '#splitline{#mu_{#Delta t} =' + '{: 4.2f}'.format(meanCBS.getVal()) + '}{    #pm ' +
               '{:4.2f}'.format(meanCBS.getError()) + ' ps}')  # '{:>6}'.format(Shift)
    l.AddEntry(0, '#splitline{#sigma_{#Delta t} =' + '{: 4.2f}'.format(sigmaCBS.getVal()) +
               '}{    #pm ' + '{:4.2f}'.format(sigmaCBS.getError()) + ' ps}')  # '{:>4}'.format(Resol)
    l.SetTextSize(0.054)
    l.SetFillColorAlpha(ROOT.kWhite, 0)
    # l.Draw()
    Pad.Update()
    nPlot = PATH + "/test6_CPVResDeltaTError" + VXDReq + ".pdf"
    c1.SaveAs(nPlot)
    c1.Clear()

    fitRes.Clear()
    model.Clear()

    # Fit of Delta Z for B0_sig

    Mu1SigZ = ROOT.RooRealVar("Mu1SigZ", "Mu1SigZ", -6.03806e-06, -limZSig, limZSig)
    Mu2SigZ = ROOT.RooRealVar("Mu2SigZ", "Mu2SigZ", 1.45755e-05, -limZSig, limZSig)
    Mu3SigZ = ROOT.RooRealVar("Mu3SigZ", "Mu3SigZ", -1.84464e-04, -limZSig, limZSig)
    Sigma1SigZ = ROOT.RooRealVar("Sigma1SigZ", "Sigma1SigZ", 4.03530e-03, 0., limZSig)
    Sigma2SigZ = ROOT.RooRealVar("Sigma2SigZ", "Sigma2SigZ", 1.73995e-03, 0., limZSig)
    Sigma3SigZ = ROOT.RooRealVar("Sigma3SigZ", "Sigma3SigZ", 2.18176e-02, 0., limZSig)
    frac1SigZ = ROOT.RooRealVar("frac1SigZ", "frac1SigZ", 2.08032e-01, 0.0, 1.)
    frac2SigZ = ROOT.RooRealVar("frac2SigZ", "frac2SigZ", 7.80053e-01, 0.0, 1.)

    g1SigZ = ROOT.RooGaussian("g1", "g1", DSigZ, Mu1SigZ, Sigma1SigZ)
    g2SigZ = ROOT.RooGaussian("g2", "g2", DSigZ, Mu2SigZ, Sigma2SigZ)
    g3SigZ = ROOT.RooGaussian("g3", "g3", DSigZ, Mu3SigZ, Sigma3SigZ)

    argset1SigZ = ROOT.RooArgSet(g1SigZ)
    argset2SigZ = ROOT.RooArgSet(g2SigZ)
    argset3SigZ = ROOT.RooArgSet(g3SigZ)

    modelSigZ = ROOT.RooAddPdf(
        "modelSigZ", "modelSigZ", ROOT.RooArgList(g1SigZ, g2SigZ, g3SigZ),
        ROOT.RooArgList(frac1SigZ, frac2SigZ))

    DSigZ.setRange("fitRange", -limZSig, limZSig)

    fitResSigZ = modelSigZ.fitTo(
        fitDataSigZ, ROOT.RooFit.Minos(ROOT.kFALSE),
        ROOT.RooFit.Extended(ROOT.kFALSE), ROOT.RooFit.NumCPU(1), ROOT.RooFit.Save())

    resFrameSigZ = DSigZ.frame()

    f1SigZ = frac1SigZ.getVal()
    f2SigZ = frac2SigZ.getVal()
    f3SigZ = 1 - f1SigZ - f2SigZ

    argsetList = []
    argsetList.append([argset1SigZ, f1SigZ])
    argsetList.append([argset2SigZ, f2SigZ])
    argsetList.append([argset3SigZ, f3SigZ])

    argsetList = sorted(argsetList, key=itemgetter(1))

    fitDataSigZ.plotOn(resFrameSigZ)
    modelSigZ.plotOn(resFrameSigZ, ROOT.RooFit.LineWidth(4))
    modelSigZ.plotOn(resFrameSigZ, ROOT.RooFit.Components(argsetList[2][0]), ROOT.RooFit.LineColor(ROOT.kRed + 2),
                     ROOT.RooFit.LineWidth(4))
    modelSigZ.plotOn(resFrameSigZ, ROOT.RooFit.Components(argsetList[1][0]), ROOT.RooFit.LineColor(ROOT.kGreen + 3),
                     ROOT.RooFit.LineWidth(4))
    modelSigZ.plotOn(
        resFrameSigZ,
        ROOT.RooFit.Components(argsetList[0][0]),
        ROOT.RooFit.LineColor(
            ROOT.kMagenta + 2),
        ROOT.RooFit.LineWidth(4))
    resFrameSigZ.SetTitle("")

    sXtitleSigZ = "B0_Z - Gen. B0_Z / cm"

    resFrameSigZ.GetXaxis().SetTitle(sXtitleSigZ)
    resFrameSigZ.GetXaxis().SetLimits(-limZSig, limZSig)
    resFrameSigZ.GetXaxis().SetTitleSize(0.05)
    resFrameSigZ.GetXaxis().SetLabelSize(0.045)
    resFrameSigZ.GetYaxis().SetTitleSize(0.05)
    resFrameSigZ.GetYaxis().SetTitleOffset(1.5)
    resFrameSigZ.GetYaxis().SetLabelSize(0.045)

    shiftSigZ = Mu1SigZ.getVal() * f1SigZ + Mu2SigZ.getVal() * f2SigZ + Mu3SigZ.getVal() * f3SigZ
    resolutionSigZ = Sigma1SigZ.getVal() * f1SigZ + Sigma2SigZ.getVal() * f2SigZ + Sigma3SigZ.getVal() * f3SigZ

    shiftSigZ = shiftSigZ * 10000
    resolutionSigZ = resolutionSigZ * 10000

    shiftErrSigZ = math.sqrt((Mu1SigZ.getError() * f1SigZ)**2 + (Mu2SigZ.getError() * f2SigZ) ** 2 +
                             (Mu3SigZ.getError() * f3SigZ)**2) * 10000
    resolutionErrSigZ = math.sqrt((Sigma1SigZ.getError() * f1SigZ)**2 + (Sigma2SigZ.getError() * f2SigZ)**2 +
                                  (Sigma3SigZ.getError() * f3SigZ)**2) * 10000

    cSig = ROOT.TCanvas("c1", "c1", 1400, 1100)
    cSig.cd()
    Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
    Pad.SetLeftMargin(0.15)
    Pad.SetBottomMargin(0.15)
    Pad.Draw()
    Pad.cd()
    resFrameSigZ.Draw()
    l = ROOT.TLegend(0.63, 0.65, 0.9, 0.9)
    # NumbrSigZ = '{:d}'.format(int((f1+f2)*fitDataSigZ.numEntries()))
    # l.AddEntry(0, 'Entries' + '{:>11}'.format(NumbrSigZ))

    l.AddEntry(
        0,
        '#splitline{#mu_{#Delta z} =' +
        '{: 1.1f}'.format(shiftSigZ) +
        '}{    #pm ' +
        '{:1.1f}'.format(shiftErrSigZ) +
        ' #mum}')
    l.AddEntry(
        0,
        '#splitline{#sigma_{#Delta z} =' +
        '{: 1.1f}'.format(resolutionSigZ) +
        '}{    #pm ' +
        '{:1.1f}'.format(resolutionErrSigZ) +
        ' #mum}')

    l.SetTextSize(0.05)
    l.SetFillColorAlpha(ROOT.kWhite, 0)
    l.Draw()
    Pad.Update()
    nPlot = PATH + "/test6_CPVResDeltaZsig" + VXDReq + ".pdf"
    cSig.SaveAs(nPlot)
    cSig.Clear()

    fitResSigZ.Clear()
    modelSigZ.Clear()

    iResult.append(['mu = ' + '{:^5.1f}'.format(shiftSigZ) + ' +- ' + '{:^4.1f}'.format(shiftErrSigZ) + ' mum',
                    'sigma = ' + '{:^4.1f}'.format(resolutionSigZ) + ' +- ' + '{:^4.1f}'.format(resolutionErrSigZ) + ' mum'])

    # Fit of Delta z for B0_Tag

    Mu1TagZ = ROOT.RooRealVar("Mu1TagZ", "Mu1TagZ", 0., -limZTag, limZTag)
    Mu2TagZ = ROOT.RooRealVar("Mu2TagZ", "Mu2TagZ", 0., -limZTag, limZTag)
    Mu3TagZ = ROOT.RooRealVar("Mu3TagZ", "Mu3TagZ", 0., -limZTag, limZTag)
    Sigma1TagZ = ROOT.RooRealVar("Sigma1TagZ", "Sigma1TagZ", 2.51877e-02, 0., limZTag)
    Sigma2TagZ = ROOT.RooRealVar("Sigma2TagZ", "Sigma2TagZ", 1.54011e-02, 0., limZTag)
    Sigma3TagZ = ROOT.RooRealVar("Sigma3TagZ", "Sigma3TagZ", 1.61081e-02, 0., limZTag)
    frac1TagZ = ROOT.RooRealVar("frac1TagZ", "frac1TagZ", 1.20825e-01, 0.0, 1.)
    frac2TagZ = ROOT.RooRealVar("frac2TagZ", "frac2TagZ", 1.10840e-01, 0.0, 1.)

    g1TagZ = ROOT.RooGaussian("g1", "g1", DTagZ, Mu1TagZ, Sigma1TagZ)
    g2TagZ = ROOT.RooGaussian("g2", "g2", DTagZ, Mu2TagZ, Sigma2TagZ)
    g3TagZ = ROOT.RooGaussian("g3", "g3", DTagZ, Mu3TagZ, Sigma3TagZ)

    argset1TagZ = ROOT.RooArgSet(g1TagZ)
    argset2TagZ = ROOT.RooArgSet(g2TagZ)
    argset3TagZ = ROOT.RooArgSet(g3TagZ)

    modelTagZ = ROOT.RooAddPdf("modelTagZ", "modelTagZ", ROOT.RooArgList(
        g1TagZ, g2TagZ, g3TagZ), ROOT.RooArgList(frac1TagZ, frac2TagZ))

    DTagZ.setRange("fitRange", -limZTag, limZTag)

    fitResTagZ = modelTagZ.fitTo(
        fitDataTagZ, ROOT.RooFit.Minos(
            ROOT.kFALSE), ROOT.RooFit.Extended(
            ROOT.kFALSE), ROOT.RooFit.NumCPU(1), ROOT.RooFit.Save())

    fitResTagZ.Print("v")
    resFrameTagZ = DTagZ.frame()

    f1TagZ = frac1TagZ.getVal()
    f2TagZ = frac2TagZ.getVal()
    f3TagZ = 1 - f1TagZ - f2TagZ

    argsetList = []
    argsetList.append([argset1TagZ, f1TagZ])
    argsetList.append([argset2TagZ, f2TagZ])
    argsetList.append([argset3TagZ, f3TagZ])

    argsetList = sorted(argsetList, key=itemgetter(1))

    fitDataTagZ.plotOn(resFrameTagZ)
    modelTagZ.plotOn(resFrameTagZ, ROOT.RooFit.LineWidth(4))
    modelTagZ.plotOn(
        resFrameTagZ, ROOT.RooFit.Components(
            argsetList[2][0]), ROOT.RooFit.LineColor(
            ROOT.kRed + 2), ROOT.RooFit.LineWidth(4))
    modelTagZ.plotOn(
        resFrameTagZ,
        ROOT.RooFit.Components(argsetList[1][0]),
        ROOT.RooFit.LineColor(ROOT.kGreen + 3),
        ROOT.RooFit.LineWidth(4))
    modelTagZ.plotOn(
        resFrameTagZ,
        ROOT.RooFit.Components(argsetList[0][0]),
        ROOT.RooFit.LineColor(ROOT.kMagenta + 2),
        ROOT.RooFit.LineWidth(4))
    resFrameTagZ.SetTitle("")

    sXtitleTagZ = "B0_TagVz - Gen. B0_TagVz / cm"

    resFrameTagZ.GetXaxis().SetTitle(sXtitleTagZ)
    resFrameTagZ.GetXaxis().SetTitleSize(0.05)
    resFrameTagZ.GetXaxis().SetLabelSize(0.045)
    resFrameTagZ.GetYaxis().SetTitleSize(0.05)
    resFrameTagZ.GetYaxis().SetTitleOffset(1.5)
    resFrameTagZ.GetYaxis().SetLabelSize(0.045)

    shiftTagZ = Mu1TagZ.getVal() * f1TagZ + Mu2TagZ.getVal() * f2TagZ + Mu3TagZ.getVal() * f3TagZ
    resolutionTagZ = Sigma1TagZ.getVal() * f1TagZ + Sigma2TagZ.getVal() * f2TagZ + Sigma3TagZ.getVal() * f3TagZ

    shiftTagZ = shiftTagZ * 10000
    resolutionTagZ = resolutionTagZ * 10000

    shiftErrTagZ = math.sqrt((Mu1TagZ.getError() * f1TagZ)**2 + (Mu2TagZ.getError() * f2TagZ) ** 2 +
                             (Mu3TagZ.getError() * f3TagZ)**2) * 10000
    resolutionErrTagZ = math.sqrt((Sigma1TagZ.getError() * f1TagZ)**2 + (Sigma2TagZ.getError() * f2TagZ)**2 +
                                  (Sigma3TagZ.getError() * f3TagZ)**2) * 10000

    cTag = ROOT.TCanvas("c1", "c1", 1400, 1100)
    cTag.cd()
    Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
    Pad.SetLeftMargin(0.15)
    Pad.SetBottomMargin(0.15)
    Pad.Draw()
    Pad.cd()
    resFrameTagZ.Draw()
    l = ROOT.TLegend(0.64, 0.65, 0.9, 0.9)
    # NumbrTagZ = '{:d}'.format(int((f1+f2)*fitDataTagZ.numEntries()))
    # l.AddEntry(0, 'Entries' + '{:>11}'.format(NumbrTagZ))

    l.AddEntry(0, '#splitline{#mu_{#Delta z} =' + '{: 1.1f}'.format(shiftTagZ) +
               '}{  #pm ' + '{: 1.1f}'.format(shiftErrTagZ) + ' #mum}')
    l.AddEntry(0, '#splitline{#sigma_{#Delta z} =' + '{: 1.1f}'.format(resolutionTagZ) +
               '}{  #pm ' + '{: 1.1f}'.format(resolutionErrTagZ) + ' #mum}')
    l.SetTextSize(0.05)
    l.SetFillColorAlpha(ROOT.kWhite, 0)
    l.Draw()
    Pad.Update()
    nPlot = PATH + "/test6_CPVResDeltaZtag" + VXDReq + ".pdf"
    cTag.SaveAs(nPlot)
    cTag.Clear()

    iResult.append(['mu = ' + '{:^5.1f}'.format(shiftTagZ) + ' +- ' + '{:^4.1f}'.format(shiftErrTagZ) + ' mum',
                    'sigma = ' + '{:^4.1f}'.format(resolutionTagZ) + ' +- ' + '{:^4.1f}'.format(resolutionErrTagZ) + ' mum'])

    fitResults.append(iResult)


print('*********************** FIT RESULTS ***************************')
print('*                                                             *')
print('* WEIGHTED AVERAGES OF THE PARAMETERS                         *')
print('* OF THE FITTED 3 GAUSSIAN                                    *')
print('*                                                             *')
print('**************** WITHOUT PXD HIT REQUIREMENT ******************')
print('*                                                             *')
print('* DeltaT - Gen. DeltaT                                        *')
print('*                                                             *')
print('*    ' + fitResults[0][0][0] + '         ' + fitResults[0][0][1] + '    *')
print('*                                                             *')
print('* SigZ - Gen. SigZ                                            *')
print('*                                                             *')
print('*    ' + fitResults[0][1][0] + '        ' + fitResults[0][1][1] + '   *')
print('*                                                             *')
print('* TagZ - Gen. TagZ                                            *')
print('*                                                             *')
print('*    ' + fitResults[0][2][0] + '        ' + fitResults[0][2][1] + '   *')
print('*                                                             *')
print('********REQUIRING BOTH MUON TRACKS TO HAVE A PXD HIT***********')
print('*                                                             *')
print('* Efficiency                                                  *')
print('*                                                             *')
print('* N_' + VXDReqs[1] + '/N_' + VXDReqs[0] + ' = ' + str(numberOfEntries[1]) + "/" + str(numberOfEntries[0]) + ' = ' +
      '{:^3.2f}'.format(float((numberOfEntries[1] / numberOfEntries[0]) * 100)) + '%  *')
print('*                                                             *')
print('* DeltaT - Gen. DeltaT                                        *')
print('*                                                             *')
print('*    ' + fitResults[1][0][0] + '         ' + fitResults[1][0][1] + '    *')
print('*                                                             *')
print('* SigZ - Gen. SigZ                                            *')
print('*                                                             *')
print('*    ' + fitResults[1][1][0] + '        ' + fitResults[1][1][1] + '   *')
print('*                                                             *')
print('* TagZ - Gen. TagZ                                            *')
print('*                                                             *')
print('*    ' + fitResults[1][2][0] + '        ' + fitResults[1][2][1] + '   *')
print('*                                                             *')
print('***************************************************************')
