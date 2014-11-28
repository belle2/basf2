#!/usr/bin/env python
# -*- coding: utf-8 -*-

import ROOT


def makeMbcPlot(fileName, outputFileName):
    """
    Using the TNTuple in 'fileName', save M_bc plot in 'outputFileName'.
    Shows effect of different cuts on SignalProbability, plus signal distribution.
    """
    ntupleFile = ROOT.TFile(fileName)
    ntupleName = 'variables'

    testTree = ntupleFile.Get(ntupleName)
    if testTree.GetEntries() == 0:
        raise RuntimeError('Couldn\'t find TNtuple "' + ntupleName + '" in file ' + ntupleFile)

    plotTitle = 'Mbc plot'
    ROOT.gStyle.SetOptStat(0)
    canvas = ROOT.TCanvas(plotTitle, plotTitle, 600, 400)
    canvas.cd()

    testTree.SetLineColor(ROOT.kBlack)
    testTree.Draw('Mbc', 'Mbc > 5.23', '')
    testTree.SetLineStyle(ROOT.kDotted)
    testTree.Draw('Mbc', '!isSignal', 'same')
    color = ROOT.kRed + 4
    for cut in [0.0001, 0.001, 0.01, 0.1, 0.5]:
        testTree.SetLineColor(int(color))
        testTree.SetLineStyle(ROOT.kSolid)
        testTree.Draw('Mbc', 'extraInfoSignalProbability > ' + str(cut), 'same')

        testTree.SetLineStyle(ROOT.kDotted)
        testTree.Draw('Mbc', 'extraInfoSignalProbability > ' + str(cut) + ' && !isSignal', 'same')
        color -= 1

    l = canvas.GetListOfPrimitives()
    for i in range(l.GetEntries()):
        hist = l[i]
        if isinstance(hist, ROOT.TH1F):
            hist.GetXaxis().SetRangeUser(5.24, 5.29)
            break

    canvas.BuildLegend(0.1, 0.65, 0.6, 0.9)

    canvas.SaveAs(outputFileName)
