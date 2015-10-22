#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math
import ROOT
from ROOT import Belle2


def rename_MCCount(name):
    import variables
    old_name = Belle2.Variable.invertMakeROOTCompatible(name)
    return old_name[len('NumberOfMCParticlesInEvent('):-len(")")]


def rename_ListCount(name):
    old_name = Belle2.Variable.invertMakeROOTCompatible(name)
    if ',' in old_name:
        if old_name[-2] == '1':
            return old_name.split(',')[0][len('countInList('):] + '_Signal'
        if old_name[-2] == '0':
            return old_name.split(',')[0][len('countInList('):] + '_Background'
        raise RuntimeError("Given listCount name is not valid " + old_name)
    else:
        return old_name[len('countInList('):-1] + '_All'


def calcSum(hist):
    sum = 0
    for bin in range(hist.GetNbinsX()):
        sum += hist.GetXaxis().GetBinCenter(bin + 1) * hist.GetBinContent(bin + 1)
    return sum


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
    return math.sqrt(variance)


def purityError(nSig, nBg):
    nTot = nSig + nBg
    if nTot == 0:
        return 0.0
    return efficiencyError(nSig, nTot)


def makeROCPlotFromNtuple(fileName, outputFileName, nTrueSignal, targetVar):
    """
    Using the TNTuple in 'fileName', save an efficiency over purity plot in 'outputFileName'.

    @param nTrueSignal number of true signal particles in the sample.
    """
    ROOT.gROOT.SetBatch(True)
    ntupleFile = ROOT.TFile(fileName)
    ntupleName = 'variables'

    tree = ntupleFile.Get(ntupleName)
    if tree.GetEntries() == 0:
        raise RuntimeError('Couldn\'t find TNtuple "' + ntupleName + '" in file ' + ntupleFile)

    plotTitle = 'ROC curve'
    canvas = ROOT.TCanvas(outputFileName, plotTitle, 600, 400)
    canvas.cd()

    nbins = 100
    import array

    bgHist = ROOT.TH1D('ROCbackground', 'background', nbins, 0.0, 1.0)
    signalHist = ROOT.TH1D('ROCsignal', 'signal', nbins, 0.0, 1.0)

    probabilityVar = 'extraInfo__boSignalProbability__bc'
    tree.Project('ROCbackground', probabilityVar, '!' + targetVar)
    tree.Project('ROCsignal', probabilityVar, targetVar)

    x = array.array('d')
    y = array.array('d')
    xerr = array.array('d')
    yerr = array.array('d')

    for cutBin in range(nbins + 1):
        nSig = signalHist.Integral(cutBin, nbins + 1)
        nBg = bgHist.Integral(cutBin, nbins + 1)

        x.append(100 * purity(nSig, nBg))
        y.append(100 * efficiency(nSig, nTrueSignal))
        xerr.append(100 * purityError(nSig, nBg))
        yerr.append(100 * efficiencyError(nSig, nTrueSignal))

    rocgraph = ROOT.TGraphErrors(len(x), x, y, xerr, yerr)
    rocgraph.SetLineColor(ROOT.kBlue - 2)
    rocgraph.SetTitle(';purity (%);efficiency (%)')
    rocgraph.GetXaxis().SetTitleSize(0.05)
    rocgraph.GetXaxis().SetLabelSize(0.05)
    rocgraph.GetYaxis().SetTitleSize(0.05)
    rocgraph.GetYaxis().SetLabelSize(0.05)
    rocgraph.Draw('ALPZ')

    canvas.SaveAs(outputFileName)
