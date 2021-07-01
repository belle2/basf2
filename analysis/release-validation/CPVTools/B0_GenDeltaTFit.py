#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#######################################################
#
#
# This script checks if the time dependent CP asymmetry has been correctly
# simulated. It saves the generated times for Bsig and Btag
# fits the distributions and saves the plots
# It works for the following decay modes:
#
# B0 -> J/psi Ks
# B0 -> nu_tau anti_nu_tau
#
# If you want to use this script for other decay channels
# you need to modify lines 379-381.
#
# Contributor: F. Abudinen (December 2018)
#
######################################################

import ROOT
import basf2 as b2
from ROOT import Belle2
import modularAnalysis as ma
import numpy as np
import math
from variables import variables as mc_variables

inputFiles = []
Xlim1 = 0
Xlim2 = 999

validation_path = b2.Path()

ma.inputMdstList("default", inputFiles, path=validation_path)

B0s = ('B0:all', '')
ma.fillParticleListsFromMC([B0s], True, path=validation_path)

ma.matchMCTruth('B0:all', path=validation_path)

Upsilon4S = ('Upsilon(4S):all', '')
ma.fillParticleListsFromMC([Upsilon4S], True, path=validation_path)

ma.matchMCTruth('Upsilon(4S):all', path=validation_path)

labelAsymmetry = "#frac{N^{#it{B}^{0}} - N^{#bar{#it{B}}^{0}}}{N^{#it{B}^{0}} + N^{#bar{#it{B}}^{0}}}"


def plotWithAsymmetry(
        rooFitFrame,
        rooRealVar,
        dotsAll,
        dotsA,
        dotsB,
        curveAll,
        curveA,
        curveB,
        units,
        nameOfPlot,
        legend,
        textFitRes,
        asymmVarName,
        removeArtifacts):
    """
    Produces a plot with an asymmetry (y_A -y_B)/(y_A + y_B) below, where y_A and y_B are the y values of curves A and B.
    """

    # plot for Thesis
    rooFitFrame.Print()
    rooFitFrame.GetXaxis().SetTitle("")
    rooFitFrame.GetXaxis().SetLabelSize(0)

    rooFitFrame.GetYaxis().SetTitleSize(0.072)
    rooFitFrame.GetYaxis().SetTitleOffset(0.98)
    rooFitFrame.GetYaxis().SetLabelSize(0.055)

    # yLabelBin = 0
    # yLabelBin = '{:0.0f}'.format(float(dots.GetErrorXlow(1) + dots.GetErrorXhigh(1)))

    pointsHist = ROOT.RooHist()

    iBin = 0

    # xValModel = ROOT.Double(-1.E30)
    # yValModel = ROOT.Double(-1.E30)
    xValDot = ROOT.Double(-1.E30)
    yValDotA = ROOT.Double(-1.E30)
    yValDotB = ROOT.Double(-1.E30)

    iDotsA = ROOT.RooRealVar("iDotsA", "", 0.)
    idotsB = ROOT.RooRealVar("idotsB", "", 0.)
    # iDotError = ROOT.RooRealVar("iDotErrorA", "", 0.)
    # yComb = ROOT.RooFormulaVar("yComb", "", "-1*@0", ROOT.RooArgList(y1RV))
    iAsymmetry = ROOT.RooFormulaVar("yComb", "", "(@0 - @1)/(@0 + @1)", ROOT.RooArgList(iDotsA, idotsB))

    while iBin < dotsA.GetN() - 1:
        dotsA.GetPoint(iBin, xValDot, yValDotA)
        dotsB.GetPoint(iBin, xValDot, yValDotB)

        iDotsA.setVal(yValDotA)
        idotsB.setVal(yValDotB)

        errA = float(dotsA.GetErrorYlow(iBin) + dotsA.GetErrorYhigh(iBin))
        errB = float(dotsB.GetErrorYlow(iBin) + dotsB.GetErrorYhigh(iBin))

        if np.isnan(iAsymmetry.getVal()) is not True:

            asymValue = iAsymmetry.getVal()

            if (yValDotA + yValDotB) > 0:
                asymError = float(2 * math.sqrt(yValDotA**2 * errA**2 + yValDotB**2 * errB**2) / (yValDotA + yValDotB)**2)
            else:
                asymError = 0

            if removeArtifacts:
                if abs(asymValue) > 3:
                    asymValue = 0

            # print("xval = ", xValDot)
            # print(iBin, " = ", iAsymmetry.getVal(), ", @0 = ", yValDot, ", @1 = ", curveA.interpolate(xValDot), ", @2 = ",
            # float(dots.GetErrorYlow(iBin) + dots.GetErrorYhigh(iBin)))
            pointsHist.addBinWithXYError(xValDot, asymValue,
                                         dotsA.GetErrorXlow(iBin),
                                         dotsA.GetErrorXhigh(iBin),
                                         asymError, asymError,)

        iBin += 1

    pointsHist.SetMarkerStyle(dotsA.GetMarkerStyle())
    # pointsHist.SetMarkerSize(dotsA.GetMarkerSize())

    rooFitFramAsym = rooRealVar.frame()
    # rooFitFramAsym.addObject(mbcFrame.pullHist())
    # rooFitFramAsym.addPlotable(pointsHist, "P")

    rooFitFramAsym.GetXaxis().SetTickSize(0.07)
    rooFitFramAsym.GetYaxis().SetTickSize(0.024)

    rooFitFramAsym.SetTitle("")
    rooFitFramAsym.GetXaxis().SetTitle(rooRealVar.GetTitle() + " " + units)
    rooFitFramAsym.GetYaxis().SetTitle(asymmVarName + "  ")

    if asymmVarName == "#it{a}(#Delta#it{t})":
        rooFitFramAsym.GetXaxis().SetTitleSize(0.2)
        rooFitFramAsym.GetXaxis().SetTitleOffset(0.9)
        rooFitFramAsym.GetYaxis().SetTitleSize(0.19)
        rooFitFramAsym.GetYaxis().SetTitleOffset(0.32)
    else:
        rooFitFramAsym.GetXaxis().SetTitleSize(0.18)
        rooFitFramAsym.GetXaxis().SetTitleOffset(0.95)
        rooFitFramAsym.GetYaxis().SetTitleSize(0.13)
        rooFitFramAsym.GetYaxis().SetTitleOffset(0.52)

    rooFitFramAsym.GetXaxis().SetLabelSize(0.155)
    rooFitFramAsym.GetYaxis().SetLabelSize(0.120)

    rooFitFramAsym.SetAxisRange(-1.0, 1.0, "Y")
    rooFitFramAsym.GetYaxis().SetNdivisions(10)
    rooFitFramAsym.GetYaxis().ChangeLabel(1, -1, 0.)
    rooFitFramAsym.GetYaxis().ChangeLabel(3, -1, 0.)
    rooFitFramAsym.GetYaxis().ChangeLabel(5, -1, 0.)
    rooFitFramAsym.GetYaxis().ChangeLabel(6, -1, -1, -1, -1, -1, "0.0")
    rooFitFramAsym.GetYaxis().ChangeLabel(7, -1, 0.)
    rooFitFramAsym.GetYaxis().ChangeLabel(9, -1, 0.)
    rooFitFramAsym.GetYaxis().ChangeLabel(11, -1, 0.)

    # print("Number of Y axis bins = ", rooFitFramAsym.GetYaxis().GetNbins() )

    gLine0 = ROOT.TLine(rooFitFramAsym.GetXaxis().GetXmin(), 0,
                        rooFitFramAsym.GetXaxis().GetXmax(), 0)
    gLine0.SetLineWidth(1)
    gLine0.SetLineColor(13)
    gLine0.SetLineStyle(3)

    gLine1 = ROOT.TLine(rooFitFramAsym.GetXaxis().GetXmin(), 0.4,
                        rooFitFramAsym.GetXaxis().GetXmax(), 0.4)
    gLine1.SetLineWidth(1)
    gLine1.SetLineColor(13)
    gLine1.SetLineStyle(3)

    gLine2 = ROOT.TLine(rooFitFramAsym.GetXaxis().GetXmin(), -0.4,
                        rooFitFramAsym.GetXaxis().GetXmax(), -0.4)
    gLine2.SetLineWidth(1)
    gLine2.SetLineColor(13)
    gLine2.SetLineStyle(3)

    gLine3 = ROOT.TLine(rooFitFramAsym.GetXaxis().GetXmin(), 0.8,
                        rooFitFramAsym.GetXaxis().GetXmax(), 0.8)
    gLine3.SetLineWidth(1)
    gLine3.SetLineColor(13)
    gLine3.SetLineStyle(3)

    gLine4 = ROOT.TLine(rooFitFramAsym.GetXaxis().GetXmin(), -0.8,
                        rooFitFramAsym.GetXaxis().GetXmax(), -0.8)
    gLine4.SetLineWidth(1)
    gLine4.SetLineColor(13)
    gLine4.SetLineStyle(3)

    c1 = ROOT.TCanvas("c1", "c1", 700, 640)
    c1.SetBottomMargin(0)
    c1.cd()

    Pad1 = ROOT.TPad("p1", "p1", 0, 0.277, 1, 1, 0)
    Pad2 = ROOT.TPad("p2", "p2", 0, 0, 1, 0.276, 0)
    Pad1.Draw()
    Pad2.Draw()

    Pad1.SetLeftMargin(0.15)
    Pad1.SetBottomMargin(0.02)
    Pad1.SetTopMargin(0.06)

    Pad2.SetLeftMargin(0.15)
    Pad2.SetBottomMargin(0.4)
    Pad2.SetTopMargin(0.01)

    Pad2.cd()
    rooFitFramAsym.Draw()
    gLine0.Draw("SAME")
    gLine1.Draw("SAME")
    gLine2.Draw("SAME")
    gLine3.Draw("SAME")
    gLine4.Draw("SAME")
    pointsHist.Draw("P SAME")
    # Pad2.Update()

    Pad1.cd()

    rooFitFrame.Draw()

    # if textFitRes != "":
    #
    #     lFitRes = ROOT.TLegend(0.238, 0.95, 0.7, 0.98)
    #     lFitRes.AddEntry("", textFitRes , " ")
    #     lFitRes.SetBorderSize(0)
    #     lFitRes.SetTextSize(0.056)
    #     lFitRes.Draw()

    if legend != "":
        legend.Draw()
    # Pad1.Update()
    nameOfPlot = "WithAsymm" + nameOfPlot + ".pdf"
    c1.SaveAs(nameOfPlot)
    c1.Clear()


class fitDeltaT(b2.Module):
    """
    This class makes plots to validate the generated CP asymmetry. It saves the decay time information of the simulation
    for both B mesons by running as a basf2 module.  It fills the information in RooDataSets and at the end it fits the
    known quantum mechanical pdfs to the delta T distribution and the individual decay time distributions of
    the signal and the tag-side B mesons to check if the S and the A parameters in the simulation are correct.
    """
    #: @var nentries
    #: Total number of analyzed events.
    #: @var DT
    #: Time difference between the decays of the two generated B mesons.
    #: @var Tsig
    #: Decay time of the signal B mesons. Attention! This script recognizes only B0-> JPsiKs as signal B meson.
    #: @var Ttag
    #: Decay time of the tag-side B meson.
    #: @var DT0
    #: Production time of the B mesons.
    #: @var Tau
    #: B0-meson lifetime.
    #: @var A
    #: Direct CP-violation parameter (CP violation in decay).
    #: @var S
    #: Mixing-induced CP-violation parameter (CP violation caused by the overlap between mixing and decay phases)
    #: @var DM
    #: Mass difference between the two B0-mass eigenstates.
    #: @var Norm
    #: Normalization factor of the quantum mechanical pdfs.
    #: @var BetaGamma
    #: Lorentz boost of the B mesons.
    #: @var q
    #: Flavor of the tag-side B0 meson at the time of its decay.
    #: @var qsig
    #: Flavor of the signal-side B0 meson at the time of its decay.
    #: @var fitData
    #: RooDataSet containing DT and q for each event.
    #: @var fitDataTag
    #: RooDataSet containing Ttag and q for each event.
    #: @var fitDataTagDeltaTPos
    #: RooDataSet containing Ttag and q for positive DT
    #: @var fitDataTagDeltaTNeg
    #: RooDataSet containing Ttag and q for negative DT
    #: @var fitDataSig
    #: RooDataSet containing Tsig and q for each event.
    #: @var fitDataSigDeltaTPos
    #: RooDataSet containing Tsig and q for positive DT
    #: @var fitDataSigDeltaTNeg
    #: RooDataSet containing Tsig and q for negative DT
    #: @var B0sInTagSide
    #: Number of tag-side B0s (positive flavor).
    #: @var B0barsInTagSide
    #: Number of tag-side anti-B0s (negative flavor).
    #: @var B0sInSignalSide
    #: Number of B0s in the signal side (positive flavor).
    #: @var B0barsInSignalSide
    #: Number of anti-B0s in the signal side (negative flavor).
    #: @var fractionB0barInSignalSide
    #: Fraction of anti-B0s in the signal side.
    #: @var fractionB0InSignalSide
    #: Fraction of B0s in the signal side
    #: @var DeltaZsigUpsilon
    #: Difference between the production and the decay vertices of the signal-side B0 meson in z direction.
    #: @var DeltaZtagUpsilon
    #: Difference between the production and the decay vertices of the tag-side B0 meson in z direction.
    #: @var fitDataDeltaZsigUpsilon
    #: RooDataSet containing DeltaZsigUpsilon and q.
    #: @var fitDataDeltaZtagUpsilon
    #: RooDataSet containing DeltaZtagUpsilon and q.
    #: @var TsigPosNeg
    #: Empty variable just to define the limits of the frames in the plots for Tsig.
    #: @var TtagPosNeg
    #: Empty variable just to define the limits of the frames in the plots for Ttag.

    def __init__(self):
        """
        Here the module initializes by declaring the variables corresponding to the interesting physical parameters and
        decay times. The RooDataSets where the information will be saved are also declared.
        """
        super(fitDeltaT, self).__init__()
        self.nentries = 0

        self.DT = ROOT.RooRealVar("DT", "#Delta#it{t}", 0., -11., 11., "ps")
        self.Tsig = ROOT.RooRealVar("Tsig", "#it{t}_{sig}^{gen}", 0., 0., 11., "ps")
        self.Ttag = ROOT.RooRealVar("Ttag", "#it{t}_{tag}^{gen}", 0., 0., 11., "ps")
        self.DT0 = ROOT.RooRealVar("DT0", "DT0", 0., -7., 7.)
        self.Tau = ROOT.RooRealVar("Tau", "Tau", 1.525, 0., 4.)
        self.A = ROOT.RooRealVar("A", "A", 0, -1, 1)
        self.S = ROOT.RooRealVar("S", "S", 0, -1, 1)
        self.DM = ROOT.RooRealVar("DM", "DM", 0.507)
        self.Norm = ROOT.RooRealVar("Norm", "Norm", 2, 1, 8)
        self.BetaGamma = ROOT.RooRealVar("BetaGamma", "BetaGamma", 0.5, 0.1, 1)

        self.q = ROOT.RooCategory("q", "q")
        self.q.defineType("1")
        self.q.defineType("-1")

        self.qsig = ROOT.RooCategory("qsig", "qsig")
        self.qsig.defineType("1")
        self.qsig.defineType("-1")

        self.fitData = ROOT.RooDataSet("fitData", "fitData", ROOT.RooArgSet(self.DT, self.q))
        self.fitDataTag = ROOT.RooDataSet("fitDataTag", "fitDataTag", ROOT.RooArgSet(self.Ttag, self.q))
        self.fitDataTagDeltaTPos = ROOT.RooDataSet("fitDataTagDeltaTPos", "fitDataTagDeltaTPos", ROOT.RooArgSet(self.Ttag, self.q))
        self.fitDataTagDeltaTNeg = ROOT.RooDataSet("fitDataTagDeltaTNeg", "fitDataTagDeltaTNeg", ROOT.RooArgSet(self.Ttag, self.q))
        self.fitDataSig = ROOT.RooDataSet("fitDataSig", "fitDataSig", ROOT.RooArgSet(self.Tsig, self.q, self.qsig))
        self.fitDataSigDeltaTPos = ROOT.RooDataSet("fitDataSigDeltaTPos", "fitDataSigDeltaTPos", ROOT.RooArgSet(self.Tsig, self.q))
        self.fitDataSigDeltaTNeg = ROOT.RooDataSet("fitDataSigDeltaTNeg", "fitDataSigDeltaTNeg", ROOT.RooArgSet(self.Tsig, self.q))

        self.B0sInTagSide = 0
        self.B0barsInTagSide = 0

        self.B0sInSignalSide = 0
        self.B0barsInSignalSide = 0
        self.fractionB0barInSignalSide = 0.7057091319609885
        self.fractionB0InSignalSide = 0.2942908680390115

        # Study od Delta z

        self.DeltaZsigUpsilon = ROOT.RooRealVar("DeltaZsigUpsilon", "(#it{z}_{sig}^{dec} - #it{z}_{sig}^{prod})^{gen}", 0., 0., 0.8)
        self.DeltaZtagUpsilon = ROOT.RooRealVar(
            "DeltaZtagUpsilon",
            "(#it{z}_{tag}^{dec} - #it{z}_{tag}^{prod})^{gen}",
            0.,
            0.,
            0.8,
            "mm")
        self.fitDataDeltaZsigUpsilon = ROOT.RooDataSet(
            "fitDataDeltaZsigUpsilon",
            "fitDataDeltaZsigUpsilon",
            ROOT.RooArgSet(
                self.DeltaZsigUpsilon,
                self.q))
        self.fitDataDeltaZtagUpsilon = ROOT.RooDataSet(
            "fitDataDeltaZtagUpsilon",
            "fitDataDeltaZtagUpsilon",
            ROOT.RooArgSet(
                self.DeltaZtagUpsilon,
                self.q))

        # Only for plotting

        self.TsigPosNeg = ROOT.RooRealVar("Tsig", "#it{t}_{sig}^{gen}", 0., 0., 7., "ps")
        self.TtagPosNeg = ROOT.RooRealVar("Ttag", "#it{t}_{tag}^{gen}", 0., 0., 7., "ps")

    def event(self):
        """
        Here the information is collected event by event.
        """

        plist = Belle2.PyStoreObj('B0:all')
        plistUpsilon = Belle2.PyStoreObj('Upsilon(4S):all')

        tagPDG = 0
        sigPDG = 0
        tSig = 0
        tTag = 0

        zSig = 0
        zTag = 0
        zUpsilon = 0

        DeltaT = 0
        B0Flags = []

        for index in range(0, plistUpsilon.obj().getListSize()):
            Upsilon4Sparticle = plistUpsilon.obj().getParticle(index)

            if Upsilon4Sparticle.getPDGCode() == 300553:
                zUpsilon = mc_variables.evaluate("mcZ", Upsilon4Sparticle)

        for index in range(0, plist.obj().getListSize()):
            B0particle = plist.obj().getParticle(index)  # Pointer to the particle with highest prob
            # MCB0particle = B0particle.getMCParticle()

            isB0sig = False

            if B0particle.getNDaughters() == 2:
                B0daughters = []
                for B0daughter in B0particle.getDaughters():
                    B0daughters.append(B0daughter)
                if (abs(B0daughters[0].getPDGCode()) == 443 and abs(B0daughters[1].getPDGCode()) == 310) or \
                   (abs(B0daughters[1].getPDGCode()) == 443 and abs(B0daughters[0].getPDGCode()) == 310) or \
                   (abs(B0daughters[1].getPDGCode()) == 16 and abs(B0daughters[0].getPDGCode()) == 16):
                    isB0sig = True
                    # print("hi")

            if isB0sig:
                tSig = mc_variables.evaluate("mcDecayTime", B0particle)
                zSig = mc_variables.evaluate("mcZ", B0particle)
                sigPDG = B0particle.getPDGCode()
            else:
                tTag = mc_variables.evaluate("mcDecayTime", B0particle)
                zTag = mc_variables.evaluate("mcZ", B0particle)
                tagPDG = B0particle.getPDGCode()

            # print(tSig, " ", tTag)
            B0Flags.append(isB0sig)

        rejectEventFlag = False

        DeltaT = 1000 * (tSig - tTag)
        # DeltaZ = (zSig - zTag)
        # DeltaT = 100*DeltaZ /(0.284*3)
        if len(B0Flags) == 2:
            if B0Flags[0] != B0Flags[1] and rejectEventFlag is not True:
                self.nentries += 1
                # DeltaT = 1000*(tSig - tTag)
                # print(DeltaT)
                self.DT.setVal(DeltaT)
                self.Tsig.setVal(float(1000 * tSig))
                self.Ttag.setVal(float(1000 * tTag))

                if tagPDG > 0:
                    self.q.setLabel("1")
                    self.B0sInTagSide += 1
                elif tagPDG < 0:
                    self.q.setLabel("-1")
                    self.B0barsInTagSide += 1
                if sigPDG > 0:
                    self.qsig.setLabel("1")
                    self.B0sInSignalSide += 1
                elif sigPDG < 0:
                    self.qsig.setLabel("-1")
                    self.B0barsInSignalSide += 1
                self.fitData.add(ROOT.RooArgSet(self.DT, self.q))
                self.fitDataSig.add(ROOT.RooArgSet(self.Tsig, self.q, self.qsig))
                self.fitDataTag.add(ROOT.RooArgSet(self.Ttag, self.q))

                if DeltaT >= 0:
                    self.fitDataTagDeltaTPos.add(ROOT.RooArgSet(self.Ttag, self.q))
                    self.fitDataSigDeltaTPos.add(ROOT.RooArgSet(self.Tsig, self.q))

                if DeltaT <= 0:
                    self.fitDataTagDeltaTNeg.add(ROOT.RooArgSet(self.Ttag, self.q))
                    self.fitDataSigDeltaTNeg.add(ROOT.RooArgSet(self.Tsig, self.q))

                self.DeltaZtagUpsilon.setVal(10 * float(zTag - zUpsilon))
                self.DeltaZsigUpsilon.setVal(10 * float(zSig - zUpsilon))
                self.fitDataDeltaZtagUpsilon.add(ROOT.RooArgSet(self.DeltaZtagUpsilon, self.q))
                self.fitDataDeltaZsigUpsilon.add(ROOT.RooArgSet(self.DeltaZsigUpsilon, self.q))

    def terminate(self):
        """
        Here the known quantum mechanical pdfs are defined and fitted to the generated MC distributions.
        Afterwards, the plots are saved.
        """
        # S

        # Delta t Fit

        self.fitData.Print()

        self.DM.setConstant(ROOT.kTRUE)
        self.Tau.setConstant(ROOT.kTRUE)
        self.DT0.setConstant(ROOT.kTRUE)
        self.Norm.setConstant(ROOT.kTRUE)
        # self.A.setConstant(ROOT.kTRUE)
        # self.S.setConstant(ROOT.kTRUE)

        # self.S.setVal(6.9184e-01)
        # self.S.setConstant(ROOT.kTRUE)
        # self.A.setVal(4.4702e-03)
        # self.A.setConstant(ROOT.kTRUE)

        DeltaTp = ROOT.RooGenericPdf(
            "DeltaTp",
            "DeltaTp",
            "(exp(-1*abs(DT-DT0)/Tau)/(2*Norm*Tau))*(1+(A*cos(DM*DT)+S*sin(DM*DT)))",
            ROOT.RooArgList(
                self.Norm,
                self.DT,
                self.DT0,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        DeltaTm = ROOT.RooGenericPdf(
            "DeltaTm",
            "DeltaTm",
            "(exp(-1*abs(DT-DT0)/Tau)/(2*Norm*Tau))*(1-(A*cos(DM*DT)+S*sin(DM*DT)))",
            ROOT.RooArgList(
                self.Norm,
                self.DT,
                self.DT0,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        DeltaT = ROOT.RooSimultaneous("DeltaT", "DeltaT", self.q)
        DeltaT.addPdf(DeltaTp, "1")
        DeltaT.addPdf(DeltaTm, "-1")
        DeltaT.Print()

        fitRes = DeltaT.fitTo(
            self.fitData, ROOT.RooFit.Minos(
                ROOT.kFALSE), ROOT.RooFit.Extended(
                ROOT.kFALSE), ROOT.RooFit.NumCPU(1), ROOT.RooFit.Save())
        fitRes.Print("v")

        DTFrame = self.DT.frame()
        print("here", str(DTFrame.GetXaxis().GetXmax()))
        self.fitData.plotOn(DTFrame, ROOT.RooFit.Cut("abs(DT) < " + str(DTFrame.GetXaxis().GetXmax())))
        self.fitData.plotOn(DTFrame,
                            ROOT.RooFit.Cut("q==q::1" + " && abs(DT) < " + str(DTFrame.GetXaxis().GetXmax())),
                            ROOT.RooFit.MarkerColor(ROOT.kRed),
                            ROOT.RooFit.Name("data_histB0"))
        self.fitData.plotOn(DTFrame, ROOT.RooFit.Cut("q==1" + " && abs(DT) < " + str(DTFrame.GetXaxis().GetXmax())),
                            ROOT.RooFit.MarkerColor(ROOT.kBlue + 1), ROOT.RooFit.Name("data_histAB0"))

        # DeltaT.plotOn(DTFrame, ROOT.RooFit.Slice(self.q,"1"), ROOT.RooFit.ProjWData(ROOT.RooArgSet(self.q),self.fitData),
        #  ROOT.RooFit.LineWidth(3), ROOT.RooFit.LineColor( ROOT.kRed),ROOT.RooFit.Components("B02JpsiKsp"));
        # DeltaT.plotOn(DTFrame, ROOT.RooFit.Slice(self.q,"-1"), ROOT.RooFit.ProjWData(ROOT.RooArgSet(self.q),self.fitData),
        # ROOT.RooFit.LineWidth(3), ROOT.RooFit.LineColor( ROOT.kBlue+1), ROOT.RooFit.Components("B02JpsiKsm"));
        DeltaT.plotOn(
            DTFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitData),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlack),
            ROOT.RooFit.LineStyle(3))
        DeltaT.plotOn(
            DTFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitData),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kRed),
            ROOT.RooFit.Components("DeltaTp"))
        DeltaT.plotOn(DTFrame, ROOT.RooFit.ProjWData(ROOT.RooArgSet(self.q), self.fitData), ROOT.RooFit.LineWidth(
            3), ROOT.RooFit.LineColor(ROOT.kBlue + 1), ROOT.RooFit.LineStyle(7), ROOT.RooFit.Components("DeltaTm"))

        DTFrame.SetTitle("")
        DTFrame.GetXaxis().SetTitle("#Delta#it{t}^{gen} [ps]")
        DTFrame.GetXaxis().SetTitleSize(0.05)
        DTFrame.GetXaxis().SetLabelSize(0.045)
        DTFrame.GetYaxis().SetTitleSize(0.05)
        DTFrame.GetYaxis().SetTitleOffset(1.5)
        DTFrame.GetYaxis().SetLabelSize(0.045)

        c1 = ROOT.TCanvas("c1", "c1", 1400, 1100)
        c1.cd()

        self.DT.Print()

        Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
        Pad.SetLeftMargin(0.15)
        Pad.SetBottomMargin(0.15)
        Pad.Draw()
        Pad.cd()
        DTFrame.Draw()
        DTFrame.Print()

        curveMC = DTFrame.getCurve("DeltaT_Norm[DT]")
        curveMCB0 = DTFrame.getCurve("DeltaT_Norm[DT]_Comp[DeltaTp]")
        curveMCAB0 = DTFrame.getCurve("DeltaT_Norm[DT]_Comp[DeltaTm]")
        curveMC.SetFillColor(ROOT.kWhite)
        curveMCB0.SetFillColor(ROOT.kWhite)
        curveMCAB0.SetFillColor(ROOT.kWhite)

        lMC = ROOT.TLegend(0.73, 0.60, 0.89, 0.89)
        lMC.AddEntry(curveMC, "Both")
        lMC.AddEntry(curveMCAB0, '#bar{#it{B}}^{0}')
        lMC.AddEntry(curveMCB0, '#it{B}^{0}')

        lMC.SetTextSize(0.065)
        lMC.Draw()

        textFitRes = "#it{A}_{#it{CP}} = " + \
            '{: 1.2f}'.format(self.A.getValV() if abs(self.A.getValV()) > 0.005 else abs(self.A.getValV())) + \
            ", " + "#it{S}_{#it{CP}} = " + \
            '{: 1.2f}'.format(self.S.getValV() if abs(self.S.getValV()) > 0.005 else abs(self.S.getValV()))

        lFitRes = ROOT.TLegend(0.215, 0.905, 0.7, 0.98)
        lFitRes.AddEntry("", textFitRes, " ")
        lFitRes.SetBorderSize(0)
        lFitRes.SetTextSize(0.054)
        lFitRes.Draw()

        # lFitRes = ROOT.TLegend(0.18, 0.68, 0.40, 0.89)
        # lFitRes.AddEntry("", "#it{A}_{#it{CP}} = " + '{: 1.2f}'.format(self.A.getValV()), " ")
        # lFitRes.AddEntry("", "#it{S}_{#it{CP}} = " + '{: 1.2f}'.format(self.S.getValV()), " " )
        # lFitRes.SetBorderSize(0)
        # lFitRes.SetTextSize(0.054)
        # lFitRes.Draw()

        ROOT.TText(
            0.1,
            1,
            "#it{A}_{#it{CP}} = " +
            '{:1.2f}'.format(
                self.A.getValV()) +
            " #pm " +
            '{:1.2f}'.format(
                self.A.getError()))
        ROOT.TText(
            0.1,
            0.5,
            "#it{A}_{#it{CP}} = " +
            '{:1.2f}'.format(
                self.S.getValV()) +
            " #pm " +
            '{:1.2f}'.format(
                self.S.getError()))

        fitRes.Print("v")
        c1.SaveAs("./" + 'B0_JPsiKs_DeltaTFittedOnGenMC.pdf')
        c1.Clear()

        dotsMC = DTFrame.getHist("h_fitData")
        dotsMCB0 = DTFrame.getHist("data_histB0")
        dotsMCAB0 = DTFrame.getHist("data_histAB0")

        plotWithAsymmetry(
            DTFrame,
            self.DT,
            dotsMC,
            dotsMCB0,
            dotsMCAB0,
            curveMC,
            curveMCB0,
            curveMCAB0,
            "[ps]",
            "DeltaTFittedOnGenMC",
            lMC,
            textFitRes,
            "#it{a}_{#it{CP}}(#Delta#it{t})",
            True)

        # -----------------

        # z_tag fit

        # z_tag fit whole Delta t range

        # self.S.setVal(0.)
        # self.A.setVal(0.)
        self.S.setConstant(ROOT.kTRUE)
        self.A.setConstant(ROOT.kTRUE)

        self.fitDataDeltaZtagUpsilon.Print()
        DeltaZtagUpsilonp = ROOT.RooGenericPdf(
            "DeltaZtagUpsilonp",
            "DeltaZtagUpsilonp",
            "(exp(-1*abs(DeltaZtagUpsilon/(0.3*BetaGamma))/Tau)/(Norm*Tau))*(1 + " +
            "(A*(cos(DM*DeltaZtagUpsilon/(0.3*BetaGamma))+(DM*Tau)*sin(DM*DeltaZtagUpsilon/(0.3*BetaGamma))) + " +
            " S*((DM*Tau)*cos(DM*DeltaZtagUpsilon/(0.3*BetaGamma)) - sin(DM*DeltaZtagUpsilon/(0.3*BetaGamma))))" +
            "/(1+(Tau*DM)*(Tau*DM)))",
            ROOT.RooArgList(
                self.Norm,
                self.BetaGamma,
                self.DeltaZtagUpsilon,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        DeltaZtagUpsilonm = ROOT.RooGenericPdf(
            "DeltaZtagUpsilonm",
            "DeltaZtagUpsilonm",
            "(exp(-1*abs(DeltaZtagUpsilon/(0.3*BetaGamma))/Tau)/(Norm*Tau))*(1 - " +
            "(A*(cos(DM*DeltaZtagUpsilon/(0.3*BetaGamma))+(DM*Tau)*sin(DM*DeltaZtagUpsilon/(0.3*BetaGamma))) + " +
            " S*((DM*Tau)*cos(DM*DeltaZtagUpsilon/(0.3*BetaGamma)) - sin(DM*DeltaZtagUpsilon/(0.3*BetaGamma))))" +
            "/(1+(Tau*DM)*(Tau*DM)))",
            ROOT.RooArgList(
                self.Norm,
                self.BetaGamma,
                self.DeltaZtagUpsilon,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        DeltaZtagUpsilonModel = ROOT.RooSimultaneous("DeltaZtagUpsilonModel", "DeltaZtagUpsilonModel", self.q)
        DeltaZtagUpsilonModel.addPdf(DeltaZtagUpsilonp, "1")
        DeltaZtagUpsilonModel.addPdf(DeltaZtagUpsilonm, "-1")
        DeltaZtagUpsilonModel.Print()

        fitResTag = DeltaZtagUpsilonModel.fitTo(
            self.fitDataDeltaZtagUpsilon, ROOT.RooFit.Minos(
                ROOT.kFALSE), ROOT.RooFit.Extended(
                ROOT.kFALSE), ROOT.RooFit.NumCPU(1), ROOT.RooFit.Save())
        fitResTag.Print("v")

        DeltaZtagUpsilonFrame = self.DeltaZtagUpsilon.frame()
        self.fitDataDeltaZtagUpsilon.plotOn(DeltaZtagUpsilonFrame)
        self.fitDataDeltaZtagUpsilon.plotOn(
            DeltaZtagUpsilonFrame,
            ROOT.RooFit.Cut("q==q::1"),
            ROOT.RooFit.MarkerColor(
                ROOT.kRed),
            ROOT.RooFit.Name("data_histB0tag"))
        self.fitDataDeltaZtagUpsilon.plotOn(
            DeltaZtagUpsilonFrame,
            ROOT.RooFit.Cut("q==1"),
            ROOT.RooFit.MarkerColor(
                ROOT.kBlue + 1),
            ROOT.RooFit.Name("data_histAB0tag"))

        DeltaZtagUpsilonModel.plotOn(
            DeltaZtagUpsilonFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataDeltaZtagUpsilon),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlack),
            ROOT.RooFit.LineStyle(3))
        DeltaZtagUpsilonModel.plotOn(
            DeltaZtagUpsilonFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataDeltaZtagUpsilon),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kRed),
            ROOT.RooFit.Components("DeltaZtagUpsilonp"))
        DeltaZtagUpsilonModel.plotOn(
            DeltaZtagUpsilonFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataDeltaZtagUpsilon),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlue + 1),
            ROOT.RooFit.LineStyle(7),
            ROOT.RooFit.Components("DeltaZtagUpsilonm"))
        DeltaZtagUpsilonFrame.SetTitle("")
        DeltaZtagUpsilonFrame.GetXaxis().SetTitle("(#it{z}_{tag}^{dec} - #it{z}_{tag}^{prod}){}^{gen} [mm]")
        DeltaZtagUpsilonFrame.GetXaxis().SetTitleSize(0.05)
        DeltaZtagUpsilonFrame.GetXaxis().SetLabelSize(0.045)
        DeltaZtagUpsilonFrame.GetYaxis().SetTitleSize(0.05)
        DeltaZtagUpsilonFrame.GetYaxis().SetTitleOffset(1.5)
        DeltaZtagUpsilonFrame.GetYaxis().SetLabelSize(0.045)

        c2 = ROOT.TCanvas("c2", "c2", 1400, 1100)
        c2.cd()

        self.DeltaZtagUpsilon.Print()

        Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
        Pad.SetLeftMargin(0.15)
        Pad.SetBottomMargin(0.15)
        Pad.Draw()
        Pad.cd()
        DeltaZtagUpsilonFrame.Draw()

        curveMC = DeltaZtagUpsilonFrame.getCurve("DeltaZtagUpsilonModel_Norm[DeltaZtagUpsilon]")
        curveMCB0 = DeltaZtagUpsilonFrame.getCurve("DeltaZtagUpsilonModel_Norm[DeltaZtagUpsilon]_Comp[DeltaZtagUpsilonp]")
        curveMCAB0 = DeltaZtagUpsilonFrame.getCurve("DeltaZtagUpsilonModel_Norm[DeltaZtagUpsilon]_Comp[DeltaZtagUpsilonm]")
        curveMC.SetFillColor(ROOT.kWhite)
        curveMCB0.SetFillColor(ROOT.kWhite)
        curveMCAB0.SetFillColor(ROOT.kWhite)

        lMC = ROOT.TLegend(0.73, 0.60, 0.89, 0.89)
        lMC.AddEntry(curveMC, "Both")
        lMC.AddEntry(curveMCAB0, '#bar{#it{B}}^{0}')
        lMC.AddEntry(curveMCB0, '#it{B}^{0}')

        lMC.SetTextSize(0.065)
        lMC.Draw()

        fitResTag.Print("v")
        c2.SaveAs("./" + 'B0_JPsiKs_DeltaZtagUpsilonFittedOnGenMC.pdf')
        c2.Clear()

        dotsMC = DeltaZtagUpsilonFrame.getHist("h_fitDataDeltaZtagUpsilon")
        dotsMCB0 = DeltaZtagUpsilonFrame.getHist("data_histB0tag")
        dotsMCAB0 = DeltaZtagUpsilonFrame.getHist("data_histAB0tag")

        plotWithAsymmetry(
            DeltaZtagUpsilonFrame,
            self.DeltaZtagUpsilon,
            dotsMC,
            dotsMCB0,
            dotsMCAB0,
            curveMC,
            curveMCB0,
            curveMCAB0,
            "[mm]",
            "DeltaZtagUpsilonFittedOnGenMC",
            lMC,
            "",
            labelAsymmetry,
            True)

        self.S.setConstant(ROOT.kFALSE)
        self.A.setConstant(ROOT.kFALSE)

        # -----------------

        # t_tag fit

        # t_tag fit whole Delta t range

        # self.S.setConstant(ROOT.kTRUE)
        self.S.setVal(0.)
        self.A.setVal(0.)
        # self.A.setConstant(ROOT.kTRUE)

        self.fitDataTag.Print()
        Ttagp = ROOT.RooGenericPdf(
            "Ttagp",
            "Ttagp",
            "(exp(-1*abs(Ttag)/Tau)/(Norm*Tau))*(1 + " +
            "(A*(cos(DM*Ttag)+(DM*Tau)*sin(DM*Ttag)) + S*((DM*Tau)*cos(DM*Ttag) - sin(DM*Ttag)))/(1+(Tau*DM)*(Tau*DM)))",
            ROOT.RooArgList(
                self.Norm,
                self.Ttag,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        Ttagm = ROOT.RooGenericPdf(
            "Ttagm",
            "Ttagm",
            "(exp(-1*abs(Ttag)/Tau)/(Norm*Tau))*(1 - " +
            "(A*(cos(DM*Ttag)+(DM*Tau)*sin(DM*Ttag)) + S*((DM*Tau)*cos(DM*Ttag) - sin(DM*Ttag)))/(1+(Tau*DM)*(Tau*DM)))",
            ROOT.RooArgList(
                self.Norm,
                self.Ttag,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        TtagModel = ROOT.RooSimultaneous("TtagModel", "TtagModel", self.q)
        TtagModel.addPdf(Ttagp, "1")
        TtagModel.addPdf(Ttagm, "-1")
        TtagModel.Print()

        fitResTag = TtagModel.fitTo(
            self.fitDataTag, ROOT.RooFit.Minos(
                ROOT.kFALSE), ROOT.RooFit.Extended(
                ROOT.kFALSE), ROOT.RooFit.NumCPU(1), ROOT.RooFit.Save())
        fitResTag.Print("v")

        TtagFrame = self.Ttag.frame()
        self.fitDataTag.plotOn(TtagFrame)
        self.fitDataTag.plotOn(
            TtagFrame,
            ROOT.RooFit.Cut("q==q::1"),
            ROOT.RooFit.MarkerColor(
                ROOT.kRed),
            ROOT.RooFit.Name("data_histB0tag"))
        self.fitDataTag.plotOn(
            TtagFrame,
            ROOT.RooFit.Cut("q==1"),
            ROOT.RooFit.MarkerColor(
                ROOT.kBlue + 1),
            ROOT.RooFit.Name("data_histAB0tag"))

        TtagModel.plotOn(
            TtagFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataTag),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlack),
            ROOT.RooFit.LineStyle(3))
        TtagModel.plotOn(
            TtagFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataTag),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kRed),
            ROOT.RooFit.Components("Ttagp"))
        TtagModel.plotOn(
            TtagFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataTag),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlue + 1),
            ROOT.RooFit.LineStyle(7),
            ROOT.RooFit.Components("Ttagm"))
        TtagFrame.SetTitle("")
        TtagFrame.GetXaxis().SetTitle("#it{t}_{tag}^{gen} [ps]")
        TtagFrame.GetXaxis().SetTitleSize(0.05)
        TtagFrame.GetXaxis().SetLabelSize(0.045)
        TtagFrame.GetYaxis().SetTitleSize(0.05)
        TtagFrame.GetYaxis().SetTitleOffset(1.5)
        TtagFrame.GetYaxis().SetLabelSize(0.045)

        c2 = ROOT.TCanvas("c2", "c2", 1400, 1100)
        c2.cd()

        self.Ttag.Print()

        Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
        Pad.SetLeftMargin(0.15)
        Pad.SetBottomMargin(0.15)
        Pad.Draw()
        Pad.cd()
        TtagFrame.Draw()

        curveMC = TtagFrame.getCurve("TtagModel_Norm[Ttag]")
        curveMCB0 = TtagFrame.getCurve("TtagModel_Norm[Ttag]_Comp[Ttagp]")
        curveMCAB0 = TtagFrame.getCurve("TtagModel_Norm[Ttag]_Comp[Ttagm]")
        curveMC.SetFillColor(ROOT.kWhite)
        curveMCB0.SetFillColor(ROOT.kWhite)
        curveMCAB0.SetFillColor(ROOT.kWhite)

        lMC = ROOT.TLegend(0.73, 0.60, 0.89, 0.89)
        lMC.AddEntry(curveMC, "Both")
        lMC.AddEntry(curveMCAB0, '#bar{#it{B}}^{0}')
        lMC.AddEntry(curveMCB0, '#it{B}^{0}')

        lMC.SetTextSize(0.065)
        lMC.Draw()

        fitResTag.Print("v")
        c2.SaveAs("./" + 'B0_JPsiKs_TtagFittedOnGenMC.pdf')
        c2.Clear()

        dotsMC = TtagFrame.getHist("h_fitDataTag")
        dotsMCB0 = TtagFrame.getHist("data_histB0tag")
        dotsMCAB0 = TtagFrame.getHist("data_histAB0tag")

        plotWithAsymmetry(TtagFrame, self.Ttag, dotsMC, dotsMCB0, dotsMCAB0, curveMC, curveMCB0,
                          curveMCAB0, "[ps]", "TtagFittedOnGenMC", lMC, "", labelAsymmetry, True)

        # ---

        # t_tag fit positive Delta t range

        # self.S.setConstant(ROOT.kTRUE)
        # self.S.setVal(0.)
        # self.A.setVal(0.)
        # self.A.setConstant(ROOT.kTRUE)

        self.fitDataTagDeltaTPos.Print()
        Ttagp = ROOT.RooGenericPdf(
            "Ttagp",
            "Ttagp",
            "(exp(-2*abs(Ttag)/Tau)/(Tau))*(1 + (A + S*(DM*Tau))/(1+(Tau*DM)*(Tau*DM)))",
            ROOT.RooArgList(
                self.Norm,
                self.Ttag,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        Ttagm = ROOT.RooGenericPdf(
            "Ttagm",
            "Ttagm",
            "(exp(-2*abs(Ttag)/Tau)/(Tau))*(1 - (A + S*(DM*Tau))/(1+(Tau*DM)*(Tau*DM)))",
            ROOT.RooArgList(
                self.Norm,
                self.Ttag,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        TtagModel = ROOT.RooSimultaneous("TtagModel", "TtagModel", self.q)
        TtagModel.addPdf(Ttagp, "1")
        TtagModel.addPdf(Ttagm, "-1")
        TtagModel.Print()

        fitResTagPos = TtagModel.fitTo(
            self.fitDataTagDeltaTPos, ROOT.RooFit.Minos(
                ROOT.kFALSE), ROOT.RooFit.Extended(
                ROOT.kFALSE), ROOT.RooFit.NumCPU(1), ROOT.RooFit.Save())
        fitResTagPos.Print("v")

        TtagFrame = self.TtagPosNeg.frame()
        self.fitDataTagDeltaTPos.plotOn(TtagFrame, ROOT.RooFit.Cut("Ttag < " + str(TtagFrame.GetXaxis().GetXmax())))
        self.fitDataTagDeltaTPos.plotOn(TtagFrame,
                                        ROOT.RooFit.Cut("q==q::1 && Ttag < " + str(TtagFrame.GetXaxis().GetXmax())),
                                        ROOT.RooFit.MarkerColor(ROOT.kRed),
                                        ROOT.RooFit.Name("data_histB0tag"))
        self.fitDataTagDeltaTPos.plotOn(TtagFrame,
                                        ROOT.RooFit.Cut("q==1 && Ttag < " + str(TtagFrame.GetXaxis().GetXmax())),
                                        ROOT.RooFit.MarkerColor(ROOT.kBlue + 1),
                                        ROOT.RooFit.Name("data_histAB0tag"))

        TtagModel.plotOn(TtagFrame, ROOT.RooFit.ProjWData(ROOT.RooArgSet(self.q), self.fitDataTagDeltaTPos),
                         ROOT.RooFit.LineWidth(3), ROOT.RooFit.LineColor(ROOT.kBlack), ROOT.RooFit.LineStyle(3))
        TtagModel.plotOn(
            TtagFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataTagDeltaTPos),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kRed),
            ROOT.RooFit.Components("Ttagp"))
        TtagModel.plotOn(
            TtagFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataTagDeltaTPos),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlue + 1),
            ROOT.RooFit.LineStyle(7),
            ROOT.RooFit.Components("Ttagm"))
        TtagFrame.SetTitle("")
        TtagFrame.GetXaxis().SetTitle("#it{t}_{tag}^{gen} [ps]")
        TtagFrame.GetXaxis().SetTitleSize(0.05)
        TtagFrame.GetXaxis().SetLabelSize(0.045)
        TtagFrame.GetYaxis().SetTitleSize(0.05)
        TtagFrame.GetYaxis().SetTitleOffset(1.5)
        TtagFrame.GetYaxis().SetLabelSize(0.045)

        c2 = ROOT.TCanvas("c2", "c2", 1400, 1100)
        c2.cd()

        self.Ttag.Print()

        Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
        Pad.SetLeftMargin(0.15)
        Pad.SetBottomMargin(0.15)
        Pad.Draw()
        Pad.cd()
        TtagFrame.Draw()

        curveMC = TtagFrame.getCurve("TtagModel_Norm[Ttag]")
        curveMCB0 = TtagFrame.getCurve("TtagModel_Norm[Ttag]_Comp[Ttagp]")
        curveMCAB0 = TtagFrame.getCurve("TtagModel_Norm[Ttag]_Comp[Ttagm]")
        curveMC.SetFillColor(ROOT.kWhite)
        curveMCB0.SetFillColor(ROOT.kWhite)
        curveMCAB0.SetFillColor(ROOT.kWhite)

        lMC = ROOT.TLegend(0.73, 0.60, 0.89, 0.89)
        lMC.AddEntry(curveMC, "Both")
        lMC.AddEntry(curveMCAB0, '#bar{#it{B}}^{0}')
        lMC.AddEntry(curveMCB0, '#it{B}^{0}')

        lMC.SetTextSize(0.065)
        lMC.Draw()

        fitResTagPos.Print("v")
        c2.SaveAs("./" + 'B0_JPsiKs_TtagFittedOnGenMCPositiveDeltaT.pdf')
        c2.Clear()

        dotsMC = TtagFrame.getHist("h_fitDataTag")
        dotsMCB0 = TtagFrame.getHist("data_histB0tag")
        dotsMCAB0 = TtagFrame.getHist("data_histAB0tag")

        plotWithAsymmetry(
            TtagFrame,
            self.TtagPosNeg,
            dotsMC,
            dotsMCB0,
            dotsMCAB0,
            curveMC,
            curveMCB0,
            curveMCAB0,
            "[ps]",
            "TtagFittedOnGenMCPositiveDeltaT",
            lMC,
            "",
            labelAsymmetry,
            True)

        # t_tag fit negative Delta t range

        # self.S.setConstant(ROOT.kTRUE)
        self.S.setVal(0.)
        self.A.setVal(0.)
        # self.A.setConstant(ROOT.kTRUE)
        self.A.setConstant(ROOT.kFALSE)

        self.fitDataTagDeltaTNeg.Print()
        Ttagp = ROOT.RooGenericPdf(
            "Ttagp",
            "Ttagp",
            "(exp(-1*abs(Ttag)/Tau)*(1 + " +
            "(A*(cos(DM*Ttag)+(DM*Tau)*sin(DM*Ttag)) + S*((DM*Tau)*cos(DM*Ttag) - sin(DM*Ttag)))/(1+(Tau*DM)*(Tau*DM))) - " +
            "(exp(-2*abs(Ttag)/Tau))*(1 + (A + S*(DM*Tau))/(1+(Tau*DM)*(Tau*DM))))/(Tau)",
            ROOT.RooArgList(
                self.Ttag,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        Ttagm = ROOT.RooGenericPdf(
            "Ttagm",
            "Ttagm",
            "(exp(-1*abs(Ttag)/Tau)*(1 - " +
            "(A*(cos(DM*Ttag)+(DM*Tau)*sin(DM*Ttag)) + S*((DM*Tau)*cos(DM*Ttag) - sin(DM*Ttag)))/(1+(Tau*DM)*(Tau*DM))) - " +
            "(exp(-2*abs(Ttag)/Tau))*(1 - (A + S*(DM*Tau))/(1+(Tau*DM)*(Tau*DM))))/(Tau)",
            ROOT.RooArgList(
                self.Ttag,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        TtagModel = ROOT.RooSimultaneous("TtagModel", "TtagModel", self.q)
        TtagModel.addPdf(Ttagp, "1")
        TtagModel.addPdf(Ttagm, "-1")
        TtagModel.Print()

        fitResTagNeg = TtagModel.fitTo(
            self.fitDataTagDeltaTNeg, ROOT.RooFit.Minos(
                ROOT.kFALSE), ROOT.RooFit.Extended(
                ROOT.kFALSE), ROOT.RooFit.NumCPU(1), ROOT.RooFit.Save())
        fitResTagNeg.Print("v")

        TtagFrame = self.TtagPosNeg.frame()
        self.fitDataTagDeltaTNeg.plotOn(TtagFrame, ROOT.RooFit.Cut("Ttag < " + str(TtagFrame.GetXaxis().GetXmax())))
        self.fitDataTagDeltaTNeg.plotOn(TtagFrame,
                                        ROOT.RooFit.Cut("q==q::1 && Ttag < " + str(TtagFrame.GetXaxis().GetXmax())),
                                        ROOT.RooFit.MarkerColor(ROOT.kRed),
                                        ROOT.RooFit.Name("data_histB0tag"))
        self.fitDataTagDeltaTNeg.plotOn(TtagFrame,
                                        ROOT.RooFit.Cut("q==1 && Ttag < " + str(TtagFrame.GetXaxis().GetXmax())),
                                        ROOT.RooFit.MarkerColor(ROOT.kBlue + 1),
                                        ROOT.RooFit.Name("data_histAB0tag"))

        TtagModel.plotOn(TtagFrame, ROOT.RooFit.ProjWData(ROOT.RooArgSet(self.q), self.fitDataTagDeltaTNeg),
                         ROOT.RooFit.LineWidth(3), ROOT.RooFit.LineColor(ROOT.kBlack), ROOT.RooFit.LineStyle(3))
        TtagModel.plotOn(
            TtagFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataTagDeltaTNeg),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kRed),
            ROOT.RooFit.Components("Ttagp"))
        TtagModel.plotOn(
            TtagFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataTagDeltaTNeg),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlue + 1),
            ROOT.RooFit.LineStyle(7),
            ROOT.RooFit.Components("Ttagm"))
        TtagFrame.SetTitle("")
        TtagFrame.GetXaxis().SetTitle("#it{t}_{tag}^{gen} [ps]")
        TtagFrame.GetXaxis().SetTitleSize(0.05)
        TtagFrame.GetXaxis().SetLabelSize(0.045)
        TtagFrame.GetYaxis().SetTitleSize(0.05)
        TtagFrame.GetYaxis().SetTitleOffset(1.5)
        TtagFrame.GetYaxis().SetLabelSize(0.045)

        c2 = ROOT.TCanvas("c2", "c2", 1400, 1100)
        c2.cd()

        self.Ttag.Print()

        Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
        Pad.SetLeftMargin(0.15)
        Pad.SetBottomMargin(0.15)
        Pad.Draw()
        Pad.cd()
        TtagFrame.Draw()
        TtagFrame.Print()

        # lMC = ROOT.TLegend(0.7, 0.63, 0.9, 0.89)

        curveMC = TtagFrame.getCurve("TtagModel_Norm[Ttag]")
        curveMCB0 = TtagFrame.getCurve("TtagModel_Norm[Ttag]_Comp[Ttagp]")
        curveMCAB0 = TtagFrame.getCurve("TtagModel_Norm[Ttag]_Comp[Ttagm]")
        curveMC.SetFillColor(ROOT.kWhite)
        curveMCB0.SetFillColor(ROOT.kWhite)
        curveMCAB0.SetFillColor(ROOT.kWhite)

        lMC = ROOT.TLegend(0.73, 0.60, 0.89, 0.89)
        lMC.AddEntry(curveMC, "Both")
        lMC.AddEntry(curveMCAB0, '#bar{#it{B}}^{0}')
        lMC.AddEntry(curveMCB0, '#it{B}^{0}')

        lMC.SetTextSize(0.065)
        lMC.Draw()

        fitResTagNeg.Print("v")
        c2.SaveAs("./" + 'B0_JPsiKs_TtagFittedOnGenMCNegativeDeltaT.pdf')
        c2.Clear()

        dotsMC = TtagFrame.getHist("h_fitDataTag")
        dotsMCB0 = TtagFrame.getHist("data_histB0tag")
        dotsMCAB0 = TtagFrame.getHist("data_histAB0tag")

        plotWithAsymmetry(
            TtagFrame,
            self.TtagPosNeg,
            dotsMC,
            dotsMCB0,
            dotsMCAB0,
            curveMC,
            curveMCB0,
            curveMCAB0,
            "[ps]",
            "TtagFittedOnGenMCNegativeDeltaT",
            lMC,
            "",
            labelAsymmetry,
            True)

        # ----------------------

        # z_sig fit

        # z_sig fit whole Delta t range

        # self.S.setVal(0.)
        # self.A.setVal(0.)
        self.S.setConstant(ROOT.kTRUE)
        self.A.setConstant(ROOT.kTRUE)

        self.fitDataDeltaZsigUpsilon.Print()
        DeltaZsigUpsilonp = ROOT.RooGenericPdf(
            "DeltaZsigUpsilonp",
            "DeltaZsigUpsilonp",
            "(exp(-1*abs(DeltaZsigUpsilon/(0.3*BetaGamma))/Tau)/(Norm*Tau))*(1 + " +
            "(A*(cos(DM*DeltaZsigUpsilon/(0.3*BetaGamma))+(DM*Tau)*sin(DM*DeltaZsigUpsilon/(0.3*BetaGamma))) + " +
            "S*(-(DM*Tau)*cos(DM*DeltaZsigUpsilon/(0.3*BetaGamma)) + sin(DM*DeltaZsigUpsilon/(0.3*BetaGamma))))" +
            "/(1+(Tau*DM)*(Tau*DM)))",
            ROOT.RooArgList(
                self.Norm,
                self.BetaGamma,
                self.DeltaZsigUpsilon,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        DeltaZsigUpsilonm = ROOT.RooGenericPdf(
            "DeltaZsigUpsilonm",
            "DeltaZsigUpsilonm",
            "(exp(-1*abs(DeltaZsigUpsilon/(0.3*BetaGamma))/Tau)/(Norm*Tau))*(1 - "
            "(A*(cos(DM*DeltaZsigUpsilon/(0.3*BetaGamma))+(DM*Tau)*sin(DM*DeltaZsigUpsilon/(0.3*BetaGamma))) + "
            "S*(-(DM*Tau)*cos(DM*DeltaZsigUpsilon/(0.3*BetaGamma)) + sin(DM*DeltaZsigUpsilon/(0.3*BetaGamma))))"
            "/(1+(Tau*DM)*(Tau*DM)))",
            ROOT.RooArgList(
                self.Norm,
                self.BetaGamma,
                self.DeltaZsigUpsilon,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        DeltaZsigUpsilonModel = ROOT.RooSimultaneous("DeltaZsigUpsilonModel", "DeltaZsigUpsilonModel", self.q)
        DeltaZsigUpsilonModel.addPdf(DeltaZsigUpsilonp, "1")
        DeltaZsigUpsilonModel.addPdf(DeltaZsigUpsilonm, "-1")
        DeltaZsigUpsilonModel.Print()

        fitResTag = DeltaZsigUpsilonModel.fitTo(
            self.fitDataDeltaZsigUpsilon, ROOT.RooFit.Minos(
                ROOT.kFALSE), ROOT.RooFit.Extended(
                ROOT.kFALSE), ROOT.RooFit.NumCPU(1), ROOT.RooFit.Save())
        fitResTag.Print("v")

        DeltaZsigUpsilonFrame = self.DeltaZsigUpsilon.frame()
        self.fitDataDeltaZsigUpsilon.plotOn(DeltaZsigUpsilonFrame)
        self.fitDataDeltaZsigUpsilon.plotOn(
            DeltaZsigUpsilonFrame,
            ROOT.RooFit.Cut("q==q::1"),
            ROOT.RooFit.MarkerColor(
                ROOT.kRed),
            ROOT.RooFit.Name("data_histB0sig"))
        self.fitDataDeltaZsigUpsilon.plotOn(
            DeltaZsigUpsilonFrame,
            ROOT.RooFit.Cut("q==1"),
            ROOT.RooFit.MarkerColor(
                ROOT.kBlue + 1),
            ROOT.RooFit.Name("data_histAB0sig"))

        DeltaZsigUpsilonModel.plotOn(
            DeltaZsigUpsilonFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataDeltaZsigUpsilon),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlack),
            ROOT.RooFit.LineStyle(3))
        DeltaZsigUpsilonModel.plotOn(
            DeltaZsigUpsilonFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataDeltaZsigUpsilon),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kRed),
            ROOT.RooFit.Components("DeltaZsigUpsilonp"))
        DeltaZsigUpsilonModel.plotOn(
            DeltaZsigUpsilonFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataDeltaZsigUpsilon),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlue + 1),
            ROOT.RooFit.LineStyle(7),
            ROOT.RooFit.Components("DeltaZsigUpsilonm"))
        DeltaZsigUpsilonFrame.SetTitle("")
        DeltaZsigUpsilonFrame.GetXaxis().SetTitle("(#it{z}_{sig}^{dec} - #it{z}_{sig}^{prod}){}^{gen} [mm]")
        DeltaZsigUpsilonFrame.GetXaxis().SetTitleSize(0.05)
        DeltaZsigUpsilonFrame.GetXaxis().SetLabelSize(0.045)
        DeltaZsigUpsilonFrame.GetYaxis().SetTitleSize(0.05)
        DeltaZsigUpsilonFrame.GetYaxis().SetTitleOffset(1.5)
        DeltaZsigUpsilonFrame.GetYaxis().SetLabelSize(0.045)

        c2 = ROOT.TCanvas("c2", "c2", 1400, 1100)
        c2.cd()

        self.DeltaZsigUpsilon.Print()

        Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
        Pad.SetLeftMargin(0.15)
        Pad.SetBottomMargin(0.15)
        Pad.Draw()
        Pad.cd()
        DeltaZsigUpsilonFrame.Draw()

        curveMC = DeltaZsigUpsilonFrame.getCurve("DeltaZsigUpsilonModel_Norm[DeltaZsigUpsilon]")
        curveMCB0 = DeltaZsigUpsilonFrame.getCurve("DeltaZsigUpsilonModel_Norm[DeltaZsigUpsilon]_Comp[DeltaZsigUpsilonp]")
        curveMCAB0 = DeltaZsigUpsilonFrame.getCurve("DeltaZsigUpsilonModel_Norm[DeltaZsigUpsilon]_Comp[DeltaZsigUpsilonm]")
        curveMC.SetFillColor(ROOT.kWhite)
        curveMCB0.SetFillColor(ROOT.kWhite)
        curveMCAB0.SetFillColor(ROOT.kWhite)

        lMC = ROOT.TLegend(0.73, 0.60, 0.89, 0.89)
        lMC.AddEntry(curveMC, "Both")
        lMC.AddEntry(curveMCAB0, '#bar{#it{B}}^{0}')
        lMC.AddEntry(curveMCB0, '#it{B}^{0}')

        lMC.SetTextSize(0.065)
        lMC.Draw()

        fitResTag.Print("v")
        c2.SaveAs("./" + 'B0_JPsiKs_DeltaZsigUpsilonFittedOnGenMC.pdf')
        c2.Clear()

        dotsMC = DeltaZsigUpsilonFrame.getHist("h_fitDataDeltaZsigUpsilon")
        dotsMCB0 = DeltaZsigUpsilonFrame.getHist("data_histB0sig")
        dotsMCAB0 = DeltaZsigUpsilonFrame.getHist("data_histAB0sig")

        plotWithAsymmetry(
            DeltaZsigUpsilonFrame,
            self.DeltaZsigUpsilon,
            dotsMC,
            dotsMCB0,
            dotsMCAB0,
            curveMC,
            curveMCB0,
            curveMCAB0,
            "[mm]",
            "DeltaZsigUpsilonFittedOnGenMC",
            lMC,
            "",
            labelAsymmetry,
            True)

        self.S.setConstant(ROOT.kFALSE)
        self.A.setConstant(ROOT.kFALSE)

        # ---   Signal time fit  --------

        self.fitDataSig.Print()
        Tsigp = ROOT.RooGenericPdf(
            "Tsigp",
            "Tsigp",
            "(exp(-1*abs(Tsig)/Tau)/(Norm*Tau))*(1 + " +
            "(A*(cos(DM*Tsig)+(DM*Tau)*sin(DM*Tsig)) + S*(-(DM*Tau)*cos(DM*Tsig) + sin(DM*Tsig)))/(1+(Tau*DM)*(Tau*DM)))",
            ROOT.RooArgList(
                self.Norm,
                self.Tsig,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        Tsigm = ROOT.RooGenericPdf(
            "Tsigm",
            "Tsigm",
            "(exp(-1*abs(Tsig)/Tau)/(Norm*Tau))*(1 - " +
            "(A*(cos(DM*Tsig)+(DM*Tau)*sin(DM*Tsig)) + S*(-(DM*Tau)*cos(DM*Tsig) + sin(DM*Tsig)))/(1+(Tau*DM)*(Tau*DM)))",
            ROOT.RooArgList(
                self.Norm,
                self.Tsig,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        TsigModel = ROOT.RooSimultaneous("TsigModel", "TsigModel", self.q)
        TsigModel.addPdf(Tsigp, "1")
        TsigModel.addPdf(Tsigm, "-1")
        TsigModel.Print()

        fitResSig = TsigModel.fitTo(
            self.fitDataSig, ROOT.RooFit.Minos(
                ROOT.kFALSE), ROOT.RooFit.Extended(
                ROOT.kFALSE), ROOT.RooFit.NumCPU(1), ROOT.RooFit.Save())
        fitResSig.Print("v")

        TsigFrame = self.Tsig.frame()
        self.fitDataSig.plotOn(TsigFrame)
        self.fitDataSig.plotOn(
            TsigFrame,
            ROOT.RooFit.Cut("q==q::1"),
            ROOT.RooFit.MarkerColor(
                ROOT.kRed),
            ROOT.RooFit.Name("data_histB0sig"))
        self.fitDataSig.plotOn(
            TsigFrame,
            ROOT.RooFit.Cut("q==1"),
            ROOT.RooFit.MarkerColor(
                ROOT.kBlue + 1),
            ROOT.RooFit.Name("data_histAB0sig"))

        TsigModel.plotOn(
            TsigFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.qsig),
                self.fitDataSig),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlack),
            ROOT.RooFit.LineStyle(3))
        TsigModel.plotOn(
            TsigFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.qsig),
                self.fitDataSig),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kRed),
            ROOT.RooFit.Components("Tsigp"))
        TsigModel.plotOn(
            TsigFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.qsig),
                self.fitDataSig),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlue + 1),
            ROOT.RooFit.LineStyle(7),
            ROOT.RooFit.Components("Tsigm"))
        TsigFrame.SetTitle("")
        TsigFrame.GetXaxis().SetTitle("#it{t}_{sig}^{gen} [ps]")
        TsigFrame.GetXaxis().SetTitleSize(0.05)
        TsigFrame.GetXaxis().SetLabelSize(0.045)
        TsigFrame.GetYaxis().SetTitleSize(0.05)
        TsigFrame.GetYaxis().SetTitleOffset(1.5)
        TsigFrame.GetYaxis().SetLabelSize(0.045)

        c3 = ROOT.TCanvas("c2", "c2", 1400, 1100)
        c3.cd()

        self.Tsig.Print()

        Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
        Pad.SetLeftMargin(0.15)
        Pad.SetBottomMargin(0.15)
        Pad.Draw()
        Pad.cd()
        TsigFrame.Draw()

        curveMC = TsigFrame.getCurve("TsigModel_Norm[Tsig]")
        curveMCB0 = TsigFrame.getCurve("TsigModel_Norm[Tsig]_Comp[Tsigp]")
        curveMCAB0 = TsigFrame.getCurve("TsigModel_Norm[Tsig]_Comp[Tsigm]")
        curveMC.SetFillColor(ROOT.kWhite)
        curveMCB0.SetFillColor(ROOT.kWhite)
        curveMCAB0.SetFillColor(ROOT.kWhite)

        lMC = ROOT.TLegend(0.73, 0.60, 0.89, 0.89)
        lMC.AddEntry(curveMC, "Both")
        lMC.AddEntry(curveMCAB0, '#bar{#it{B}}^{0}')
        lMC.AddEntry(curveMCB0, '#it{B}^{0}')

        lMC.SetTextSize(0.065)
        lMC.Draw()

        fitResSig.Print("v")
        c3.SaveAs("./" + 'B0_JPsiKs_TsigFittedOnGenMC.pdf')
        c3.Clear()

        dotsMC = TsigFrame.getHist("h_fitDataSig")
        dotsMCB0 = TsigFrame.getHist("data_histB0sig")
        dotsMCAB0 = TsigFrame.getHist("data_histAB0sig")

        plotWithAsymmetry(TsigFrame, self.Tsig, dotsMC, dotsMCB0, dotsMCAB0, curveMC, curveMCB0,
                          curveMCAB0, "[ps]", "TsigFittedOnGenMC", lMC, "", labelAsymmetry, True)

        # ---  ------

        # t_sig fit positive Delta t range

        # self.S.setConstant(ROOT.kTRUE)
        # self.S.setVal(0.)
        # self.A.setVal(0.)
        # self.A.setConstant(ROOT.kTRUE)

        self.fitDataSigDeltaTPos.Print()

        Tsigp = ROOT.RooGenericPdf(
            "Tsigp",
            "Tsigp",
            "(exp(-1*abs(Tsig)/Tau)*(1 + " +
            "(A*(cos(DM*Tsig)+(DM*Tau)*sin(DM*Tsig)) + S*(-(DM*Tau)*cos(DM*Tsig) + sin(DM*Tsig)))/(1+(Tau*DM)*(Tau*DM))) - " +
            "(exp(-2*abs(Tsig)/Tau))*(1 + (A - S*(DM*Tau))/(1+(Tau*DM)*(Tau*DM))))/(Tau)",
            ROOT.RooArgList(
                self.Tsig,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        Tsigm = ROOT.RooGenericPdf(
            "Tsigm",
            "Tsigm",
            "(exp(-1*abs(Tsig)/Tau)*(1 - " +
            "(A*(cos(DM*Tsig)+(DM*Tau)*sin(DM*Tsig)) + S*(-(DM*Tau)*cos(DM*Tsig) + sin(DM*Tsig)))/(1+(Tau*DM)*(Tau*DM))) - " +
            "(exp(-2*abs(Tsig)/Tau))*(1 - (A - S*(DM*Tau))/(1+(Tau*DM)*(Tau*DM))))/(Tau)",
            ROOT.RooArgList(
                self.Tsig,
                self.Tau,
                self.A,
                self.S,
                self.DM))

        TsigModel = ROOT.RooSimultaneous("TsigModel", "TsigModel", self.q)
        TsigModel.addPdf(Tsigp, "1")
        TsigModel.addPdf(Tsigm, "-1")
        TsigModel.Print()

        fitResSigPos = TsigModel.fitTo(
            self.fitDataSigDeltaTPos, ROOT.RooFit.Minos(
                ROOT.kFALSE), ROOT.RooFit.Extended(
                ROOT.kFALSE), ROOT.RooFit.NumCPU(1), ROOT.RooFit.Save())
        fitResSigPos.Print("v")

        TsigFrame = self.TsigPosNeg.frame()
        self.fitDataSigDeltaTPos.plotOn(TsigFrame, ROOT.RooFit.Cut("Tsig < " + str(TsigFrame.GetXaxis().GetXmax())))
        self.fitDataSigDeltaTPos.plotOn(TsigFrame,
                                        ROOT.RooFit.Cut("q==q::1 && Tsig < " + str(TsigFrame.GetXaxis().GetXmax())),
                                        ROOT.RooFit.MarkerColor(ROOT.kRed),
                                        ROOT.RooFit.Name("data_histB0sig"))
        self.fitDataSigDeltaTPos.plotOn(TsigFrame,
                                        ROOT.RooFit.Cut("q==1 && Tsig < " + str(TsigFrame.GetXaxis().GetXmax())),
                                        ROOT.RooFit.MarkerColor(ROOT.kBlue + 1),
                                        ROOT.RooFit.Name("data_histAB0sig"))

        TsigModel.plotOn(TsigFrame, ROOT.RooFit.ProjWData(ROOT.RooArgSet(self.q), self.fitDataSigDeltaTPos),
                         ROOT.RooFit.LineWidth(3), ROOT.RooFit.LineColor(ROOT.kBlack), ROOT.RooFit.LineStyle(3))
        TsigModel.plotOn(
            TsigFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataSigDeltaTPos),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kRed),
            ROOT.RooFit.Components("Tsigp"))
        TsigModel.plotOn(
            TsigFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataSigDeltaTPos),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlue + 1),
            ROOT.RooFit.LineStyle(7),
            ROOT.RooFit.Components("Tsigm"))
        TsigFrame.SetTitle("")
        TsigFrame.GetXaxis().SetTitle("#it{t}_{sig}^{gen} [ps]")
        TsigFrame.GetXaxis().SetTitleSize(0.05)
        TsigFrame.GetXaxis().SetLabelSize(0.045)
        TsigFrame.GetYaxis().SetTitleSize(0.05)
        TsigFrame.GetYaxis().SetTitleOffset(1.5)
        TsigFrame.GetYaxis().SetLabelSize(0.045)

        c2 = ROOT.TCanvas("c2", "c2", 1400, 1100)
        c2.cd()

        self.Tsig.Print()

        Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
        Pad.SetLeftMargin(0.15)
        Pad.SetBottomMargin(0.15)
        Pad.Draw()
        Pad.cd()
        TsigFrame.Draw()

        curveMC = TsigFrame.getCurve("TsigModel_Norm[Tsig]")
        curveMCB0 = TsigFrame.getCurve("TsigModel_Norm[Tsig]_Comp[Tsigp]")
        curveMCAB0 = TsigFrame.getCurve("TsigModel_Norm[Tsig]_Comp[Tsigm]")
        curveMC.SetFillColor(ROOT.kWhite)
        curveMCB0.SetFillColor(ROOT.kWhite)
        curveMCAB0.SetFillColor(ROOT.kWhite)

        lMC = ROOT.TLegend(0.73, 0.60, 0.89, 0.89)
        lMC.AddEntry(curveMC, "Both")
        lMC.AddEntry(curveMCAB0, '#bar{#it{B}}^{0}')
        lMC.AddEntry(curveMCB0, '#it{B}^{0}')

        lMC.SetTextSize(0.065)
        lMC.Draw()

        fitResSigPos.Print("v")
        c2.SaveAs("./" + 'B0_JPsiKs_TsigFittedOnGenMCPositiveDeltaT.pdf')
        c2.Clear()

        dotsMC = TsigFrame.getHist("h_fitDataSig")
        dotsMCB0 = TsigFrame.getHist("data_histB0sig")
        dotsMCAB0 = TsigFrame.getHist("data_histAB0sig")

        plotWithAsymmetry(
            TsigFrame,
            self.TsigPosNeg,
            dotsMC,
            dotsMCB0,
            dotsMCAB0,
            curveMC,
            curveMCB0,
            curveMCAB0,
            "[ps]",
            "TsigFittedOnGenMCPositiveDeltaT",
            lMC,
            "",
            labelAsymmetry,
            True)

        # t_sig fit negative Delta t range

        # self.S.setConstant(ROOT.kTRUE)
        self.S.setVal(0.)
        self.A.setVal(0.)
        # self.A.setConstant(ROOT.kTRUE)
        self.A.setConstant(ROOT.kFALSE)

        self.fitDataSigDeltaTNeg.Print()

        Tsigp = ROOT.RooGenericPdf(
            "Tsigp",
            "Tsigp",
            "(exp(-2*abs(Tsig)/Tau)/(Tau))*(1 + (A - S*(DM*Tau))/(1+(Tau*DM)*(Tau*DM)))",
            ROOT.RooArgList(
                self.Norm,
                self.Tsig,
                self.Tau,
                self.A,
                self.S,
                self.DM))
        Tsigm = ROOT.RooGenericPdf(
            "Tsigm",
            "Tsigm",
            "(exp(-2*abs(Tsig)/Tau)/(Tau))*(1 - (A - S*(DM*Tau))/(1+(Tau*DM)*(Tau*DM)))",
            ROOT.RooArgList(
                self.Norm,
                self.Tsig,
                self.Tau,
                self.A,
                self.S,
                self.DM))

        TsigModel = ROOT.RooSimultaneous("TsigModel", "TsigModel", self.q)
        TsigModel.addPdf(Tsigp, "1")
        TsigModel.addPdf(Tsigm, "-1")
        TsigModel.Print()

        fitResSigNeg = TsigModel.fitTo(
            self.fitDataSigDeltaTNeg, ROOT.RooFit.Minos(
                ROOT.kFALSE), ROOT.RooFit.Extended(
                ROOT.kFALSE), ROOT.RooFit.NumCPU(1), ROOT.RooFit.Save())
        fitResSigNeg.Print("v")

        TsigFrame = self.TsigPosNeg.frame()
        self.fitDataSigDeltaTNeg.plotOn(TsigFrame, ROOT.RooFit.Cut("Tsig < " + str(TsigFrame.GetXaxis().GetXmax())))
        self.fitDataSigDeltaTNeg.plotOn(TsigFrame,
                                        ROOT.RooFit.Cut("q==q::1 && Tsig < " + str(TsigFrame.GetXaxis().GetXmax())),
                                        ROOT.RooFit.MarkerColor(ROOT.kRed),
                                        ROOT.RooFit.Name("data_histB0sig"))
        self.fitDataSigDeltaTNeg.plotOn(TsigFrame,
                                        ROOT.RooFit.Cut("q==1 && Tsig < " + str(TsigFrame.GetXaxis().GetXmax())),
                                        ROOT.RooFit.MarkerColor(ROOT.kBlue + 1),
                                        ROOT.RooFit.Name("data_histAB0sig"))

        TsigModel.plotOn(TsigFrame, ROOT.RooFit.ProjWData(ROOT.RooArgSet(self.q), self.fitDataSigDeltaTNeg),
                         ROOT.RooFit.LineWidth(3), ROOT.RooFit.LineColor(ROOT.kBlack), ROOT.RooFit.LineStyle(3))
        TsigModel.plotOn(
            TsigFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataSigDeltaTNeg),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kRed),
            ROOT.RooFit.Components("Tsigp"))
        TsigModel.plotOn(
            TsigFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.q),
                self.fitDataSigDeltaTNeg),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlue + 1),
            ROOT.RooFit.LineStyle(7),
            ROOT.RooFit.Components("Tsigm"))
        TsigFrame.SetTitle("")
        TsigFrame.GetXaxis().SetTitle("#it{t}_{sig}^{gen} [ps]")
        TsigFrame.GetXaxis().SetTitleSize(0.05)
        TsigFrame.GetXaxis().SetLabelSize(0.045)
        TsigFrame.GetYaxis().SetTitleSize(0.05)
        TsigFrame.GetYaxis().SetTitleOffset(1.5)
        TsigFrame.GetYaxis().SetLabelSize(0.045)

        c2 = ROOT.TCanvas("c2", "c2", 1400, 1100)
        c2.cd()

        self.Tsig.Print()

        Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
        Pad.SetLeftMargin(0.15)
        Pad.SetBottomMargin(0.15)
        Pad.Draw()
        Pad.cd()
        TsigFrame.Draw()
        TsigFrame.Print()

        # lMC = ROOT.TLegend(0.7, 0.63, 0.9, 0.89)

        curveMC = TsigFrame.getCurve("TsigModel_Norm[Tsig]")
        curveMCB0 = TsigFrame.getCurve("TsigModel_Norm[Tsig]_Comp[Tsigp]")
        curveMCAB0 = TsigFrame.getCurve("TsigModel_Norm[Tsig]_Comp[Tsigm]")
        curveMC.SetFillColor(ROOT.kWhite)
        curveMCB0.SetFillColor(ROOT.kWhite)
        curveMCAB0.SetFillColor(ROOT.kWhite)

        lMC = ROOT.TLegend(0.73, 0.60, 0.89, 0.89)
        lMC.AddEntry(curveMC, "Both")
        lMC.AddEntry(curveMCAB0, '#bar{#it{B}}^{0}')
        lMC.AddEntry(curveMCB0, '#it{B}^{0}')

        lMC.SetTextSize(0.065)
        lMC.Draw()

        fitResSigNeg.Print("v")
        c2.SaveAs("./" + 'B0_JPsiKs_TsigFittedOnGenMCNegativeDeltaT.pdf')
        c2.Clear()

        dotsMC = TsigFrame.getHist("h_fitDataSig")
        dotsMCB0 = TsigFrame.getHist("data_histB0sig")
        dotsMCAB0 = TsigFrame.getHist("data_histAB0sig")

        plotWithAsymmetry(
            TsigFrame,
            self.TsigPosNeg,
            dotsMC,
            dotsMCB0,
            dotsMCAB0,
            curveMC,
            curveMCB0,
            curveMCAB0,
            "[ps]",
            "TsigFittedOnGenMCNegativeDeltaT",
            lMC,
            "",
            labelAsymmetry,
            True)

        # -----   Signal time fit qsig -------

        self.Tau.setConstant(ROOT.kFALSE)

        self.fitDataSig.Print()
        Tsigp = ROOT.RooGenericPdf("Tsigp", "Tsigp", "(exp(-1*abs(Tsig)/Tau)/(Norm*Tau))",
                                   ROOT.RooArgList(self.Norm, self.Tsig, self.Tau, self.DM))
        Tsigm = ROOT.RooGenericPdf("Tsigm", "Tsigm", "(exp(-1*abs(Tsig)/Tau)/(Norm*Tau))",
                                   ROOT.RooArgList(self.Norm, self.Tsig, self.Tau, self.DM))
        TsigModel = ROOT.RooSimultaneous("TsigModel", "TsigModel", self.qsig)
        TsigModel.addPdf(Tsigp, "1")
        TsigModel.addPdf(Tsigm, "-1")
        TsigModel.Print()

        fitResSig = TsigModel.fitTo(
            self.fitDataSig, ROOT.RooFit.Minos(
                ROOT.kFALSE), ROOT.RooFit.Extended(
                ROOT.kFALSE), ROOT.RooFit.NumCPU(1), ROOT.RooFit.Save())
        fitResSig.Print("v")

        TsigFrame = self.Tsig.frame()
        self.fitDataSig.plotOn(TsigFrame)
        self.fitDataSig.plotOn(
            TsigFrame,
            ROOT.RooFit.Cut("qsig==qsig::1"),
            ROOT.RooFit.MarkerColor(
                ROOT.kRed),
            ROOT.RooFit.Name("data_histB0sig"))
        self.fitDataSig.plotOn(
            TsigFrame,
            ROOT.RooFit.Cut("qsig==1"),
            ROOT.RooFit.MarkerColor(
                ROOT.kBlue + 1),
            ROOT.RooFit.Name("data_histAB0sig"))

        TsigModel.plotOn(
            TsigFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.qsig),
                self.fitDataSig),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlack),
            ROOT.RooFit.LineStyle(3))
        TsigModel.plotOn(
            TsigFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.qsig),
                self.fitDataSig),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kRed),
            ROOT.RooFit.Components("Tsigp"))
        TsigModel.plotOn(
            TsigFrame,
            ROOT.RooFit.ProjWData(
                ROOT.RooArgSet(
                    self.qsig),
                self.fitDataSig),
            ROOT.RooFit.LineWidth(3),
            ROOT.RooFit.LineColor(
                ROOT.kBlue + 1),
            ROOT.RooFit.LineStyle(7),
            ROOT.RooFit.Components("Tsigm"))
        TsigFrame.SetTitle("")
        TsigFrame.GetXaxis().SetTitle("#it{t}_{sig}^{gen} [ps]")
        TsigFrame.GetXaxis().SetTitleSize(0.05)
        TsigFrame.GetXaxis().SetLabelSize(0.045)
        TsigFrame.GetYaxis().SetTitleSize(0.05)
        TsigFrame.GetYaxis().SetTitleOffset(1.5)
        TsigFrame.GetYaxis().SetLabelSize(0.045)

        c3 = ROOT.TCanvas("c2", "c2", 1400, 1100)
        c3.cd()

        self.Tsig.Print()

        Pad = ROOT.TPad("p1", "p1", 0, 0, 1, 1, 0, 0, 0)
        Pad.SetLeftMargin(0.15)
        Pad.SetBottomMargin(0.15)
        Pad.Draw()
        Pad.cd()
        TsigFrame.Draw()

        curveMC = TsigFrame.getCurve("TsigModel_Norm[Tsig]")
        curveMCB0 = TsigFrame.getCurve("TsigModel_Norm[Tsig]_Comp[Tsigp]")
        curveMCAB0 = TsigFrame.getCurve("TsigModel_Norm[Tsig]_Comp[Tsigm]")
        curveMC.SetFillColor(ROOT.kWhite)
        curveMCB0.SetFillColor(ROOT.kWhite)
        curveMCAB0.SetFillColor(ROOT.kWhite)

        lMC = ROOT.TLegend(0.73, 0.60, 0.89, 0.89)
        lMC.AddEntry(curveMC, "Both")
        lMC.AddEntry(curveMCAB0, '#bar{#it{B}}^{0}')
        lMC.AddEntry(curveMCB0, '#it{B}^{0}')

        lMC.SetTextSize(0.065)
        lMC.Draw()

        fitResSig.Print("v")
        c3.SaveAs("./" + 'B0_JPsiKs_TsigQsigFittedOnGenMC.pdf')
        c3.Clear()

        B0tagPercentage = self.B0sInTagSide / (self.B0barsInTagSide + self.B0sInTagSide) * 100
        B0tagbarsPercentage = self.B0barsInTagSide / (self.B0barsInTagSide + self.B0sInTagSide) * 100
        Diff = B0tagbarsPercentage - B0tagPercentage
        print("B0 Tag Flavor: ")
        print(
            "B0s = ",
            " %0.4f" %
            B0tagPercentage,
            "% B0bars = ",
            " %0.4f" %
            B0tagbarsPercentage,
            "% Diff = ",
            " %0.2f" %
            Diff,
            "%")
        B0signalPercentage = self.B0sInSignalSide / (self.B0barsInSignalSide + self.B0sInSignalSide) * 100
        B0signalbarsPercentage = self.B0barsInSignalSide / (self.B0barsInSignalSide + self.B0sInSignalSide) * 100
        Diff = B0signalbarsPercentage - B0signalPercentage
        print("B0 Signal Flavor: ")
        print(
            "B0s = ",
            " %0.4f" %
            B0signalPercentage,
            "% B0bars = ",
            " %0.4f" %
            B0signalbarsPercentage,
            "% Diff = ",
            " %0.2f" %
            Diff,
            "%")
        print("Total amount of events = ", self.nentries)


fitDeltaTModule = fitDeltaT()
validation_path.add_module(fitDeltaTModule)

b2.process(validation_path)
print(b2.statistics)
