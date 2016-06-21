#!/usr/bin/env python
# -*- coding: utf-8 -*-

import ROOT
from ROOT import gSystem
gSystem.Load('libanalysis.so')

import array
import math


def purity(nSig, nBg):
    if nSig == 0:
        return 0.0
    if nSig + nBg == 0:
        return 0.0
    return nSig / float(nSig + nBg)


def efficiency(nSig, nTrueSig):
    if nSig == 0:
        return 0.0
    if nTrueSig == 0:
        return float('inf')
    return nSig / float(nTrueSig)


def efficiencyError(nSig, nTrueSig):
    """
    for an efficiency eps = nSig/nTrueSig, this function calculates the
    standard deviation according to http://arxiv.org/abs/physics/0701199 .
    """
    if nTrueSig == 0:
        return float('inf')

    k = float(nSig)
    n = float(nTrueSig)
    variance = (k + 1) * (k + 2) / ((n + 2) * (n + 3)) - (k + 1) ** 2 / ((n + 2) ** 2)
    if variance <= 0:
        return 0.0
    return math.sqrt(variance)


def purityError(nSig, nBg):
    nTot = nSig + nBg
    if nTot == 0:
        return 0.0
    return efficiencyError(nSig, nTot)


def ROC(rootfilename, outputfilename, probabilities, truths, nbins=100):

    ROOT.gROOT.SetBatch(True)
    rootfile = ROOT.TFile(rootfilename, "UPDATE")
    ntuple = rootfile.Get("variables")

    canvas = ROOT.TCanvas(outputfilename, "ROC curve", 1600, 1200)
    canvas.cd()

    for i, (probability, truth) in enumerate(zip(probabilities, truths)):
        bckgrdHist = ROOT.TH1D('ROCbackground', 'background', nbins, 0.0, 1.0)
        signalHist = ROOT.TH1D('ROCsignal', 'signal', nbins, 0.0, 1.0)

        ntuple.Project('ROCbackground', probability, '!' + truth)
        ntuple.Project('ROCsignal', probability, truth)

        x = array.array('d')
        y = array.array('d')
        xerr = array.array('d')
        yerr = array.array('d')

        nSignalTotal = signalHist.Integral(0, nbins + 1)
        for cutBin in range(nbins + 1):
            nSignal = signalHist.Integral(cutBin, nbins + 1)
            nBckgrd = bckgrdHist.Integral(cutBin, nbins + 1)

            eff = efficiency(nSignal, nSignalTotal)
            effErr = efficiencyError(nSignal, nSignalTotal)
            pur = purity(nSignal, nBckgrd)
            purErr = purityError(nSignal, nBckgrd)

            x.append(100 * pur)
            y.append(100 * eff)
            xerr.append(100 * purErr)
            yerr.append(100 * effErr)

        rocgraph = ROOT.TGraphErrors(len(x), x, y, xerr, yerr)
        rocgraph.SetLineColor(ROOT.kBlue - 2 - i)
        rocgraph.SetTitle(';purity (%);efficiency (%)')
        rocgraph.GetXaxis().SetTitleSize(0.05)
        rocgraph.GetXaxis().SetLabelSize(0.05)
        rocgraph.GetYaxis().SetTitleSize(0.05)
        rocgraph.GetYaxis().SetLabelSize(0.05)
        rocgraph.Draw('ALPZ')

    canvas.SaveAs(outputfilename)


def Diagonal(rootfilename, outputfilename, probabilities, truths, nbins=100):
    ROOT.gROOT.SetBatch(True)
    rootfile = ROOT.TFile(rootfilename, "UPDATE")
    ntuple = rootfile.Get("variables")

    canvas = ROOT.TCanvas(outputfilename, "Diagonal plot", 1600, 1200)
    canvas.cd()

    for i, (probability, truth) in enumerate(zip(probabilities, truths)):
        bgHist = ROOT.TH1D('background' + probability, 'background', nbins, 0.0, 1.0)
        signalHist = ROOT.TH1D('signal' + probability, 'signal', nbins, 0.0, 1.0)

        ntuple.Project('background' + probability, probability, '!' + truth)
        ntuple.Project('signal' + probability, probability, truth)

        x = array.array('d')
        y = array.array('d')
        xerr = array.array('d')
        yerr = array.array('d')

        for i in range(1, nbins + 1):  # no under/overflow bins
            nSig = 1.0 * signalHist.GetBinContent(i)
            nBg = 1.0 * bgHist.GetBinContent(i)
            binCenter = signalHist.GetXaxis().GetBinCenter(i)
            x.append(binCenter)
            y.append(purity(nSig, nBg))
            xerr.append(signalHist.GetXaxis().GetBinWidth(i) / 2)
            yerr.append(purityError(nSig, nBg))

        purityPerBin = ROOT.TGraphErrors(len(x), x, y, xerr, yerr)

        plotLabel = ';classifier output;purity per bin'
        purityPerBin.SetTitle(plotLabel)
        purityPerBin.GetXaxis().SetRangeUser(0.0, 1.0)
        purityPerBin.GetYaxis().SetRangeUser(0.0, 1.0)
        purityPerBin.GetXaxis().SetTitleSize(0.05)
        purityPerBin.GetXaxis().SetLabelSize(0.05)
        purityPerBin.GetYaxis().SetTitleSize(0.05)
        purityPerBin.GetYaxis().SetLabelSize(0.05)
        purityPerBin.Draw('APZ')

    diagonal = ROOT.TLine(0.0, 0.0, 1.0, 1.0)
    diagonal.SetLineColor(ROOT.kAzure)
    diagonal.SetLineWidth(2)
    diagonal.Draw()

    canvas.SaveAs(outputfilename)


def Distribution(rootfilename, outputfilename, probabilities, truths, nbins=100):
    ROOT.gROOT.SetBatch(True)
    rootfile = ROOT.TFile(rootfilename, "UPDATE")
    ntuple = rootfile.Get("variables")

    canvas = ROOT.TCanvas(outputfilename, "Distribution plot", 1600, 1200)
    canvas.cd()

    for i, (probability, truth) in enumerate(zip(probabilities, truths)):
        bgHist = ROOT.TH1D('background' + probability, 'background', nbins, 0.0, 1.0)
        signalHist = ROOT.TH1D('signal' + probability, 'signal', nbins, 0.0, 1.0)

        ntuple.Project('background' + probability, probability, '!' + truth)
        ntuple.Project('signal' + probability, probability, truth)

        signalHist.Draw()
        bgHist.Draw("SAME")

    canvas.SaveAs(outputfilename)
