#!/usr/bin/env python
# -*- coding: utf-8 -*-

import ROOT
import math


def efficiencyError(k, n):
    """
    for an efficiency eps = k/n, this function calculates the
    standard deviation according to http://arxiv.org/abs/physics/0701199 .
    """

    variance = (k + 1) * (k + 2) / ((n + 2) * (n + 3)) - (k + 1) ** 2 / ((n + 2) ** 2)
    return math.sqrt(variance)


def makeDiagPlot(outputPrefix, tmvaFile, methodName):
    """
    creates a purity vs. output plot for method methodName in tmvaFile,
    save to file outputPrefix + '_diag_' + methodName + '.pdf'.
    """
    testTree = tmvaFile.Get('TestTree')
    if testTree.GetEntries() == 0:
        raise RuntimeError('TestTree is empty')

    nbins = 100
    varPrefix = ''  # there's also a prob_MethodName variable, but not sure what it is. it definitely looks odd.
    probabilityVar = varPrefix + methodName
    bgHist = ROOT.TH1F('background' + probabilityVar, 'background', nbins,
                       0.0, 1.0)
    testTree.Project('background' + probabilityVar, probabilityVar,
                     'classID == 0')
    signalHist = ROOT.TH1F('signal' + probabilityVar, 'signal', nbins, 0.0,
                           1.0)
    testTree.Project('signal' + probabilityVar, probabilityVar, 'classID == 1')

    import array

    x = array.array('d')
    y = array.array('d')
    xerr = array.array('d')
    yerr = array.array('d')

    for i in range(1, nbins + 1):  # no under/overflow bins
        nSig = 1.0 * signalHist.GetBinContent(i)
        nBg = 1.0 * bgHist.GetBinContent(i)

        try:
            purity = nSig / (nSig + nBg)
            purityErr = efficiencyError(nSig, nSig + nBg)
        except ZeroDivisionError:
            purity = 0
            purityErr = 0
        binCenter = signalHist.GetXaxis().GetBinCenter(i)
        x.append(binCenter)
        y.append(purity)
        xerr.append(signalHist.GetXaxis().GetBinWidth(i) / 2)
        yerr.append(purityErr)

    purityPerBin = ROOT.TGraphErrors(len(x), x, y, xerr, yerr)

    plotTitle = 'Diagonal plot for ' + methodName
    canvas = ROOT.TCanvas(plotTitle + outputPrefix, plotTitle, 600, 400)
    canvas.cd()

    purityPerBin.SetTitle(';' + probabilityVar + ' output;'
                          + 'purity per bin')
    purityPerBin.GetXaxis().SetRangeUser(0.0, 1.0)
    purityPerBin.GetYaxis().SetRangeUser(0.0, 1.0)
    purityPerBin.Draw('APZ')
    diagonal = ROOT.TLine(0.0, 1.0, 1.0, 0.0)
    diagonal.SetLineColor(ROOT.kAzure)
    diagonal.Draw()
    canvas.SaveAs(outputPrefix + '_diag_' + methodName + '.pdf')


def makeDiagPlots(fileName, outputPrefix):
    """
    saves purity vs. output plots for all methods in TMVA output file 'fileName'.
    outputPrefix can be used to distinguish them from plots from other files.
    """
    tmvaFile = ROOT.TFile(fileName)

    # which methods were trained?
    try:
        methods = tmvaFile.Get('Method_Plugins')
    except:
        raise RuntimeError('Problem opening file ' + fileName)

    for m in methods.GetListOfKeys():
        makeDiagPlot(outputPrefix, tmvaFile, m.GetName())
