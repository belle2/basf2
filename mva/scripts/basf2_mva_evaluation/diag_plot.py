#!/usr/bin/env python
# -*- coding: utf-8 -*-

import ROOT

import array
import math
import basf2_mva_evaluation.utility as utility


def _from_hists(signalHist, bckgrdHist):
    assert signalHist.GetNbinsX() == bckgrdHist.GetNbinsX()
    nbins = signalHist.GetNbinsX()

    x = array.array('d')
    y = array.array('d')
    xerr = array.array('d')
    yerr = array.array('d')

    for i in range(1, nbins + 1):  # no under/overflow bins
        nSig = 1.0 * signalHist.GetBinContent(i)
        nBkg = 1.0 * bckgrdHist.GetBinContent(i)
        binCenter = signalHist.GetXaxis().GetBinCenter(i)
        x.append(binCenter)
        y.append(utility.purity(nSig, nBkg))
        xerr.append(signalHist.GetXaxis().GetBinWidth(i) / 2)
        yerr.append(utility.purityError(nSig, nBkg))

    purityPerBin = ROOT.TGraphErrors(len(x), x, y, xerr, yerr)
    purityPerBin.ROOT_OBJECT_OWNERSHIP_WORKAROUND = purityPerBin
    return purityPerBin


def _from_ntuple(ntuple, probability, truth, nbins=100):
    bckgrdHist = ROOT.TH1D('background' + probability, 'background', nbins, 0.0, 1.0)
    signalHist = ROOT.TH1D('signal' + probability, 'signal', nbins, 0.0, 1.0)
    ntuple.Project('background' + probability, probability, '!' + truth)
    ntuple.Project('signal' + probability, probability, truth)
    return _from_hists(signalHist, bckgrdHist)


def from_file(rootfile, probabilities, truths, labels, outputfilename, nbins=100):
    ntuple = rootfile.Get("variables")
    canvas = ROOT.TCanvas("canvas", "Diagonal plot", 1600, 1200)
    canvas.cd()
    legend = ROOT.TLegend(0.1, 0.8, 0.48, 0.9)
    legend.SetBorderSize(0)
    legend.SetFillColor(0)
    legend.SetFillStyle(0)
    legend.SetTextFont(42)
    legend.SetTextSize(0.035)

    mg = ROOT.TMultiGraph()
    color = 1
    for i, (probability, truth, label) in enumerate(zip(probabilities, truths, labels)):
        purityPerBin = _from_ntuple(ntuple, probability, truth, nbins)
        purityPerBin.SetLineColor(color)
        purityPerBin.SetLineWidth(2)

        color += 1
        if color in [5, 10, 11]:
            color += 1

        mg.Add(purityPerBin)
        legend.AddEntry(purityPerBin, label, "lep")

    mg.Draw('APZ')
    mg.SetTitle(";Classifier Output;Purity Per Bin")
    mg.GetXaxis().SetRangeUser(0.0, 1.0)
    mg.GetYaxis().SetRangeUser(0.0, 1.0)
    diagonal = ROOT.TLine(0.0, 0.0, 1.0, 1.0)
    diagonal.SetLineColor(ROOT.kAzure)
    diagonal.SetLineWidth(3)
    diagonal.Draw()
    legend.Draw()

    canvas.SaveAs(outputfilename)
