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

    nbins = 100

    plotTitle = 'Mbc plot'
    canvas = ROOT.TCanvas(plotTitle, plotTitle, 600, 400)
    canvas.cd()

    first = True
    color = ROOT.kBlue
    for cut in [0.0, 0.0001, 0.001, 0.01, 0.1, 0.5]:
        if first:
            option = ''
        else:
            option = 'same'
        color -= 1.5
        testTree.SetLineColor(int(color))
        testTree.Draw('Mbc', 'Mbc > 5.24 && getExtraInfoSignalProbability > ' + str(cut), option)
        first = False

    testTree.SetLineColor(ROOT.kRed)
    testTree.Draw('Mbc', 'Mbc > 5.24 && isSignal > 0.5', 'same')

    canvas.BuildLegend(0.1, 0.65, 0.6, 0.9)

    canvas.SaveAs(outputFileName)
