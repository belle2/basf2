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
  <input>CPVToolsOutput.root</input>
  <output>test6_CPVResolutionVertexTagV.root</output>
  <contact>Frank Meier; frank.meier@duke.edu</contact>
  <description>This validation script performs a fit of the values of DeltaT, DeltaTErr, Dz for B0_sig and Deltaz for B0_tag.
  The DeltaT and DeltaZ distributions are fitted with 3 Gaussian functions.
  DeltaTErr is fitted with a CBShape function and two Gaussians.
  PXD requirement PXD0 means no hit is required.
  PXD requirement PXD2 means both muon tracks from B->J/Psi Ks are required.</description>
</header>
"""

import ROOT
import math
import array
from operator import itemgetter


PATH = "."

workingFiles = ["../CPVToolsOutput.root"]

limDeltaT = 5
limDeltaTErr = 3.0
limZSig = 0.03
limZTag = 0.03

treename = "B0tree"

# Output Validation file
outputFile = ROOT.TFile("test6_CPVResolutionVertexTagV.root", "RECREATE")

# Values to be watched
outputNtuple = ROOT.TNtuple(
    "Vertex_TagV_Resols",
    "Weighted averages of the resolution params of Vertex and TagV Tools",
    "mu_DT_PXD0:sigma_DT_PXD0:mu_DZsig_PXD0:sigma_DZsig_PXD0:mu_DZtag_PXD0:sigma_DZtag_PXD0:" +
    "mu_DT_PXD2:sigma_DT_PXD2:mu_DZsig_PXD2:sigma_DZsig_PXD2:mu_DZtag_PXD2:sigma_DZtag_PXD2:PXD2_PXD0_Eff")

outputNtuple.SetAlias('Description', "These are the weighted averages of the mean and the standard deviation " +
                      "of the residuals for DeltaT, DeltaZsig and DeltaZtag. The fit is performed with 3 Gaussian functions. " +
                      "The units are ps for DeltaT and microns for DeltaZ.")
outputNtuple.SetAlias(
    'Check',
    "These parameters should not change drastically. Since the nightly reconstruction validation runs " +
    "on the same input file (which changes only from release to release), the values between builds should be the same.")
outputNtuple.SetAlias('Contact', "frank.meier@duke.edu")

# No PXD hit required: PXD0. At least one PXD (SVD) hit for one of the muon tracks: PXD1 (SVD1).
# Hit required for both muon tracks: PXD2 (SVD2)"
VXDReqs = ["PXD0", "PXD2"]


fitResults = []
fitResultsForNtuple = []
numberOfEntries = []

for VXDReq in VXDReqs:
    iResult = []

    tdat = ROOT.TChain(treename)

    for iFile in workingFiles:
        tdat.AddFile(iFile)

    evt_no = ROOT.RooRealVar("evt_no", "evt_no", 0., -10., 10000000.)

    B0_DeltaT = ROOT.RooRealVar("DeltaT", "DeltaT", 0.)
    deltaTErr = ROOT.RooRealVar("DeltaTErr", "DeltaTErr", 0, limDeltaTErr, "ps")
    B0_TruthDeltaT = ROOT.RooRealVar("mcDeltaT", "mcDeltaT", 0.)

    B0_qrMC = ROOT.RooRealVar("qrMC", "qrMC", 0., -100, 100)
    B0_isSignal = ROOT.RooRealVar("isSignal", "isSignal", 0., 0., 512.)

    B0_TagVz = ROOT.RooRealVar("TagVz", "TagVz", 0., -100, 100, "cm")
    B0_TruthTagVz = ROOT.RooRealVar("mcTagVz", "mcTagVz", 0., -100, 100, "cm")

    B0_Z = ROOT.RooRealVar("z", "z", 0., -100, 100, "cm")
    B0_TruthZ = ROOT.RooRealVar("mcDecayVertexZ", "mcDecayVertexZ", 0., -100, 100, "cm")

    B0_Jpsi_mu0_nPXDHits = ROOT.RooRealVar("Jpsi_mu_0_nPXDHits", "Jpsi_mu_0_nPXDHits", 0., -10., 100.)
    B0_Jpsi_mu1_nPXDHits = ROOT.RooRealVar("Jpsi_mu_1_nPXDHits", "Jpsi_mu_1_nPXDHits", 0., -10., 100.)
    B0_Jpsi_mu0_nSVDHits = ROOT.RooRealVar("Jpsi_mu_0_nSVDHits", "Jpsi_mu_0_nSVDHits", 0., -10., 100.)
    B0_Jpsi_mu1_nSVDHits = ROOT.RooRealVar("Jpsi_mu_1_nSVDHits", "Jpsi_mu_1_nSVDHits", 0., -10., 100.)

    # B0_Jpsi_mu0_firstSVDLayer = ROOT.RooRealVar("B0_Jpsi_mu0__firstSVDLayer", "B0_pi0_e1__firstSVDLayer", 1., -10., 100.)
    # B0_Jpsi_mu1_firstSVDLayer = ROOT.RooRealVar("B0_Jpsi_mu1__firstSVDLayer", "B0_pi0_e0__firstSVDLayer", 1., -10., 100.)

    DT = ROOT.RooRealVar("DT", "DT", 0., -limDeltaT, limDeltaT, "ps")
    DSigZ = ROOT.RooRealVar("DSigZ", "DSigZ", 0., -limZSig, limZSig, "cm")
    # DSigZ = ROOT.RooFormulaVar("DSigZ", "DSigZ", "@@0-@@1", ROOT.RooArgList(B0_Z, B0_TruthZ))
    DTagZ = ROOT.RooRealVar("DSigZ", "DSigZ", 0., -limZTag, limZTag, "cm")
    # DTagZ = ROOT.RooFormulaVar("DTagZ", "DTagZ", "@@0-@@1", ROOT.RooArgList(B0_TagVz, B0_TruthTagVz))

    histo_DeltaT = ROOT.TH1F('B0_DeltaT_' + VXDReq, 'Residual of DeltaT',
                             100, -limDeltaT, limDeltaT)
    histo_DeltaTErr = ROOT.TH1F('B0_DeltaTErr_' + VXDReq, 'Residual of DeltaZsig',
                                100, 0, 30)
    histo_DeltaZSig = ROOT.TH1F('B0_DeltaZsig_' + VXDReq, 'Residual of DeltaZsig',
                                100, -limZSig, limZSig)
    histo_DeltaZTag = ROOT.TH1F('B0_DeltaZtag_' + VXDReq, 'Residual of DeltaZsig',
                                100, -limZTag, limZTag)

    cut = "isSignal == 1 "  # + "&& abs(B0_DeltaTErr)< " + str(limDeltaTErr) + " "

    if VXDReq == 'PXD1':
        cut = cut + "&& (Jpsi_mu_0_nPXDHits> 0 || Jpsi_mu_1_nPXDHits> 0) "
    if VXDReq == 'PXD2':
        cut = cut + "&& Jpsi_mu_0_nPXDHits> 0 && Jpsi_mu_1_nPXDHits> 0 "

    if VXDReq == 'SVD1':
        cut = cut + "&& (Jpsi_mu_0_SVDHits> 0 || Jpsi_mu_1_nSVDHits> 0) "
    if VXDReq == 'SVD2':
        cut = cut + "&& Jpsi_mu_0_nSVDHits> 0 && Jpsi_mu_1_nSVDHits> 0 "

    ROOT.gROOT.SetBatch(True)
    c1 = ROOT.TCanvas("c1", "c1", 1400, 1100)

    tdat.Draw("DeltaT - mcDeltaT >> B0_DeltaT_" + VXDReq, cut)
    tdat.Draw("DeltaTErr >> B0_DeltaTErr_" + VXDReq, cut)
    tdat.Draw("z - mcDecayVertexZ >> B0_DeltaZsig_" + VXDReq, cut)
    tdat.Draw("TagVz - mcTagVz >> B0_DeltaZtag_" + VXDReq, cut)

    # Validation Plot 1
    histo_DeltaT.GetXaxis().SetLabelSize(0.04)
    histo_DeltaT.GetYaxis().SetLabelSize(0.04)
    histo_DeltaT.GetYaxis().SetTitleOffset(0.7)
    histo_DeltaT.GetXaxis().SetTitleOffset(0.7)
    histo_DeltaT.GetXaxis().SetTitleSize(0.06)
    histo_DeltaT.GetYaxis().SetTitleSize(0.07)
    histo_DeltaT.SetTitle('DeltaT Residual for PXD requirement ' + VXDReq + '; #Deltat - Gen. #Deltat / ps ; Events')

    histo_DeltaT.GetListOfFunctions().Add(
        ROOT.TNamed('Description', 'DeltaT Residual for PXD requirement ' + VXDReq +
                    '. PXD0 means no PXD hit required. PXD2 means both muon tracks are required to have a PXD hit.'))
    histo_DeltaT.GetListOfFunctions().Add(ROOT.TNamed('Check', 'Std. Dev. and Mean should not change drastically.'))
    histo_DeltaT.GetListOfFunctions().Add(ROOT.TNamed('Contact', 'frank.meier@duke.edu'))
    histo_DeltaT.Write()

    # Validation Plot 2
    histo_DeltaTErr.GetXaxis().SetLabelSize(0.04)
    histo_DeltaTErr.GetYaxis().SetLabelSize(0.04)
    histo_DeltaTErr.GetYaxis().SetTitleOffset(0.7)
    histo_DeltaTErr.GetXaxis().SetTitleOffset(0.7)
    histo_DeltaTErr.GetXaxis().SetTitleSize(0.06)
    histo_DeltaTErr.GetYaxis().SetTitleSize(0.07)
    histo_DeltaTErr.SetTitle('DeltaT error for PXD requirement ' + VXDReq + ' ; #sigma_{#Deltat} / ps ; Events')

    histo_DeltaTErr.GetListOfFunctions().Add(ROOT.TNamed('MetaOptions', 'logy'))
    histo_DeltaTErr.GetListOfFunctions().Add(
        ROOT.TNamed('Description', 'DeltaT error for PXD requirement ' + VXDReq +
                    '. PXD0 means no PXD hit required. PXD2 means both muon tracks are required to have a PXD hit.'))
    histo_DeltaTErr.GetListOfFunctions().Add(
        ROOT.TNamed(
            'Check',
            'Std. Dev. and Mean should not change drastically. Peaks after 2.6 ps should not increase.'))
    histo_DeltaTErr.GetListOfFunctions().Add(ROOT.TNamed('Contact', 'frank.meier@duke.edu'))
    histo_DeltaTErr.Write()

    # Validation Plot 3
    histo_DeltaZSig.GetXaxis().SetLabelSize(0.04)
    histo_DeltaZSig.GetYaxis().SetLabelSize(0.04)
    histo_DeltaZSig.GetYaxis().SetTitleOffset(0.7)
    histo_DeltaZSig.GetXaxis().SetTitleOffset(0.7)
    histo_DeltaZSig.GetXaxis().SetTitleSize(0.06)
    histo_DeltaZSig.GetYaxis().SetTitleSize(0.07)
    histo_DeltaZSig.SetTitle('DeltaZ Residual on signal side for requirement ' + VXDReq + '; B0_Z - Gen. B0_Z / cm ; Events')

    histo_DeltaZSig.GetListOfFunctions().Add(
        ROOT.TNamed('Description', 'DeltaZ Residual on signal side for PXD requirement ' + VXDReq +
                    '. PXD0 means no PXD hit required. PXD2 means both muon tracks are required to have a PXD hit.'))
    histo_DeltaZSig.GetListOfFunctions().Add(ROOT.TNamed('Check', 'Std. Dev. and Mean should not change drastically.'))
    histo_DeltaZSig.GetListOfFunctions().Add(ROOT.TNamed('Contact', 'frank.meier@duke.edu'))
    histo_DeltaZSig.Write()

    # Validation Plot 4
    histo_DeltaZTag.GetXaxis().SetLabelSize(0.04)
    histo_DeltaZTag.GetYaxis().SetLabelSize(0.04)
    histo_DeltaZTag.GetYaxis().SetTitleOffset(0.7)
    histo_DeltaZTag.GetXaxis().SetTitleOffset(0.7)
    histo_DeltaZTag.GetXaxis().SetTitleSize(0.06)
    histo_DeltaZTag.GetYaxis().SetTitleSize(0.07)
    histo_DeltaZTag.SetTitle('DeltaZ Residual on tag side for requirement ' + VXDReq + '; B0_TagVz - Gen. B0_TagVz / cm; Events')

    histo_DeltaZTag.GetListOfFunctions().Add(
        ROOT.TNamed('Description', 'DeltaZ Residual on tag side for PXD requirement ' + VXDReq +
                    '. PXD0 means no PXD hit required. PXD2 means both muon tracks are required to have a PXD hit.'))
    histo_DeltaZTag.GetListOfFunctions().Add(ROOT.TNamed('Check', 'Std. Dev. and Mean should not change drastically.'))
    histo_DeltaZTag.GetListOfFunctions().Add(ROOT.TNamed('Contact', 'frank.meier@duke.edu'))
    histo_DeltaZTag.Write()

    c1.Clear()

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
            B0_isSignal, B0_Jpsi_mu0_nPXDHits, B0_Jpsi_mu1_nPXDHits, deltaTErr),
            f'{cut} && DeltaTErr >= {deltaTErr.getMin()} && DeltaTErr <= {deltaTErr.getMax()}')
    elif VXDReq == 'SVD1' or VXDReq == 'SVD2':
        fitDataDTErr = ROOT.RooDataSet("data", "data", tdat, ROOT.RooArgSet(
            B0_isSignal, B0_Jpsi_mu0_nSVDHits, B0_Jpsi_mu1_nSVDHits, deltaTErr),
            f'{cut} && DeltaTErr >= {deltaTErr.getMin()} && DeltaTErr <= {deltaTErr.getMax()}')
    else:
        fitDataDTErr = ROOT.RooDataSet("data", "data", tdat, ROOT.RooArgSet(B0_isSignal, deltaTErr),
                                       f'{cut} && DeltaTErr >= {deltaTErr.getMin()} && DeltaTErr <= {deltaTErr.getMax()}')

    # fitData.append(data)

    fitDataDT = ROOT.RooDataSet("fitDataDT", "fitDataDT", ROOT.RooArgSet(DT))
    fitDataSigZ = ROOT.RooDataSet("fitDataSigZ", "fitDataSigZ", ROOT.RooArgSet(DSigZ))
    fitDataTagZ = ROOT.RooDataSet("fitDataTagZ", "fitDataTagZ", ROOT.RooArgSet(DTagZ))

    for i in range(data.numEntries()):

        row = data.get(i)

        tDT = row.getRealValue("DeltaT", 0, ROOT.kTRUE) - row.getRealValue("mcDeltaT", 0, ROOT.kTRUE)
        if abs(tDT) < limDeltaT:
            DT.setVal(tDT)
            fitDataDT.add(ROOT.RooArgSet(DT))

        tDSigZ = row.getRealValue("z", 0, ROOT.kTRUE) - row.getRealValue("mcDecayVertexZ", 0, ROOT.kTRUE)

        if abs(tDSigZ) < limZSig:
            DSigZ.setVal(tDSigZ)
            fitDataSigZ.add(ROOT.RooArgSet(DSigZ))

        tDTagZ = row.getRealValue("TagVz", 0, ROOT.kTRUE) - row.getRealValue("mcTagVz", 0, ROOT.kTRUE)

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
    Sigma1 = ROOT.RooRealVar("Sigma1", "Sigma1", 0.4, 0., limDeltaT)
    Sigma2 = ROOT.RooRealVar("Sigma2", "Sigma2", 2.4, 0., limDeltaT)
    Sigma3 = ROOT.RooRealVar("Sigma3", "Sigma3", 0.8, 0., limDeltaT)
    frac1 = ROOT.RooRealVar("frac1", "frac1", 0.6, 0.0, 1.)
    frac2 = ROOT.RooRealVar("frac2", "frac2", 0.1, 0.0, 1.)

    g1 = ROOT.RooGaussModel("g1", "g1", DT, Mu1, Sigma1)
    g2 = ROOT.RooGaussModel("g2", "g2", DT, Mu2, Sigma2)
    g3 = ROOT.RooGaussModel("g3", "g3", DT, Mu3, Sigma3)

    argset1 = ROOT.RooArgSet(g1)
    argset2 = ROOT.RooArgSet(g2)
    argset3 = ROOT.RooArgSet(g3)

    model = ROOT.RooAddModel("model", "model", ROOT.RooArgList(g1, g2, g3), ROOT.RooArgList(frac1, frac2))

    DT.setRange("fitRange", -limDeltaT, limDeltaT)

    fitRes = model.fitTo(fitDataDT, ROOT.RooFit.NumCPU(8), ROOT.RooFit.Save())

    fitRes.Print("v")

    deltaTErr.Print("v")

    resFrame = DT.frame()
    fitDataDT.plotOn(resFrame)

    meanCBS = ROOT.RooRealVar("meanCBS", "meanCBS", 0.4, 0.3, 1, "ps")
    sigmaCBS = ROOT.RooRealVar("sigmaCBS", "sigmaCBS", 0.03, 0.01, 0.1, "ps")
    alphaCBS = ROOT.RooRealVar("alphaCBS", "alphaCBS", -0.4, -1, 0, "")
    nCBS = ROOT.RooRealVar("nCBS", "nCBS", 3, 0.5, 5, "")
    dtErrCBS = ROOT.RooCBShape("dtErrGen", "dtErrGen", deltaTErr, meanCBS, sigmaCBS, alphaCBS, nCBS)

    MuErr1 = ROOT.RooRealVar("MuErr1", "MuErr1", 0.3, 0.2, 0.6, "ps")
    SigmaErr1 = ROOT.RooRealVar("SigmaErr1", "SigmaErr1", 0.03, 0.01, 0.07, "ps")
    gErr1 = ROOT.RooGaussModel("gErr1", "gErr1", deltaTErr, MuErr1, SigmaErr1)
    fracErr1 = ROOT.RooRealVar("fracErr1", "fracErr1", 0.45, 0.0, 0.7)

    MuErr2 = ROOT.RooRealVar("MuErr2", "MuErr2", 0.24, 0.2, 0.4, "ps")
    SigmaErr2 = ROOT.RooRealVar("SigmaErr2", "SigmaErr2", 0.03, 0.01, 0.08, "ps")
    gErr2 = ROOT.RooGaussModel("gErr2", "gErr2", deltaTErr, MuErr2, SigmaErr2)
    fracErr2 = ROOT.RooRealVar("fracErr2", "fracErr2", 0.2, 0.0, 0.5)

    modelTErr = ROOT.RooAddModel(
        "modelErr", "modelErr", ROOT.RooArgList(
            dtErrCBS, gErr1, gErr2), ROOT.RooArgList(
            fracErr1, fracErr2))

    if VXDReq == 'PXD0' or VXDReq == 'PXD1' or VXDReq == 'PXD2':
        CBSFitRes = modelTErr.fitTo(fitDataDTErr, ROOT.RooFit.NumCPU(8), ROOT.RooFit.Save())
        CBSFitRes.Print("v")

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

    Numbr = f'{int((f1 + f2) * fitDataDT.numEntries()):d}'

    c1.cd()
    Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
    Pad.SetLeftMargin(0.15)
    Pad.SetBottomMargin(0.15)
    Pad.Draw()
    Pad.cd()
    resFrame.Draw()
    legend = ROOT.TLegend(0.59, 0.6, 0.9, 0.9)
    # legend.AddEntry(0, 'Entries' + '{:>11}'.format(Numbr))
    legend.AddEntry(0, f'#splitline{{#mu_{{#Delta t}} = {shift:4.2f}}}{{    #pm {shiftErr:4.2f} ps}}')
    legend.AddEntry(0, f'#splitline{{#sigma_{{#Delta t}} = {resolution:4.2f}}}{{    #pm {resolutionErr:4.2f} ps}}')
    legend.SetTextSize(0.054)
    legend.SetFillColorAlpha(ROOT.kWhite, 0)
    legend.Draw()
    Pad.Update()
    nPlot = PATH + "/test6_CPVResDeltaT" + VXDReq + ".pdf"
    c1.SaveAs(nPlot)
    c1.Clear()

    iResult.append([f'mu = {shift:4.2f} +- {shiftErr:4.2f} ps',
                    f'sigma = {resolution:4.2f} +- {resolutionErr:4.2f} ps'])
    fitResultsForNtuple.append(shift)
    fitResultsForNtuple.append(resolution)

    resFrameDtErr.SetTitle("")
    sXtitleLandau = "#sigma_{#Deltat} / ps"
    resFrameDtErr.GetXaxis().SetTitle(sXtitleLandau)
    resFrameDtErr.GetXaxis().SetTitleSize(0.05)
    resFrameDtErr.GetXaxis().SetLabelSize(0.045)
    resFrameDtErr.GetYaxis().SetTitleSize(0.05)
    resFrameDtErr.GetYaxis().SetTitleOffset(1.5)
    resFrameDtErr.GetYaxis().SetLabelSize(0.045)

    c1.cd()
    Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
    Pad.SetLeftMargin(0.15)
    Pad.SetBottomMargin(0.15)
    Pad.Draw()
    Pad.cd()
    resFrameDtErr.Draw()
    legend = ROOT.TLegend(0.59, 0.6, 0.9, 0.9)
    # legend.AddEntry(0, 'Entries' + '{:>11}'.format(Numbr))
    legend.AddEntry(0, f'#splitline{{#mu_{{#Delta t}} = {meanCBS.getVal():4.2f}}}{{    #pm '
                    f'{meanCBS.getError():4.2f} ps}}')  # '{:>6}'.format(Shift)
    legend.AddEntry(0, f'#splitline{{#sigma_{{#Delta t}} = {sigmaCBS.getVal():4.2f}}}'
                    f'{{    #pm {sigmaCBS.getError():4.2f} ps}}')  # '{:>4}'.format(Resol)
    legend.SetTextSize(0.054)
    legend.SetFillColorAlpha(ROOT.kWhite, 0)
    # legend.Draw()
    Pad.Update()
    nPlot = PATH + "/test6_CPVResDeltaTError" + VXDReq + ".pdf"
    c1.SaveAs(nPlot)
    c1.Clear()

    fitRes.Clear()
    model.Clear()

    # Fit of Delta Z for B0_sig

    Mu1SigZ = ROOT.RooRealVar("Mu1SigZ", "Mu1SigZ", -9e-06, -limZSig, limZSig)
    Mu2SigZ = ROOT.RooRealVar("Mu2SigZ", "Mu2SigZ", -1.8e-05, -limZSig, limZSig)
    Mu3SigZ = ROOT.RooRealVar("Mu3SigZ", "Mu3SigZ", 6e-05, -limZSig, limZSig)
    Sigma1SigZ = ROOT.RooRealVar("Sigma1SigZ", "Sigma1SigZ", 3e-03, 1e-6, limZSig)
    Sigma2SigZ = ROOT.RooRealVar("Sigma2SigZ", "Sigma2SigZ", 1.4e-03, 1e-6, limZSig)
    Sigma3SigZ = ROOT.RooRealVar("Sigma3SigZ", "Sigma3SigZ", 0.01, 1e-6, limZSig)
    frac1SigZ = ROOT.RooRealVar("frac1SigZ", "frac1SigZ", 0.2, 0.0, 0.5)
    frac2SigZ = ROOT.RooRealVar("frac2SigZ", "frac2SigZ", 0.75, 0.5, 1.)

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

    fitResSigZ = modelSigZ.fitTo(fitDataSigZ, ROOT.RooFit.NumCPU(1), ROOT.RooFit.Save())

    fitResSigZ.Print("v")

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

    c1.cd()
    Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
    Pad.SetLeftMargin(0.15)
    Pad.SetBottomMargin(0.15)
    Pad.Draw()
    Pad.cd()
    resFrameSigZ.Draw()
    legend = ROOT.TLegend(0.63, 0.65, 0.9, 0.9)
    # NumbrSigZ = '{:d}'.format(int((f1+f2)*fitDataSigZ.numEntries()))
    # legend.AddEntry(0, 'Entries' + '{:>11}'.format(NumbrSigZ))

    legend.AddEntry(
        0,
        f'#splitline{{#mu_{{#Delta z}} = {shiftSigZ:1.1f}}}'
        f'{{    #pm {shiftErrSigZ:1.1f} #mum}}')
    legend.AddEntry(
        0,
        f'#splitline{{#sigma_{{#Delta z}} = {resolutionSigZ:1.1f}}}'
        f'{{    #pm {resolutionErrSigZ:1.1f} #mum}}')

    legend.SetTextSize(0.05)
    legend.SetFillColorAlpha(ROOT.kWhite, 0)
    legend.Draw()
    Pad.Update()
    nPlot = PATH + "/test6_CPVResDeltaZsig" + VXDReq + ".pdf"
    c1.SaveAs(nPlot)
    c1.Clear()

    fitResSigZ.Clear()
    modelSigZ.Clear()

    iResult.append([f'mu = {shiftSigZ:4.1f}  +- {shiftErrSigZ:3.1f}  mum',
                    f'sigma = {resolutionSigZ:4.1f} +- {resolutionErrSigZ:3.1f}  mum'])
    fitResultsForNtuple.append(shiftSigZ)
    fitResultsForNtuple.append(resolutionSigZ)

    # Fit of Delta z for B0_Tag

    Mu1TagZ = ROOT.RooRealVar("Mu1TagZ", "Mu1TagZ", 0., -limZTag, limZTag)
    Mu2TagZ = ROOT.RooRealVar("Mu2TagZ", "Mu2TagZ", 0., -limZTag, limZTag)
    Mu3TagZ = ROOT.RooRealVar("Mu3TagZ", "Mu3TagZ", 0., -limZTag, limZTag)
    Sigma1TagZ = ROOT.RooRealVar("Sigma1TagZ", "Sigma1TagZ", 2.51877e-02, 1e-6, limZTag)
    Sigma2TagZ = ROOT.RooRealVar("Sigma2TagZ", "Sigma2TagZ", 1.54011e-02, 1e-6, limZTag)
    Sigma3TagZ = ROOT.RooRealVar("Sigma3TagZ", "Sigma3TagZ", 1.61081e-02, 1e-6, limZTag)
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

    fitResTagZ = modelTagZ.fitTo(fitDataTagZ, ROOT.RooFit.NumCPU(1), ROOT.RooFit.Save())
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

    c1.cd()
    Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
    Pad.SetLeftMargin(0.15)
    Pad.SetBottomMargin(0.15)
    Pad.Draw()
    Pad.cd()
    resFrameTagZ.Draw()
    legend = ROOT.TLegend(0.64, 0.65, 0.9, 0.9)
    # NumbrTagZ = '{:d}'.format(int((f1+f2)*fitDataTagZ.numEntries()))
    # legend.AddEntry(0, 'Entries' + '{:>11}'.format(NumbrTagZ))

    legend.AddEntry(0, f'#splitline{{#mu_{{#Delta z}} = {shiftTagZ:1.1f}'
                    f'}}{{  #pm {shiftErrTagZ:1.1f} #mum}}')
    legend.AddEntry(0, f'#splitline{{#sigma_{{#Delta z}} = {resolutionTagZ:1.1f}'
                    f'}}{{  #pm {resolutionErrTagZ:1.1f} #mum}}')
    legend.SetTextSize(0.05)
    legend.SetFillColorAlpha(ROOT.kWhite, 0)
    legend.Draw()
    Pad.Update()
    nPlot = PATH + f"/test6_CPVResDeltaZtag{VXDReq}.pdf"
    c1.SaveAs(nPlot)
    c1.Clear()

    iResult.append([f'mu = {shiftTagZ:4.1f}  +- {shiftErrTagZ:3.1f}  mum',
                    f'sigma = {resolutionTagZ:4.1f} +- {resolutionErrTagZ:3.1f}  mum'])
    fitResultsForNtuple.append(shiftTagZ)
    fitResultsForNtuple.append(resolutionTagZ)

    fitResults.append(iResult)

fitResultsForNtuple.append(float((numberOfEntries[1] / numberOfEntries[0]) * 100))
outputNtuple.Fill(array.array('f', fitResultsForNtuple))
outputNtuple.Write()
outputFile.Close()

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
print(f'* N_{VXDReqs[1]}/N_{VXDReqs[0]} = {numberOfEntries[1]}/{numberOfEntries[0]} = '
      f'{(numberOfEntries[1] / numberOfEntries[0]) * 100:3.2f}%                          *')
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
