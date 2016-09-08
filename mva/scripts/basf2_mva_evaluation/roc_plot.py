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

    nSignalTotal = signalHist.Integral(0, nbins + 1)
    for cutBin in range(nbins + 1):
        nSignal = signalHist.Integral(cutBin, nbins + 1)
        nBckgrd = bckgrdHist.Integral(cutBin, nbins + 1)

        x.append(utility.efficiency(nSignal, nSignalTotal) * 100)
        xerr.append(utility.efficiencyError(nSignal, nSignalTotal) * 100)
        y.append(utility.purity(nSignal, nBckgrd) * 100)
        yerr.append(utility.purityError(nSignal, nBckgrd) * 100)

    rocgraph = ROOT.TGraphErrors(len(x), x, y, xerr, yerr)
    rocgraph.ROOT_OBJECT_OWNERSHIP_WORKAROUND = rocgraph
    return rocgraph


def _from_ntuple(ntuple, probability, truth, nbins=100):
    bckgrdHist = ROOT.TH1D('ROCbackground', 'background', nbins, 0.0, 1.0)
    signalHist = ROOT.TH1D('ROCsignal', 'signal', nbins, 0.0, 1.0)
    ntuple.Project('ROCbackground', probability, '!' + truth)
    ntuple.Project('ROCsignal', probability, truth)
    return _from_hists(signalHist, bckgrdHist)


def from_file(rootfile, probabilities, truths, labels, outputfilename, nbins=100):
    ntuple = rootfile.Get("variables")

    canvas = ROOT.TCanvas("canvas", "ROC curve", 1600, 1200)
    canvas.cd()

    legend = ROOT.TLegend(0.1, 0.1, 0.48, 0.2)
    legend.SetBorderSize(0)
    legend.SetFillColor(0)
    legend.SetFillStyle(0)
    legend.SetTextFont(42)
    legend.SetTextSize(0.035)

    mg = ROOT.TMultiGraph()
    color = 1
    for i, (probability, truth, label) in enumerate(zip(probabilities, truths, labels)):
        rocgraph = _from_ntuple(ntuple, probability, truth, nbins)
        rocgraph.SetLineColor(color)
        rocgraph.SetLineWidth(3)

        color += 1
        if color in [5, 10, 11]:
            color += 1

        mg.Add(rocgraph)
        legend.AddEntry(rocgraph, label, "lep")

    mg.Draw('ALPZ')
    mg.SetTitle(";Purity (%);Efficiency (%)")
    legend.Draw()

    canvas.SaveAs(outputfilename)
