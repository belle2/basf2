#!/usr/bin/env python
# -*- coding: utf-8 -*-

##################################################################
#                                                                #
#  script for analyzing the output of the TrackFitCheckerModule  #
#                                                                #
#  written by Johannes Rauch, Technische Universitaet Muenchen   #
#  j.rauch@tum.de                                                #
#                                                                #
##################################################################

# median and MAD can be slightly inaccurate since they are calculated from binned data!
# However, errors of these values should in general be less than 1E-2

import ROOT
import glob
import math
import sys
import os
import array
from ROOT import std, TCut, TMath

# default arguments
infile = ''
indir = os.getcwd()
ofile = 'TrackFitCheckerPlots.root'
draw = False
coarse = 1.

# argument parsing
options = ['-f', '-d', '-of', '-draw', '-coarse']
opts = set(options)


def getArgsToNextOpt(stringlist, index):
    id = index + 1
    args = []
    while id < len(stringlist):
        if stringlist[id] in options:
            return args[0]
        args.append(stringlist[id])
        id += 1
    return args[0]


check = 0
for iarg in range(len(sys.argv)):
    arg = sys.argv[iarg]
    if arg == '-f':  # input file
        infile = str(getArgsToNextOpt(sys.argv, iarg))
        check += 1
    if arg == '-d':  # input directory, collect all root files in directory
        indir = str(getArgsToNextOpt(sys.argv, iarg))
        check += 1
    if arg == '-of':  # output root file
        ofile = getArgsToNextOpt(sys.argv, iarg)
    if arg == '-draw':  # draw bin histograms during analysis
        draw = True
    if arg == '-coarse':  # more coarse binning of 2D histogramms (divide by ...)
        coarse = float(getArgsToNextOpt(sys.argv, iarg))

if check > 1:
    print 'Error: use either -d (input directory) or -f (input file) argument, but not both!'
    exit(1)

# input chain
chain = ROOT.TChain('m_statDataTreePtr')

if len(infile) > 1:
    print 'input file: ', infile
    chain.Add(infile)
else:
    infiles = glob.glob(indir + '/*.root')
    if len(infiles) < 1:
        print 'Error: no *.root files in ', indir
        exit(1)
    print 'Input files: ', infiles
    chain.Add(indir + '/*.root')

# check if tree is there
chain.GetEntry()
if chain.GetEntriesFast() < 1:
    print 'Error: m_statDataTreePtr tree has no entries'
    exit(1)

print 'Analysis output file: ', ofile
# chain.Print()

if not draw:
    ROOT.gROOT.SetBatch(True)

# output files
orfile = ROOT.TFile(ofile, 'recreate')
orfileBin = ROOT.TFile(ofile + '.bins.root', 'recreate')

# important numbers -----------------------------------------------------------------------------------------------------------------------------
# 2D Histograms
ptL = 0.2  # lower lower boundary of p_t
ptU = 3.0  # upper boundary of p_t
ptN = int(14 / coarse)  # number of p_t bins
ptS = (ptU - ptL) / ptN  # width of p_t bins

costhL = -0.85  # lower lower boundary of cos(theta)
costhU = 0.95  # upper boundary of cos(theta)
costhN = int(18 / coarse)  # number of cos(theta) bins
costhS = (costhU - costhL) / costhN  # width of cos(theta) bins

pullRange = 0.5  # range for pull mean/median histogramms around 0
RMSRange = 0.5  # range for RMS/MAD histograms around 1
OutlierMax = 50  # range for outlier histograms

pValue_mean_Ideal = 0.5  # ideal value of mean of p-value distribution
pValue_RMS_Ideal = 1. / math.sqrt(12)  # ideal value of RMS of p-value distribution

pValue_mean_Range = 0.25  # range of p-value mean around ideal value
pValue_RMS_Range = 0.125

# 1D Histograms
histRange = 10.0  # range for the analysis histogram
resoScaling = 0.1  # scaling factor for resolution analysis histogram
histBins = 2000  # number of bins for the analysis histogram

MADtoSigma = 1.4826  # sigma = 1.4826*MAD (for gaussian distributions)
outlierCut = 4.  # distance from median in terms of MAD std when to count as outlier

# compute mean, RMS, median, MAD, number of outliers --------------------------------------------------------------------------------------------


def getMedian(histo):
    nbins = histo.GetNbinsX()
    integral = histo.GetIntegral()
    ibin = TMath.BinarySearch(nbins, integral, 0.5)
    while ibin < nbins - 1 and integral[ibin + 1] == 0.5:
        if integral[ibin + 2] == 0.5:
            ibin += 1
        else:
            break
    median = histo.GetBinLowEdge(ibin + 1)
    dint = integral[ibin + 1] - integral[ibin]
    if dint > 0:
        median += histo.GetBinWidth(ibin + 1) * (0.5 - integral[ibin]) / dint
    return median


def robustEstimator(histo):
    retVal = []
    retVal.append(histo.GetMean())  # mean
    retVal.append(histo.GetRMS())  # RMS
    median = getMedian(histo)
    retVal.append(median)  # median
    # MAD (median absolute deviation): create histogram with absolute deviations
    absDev.Reset()
    for i in range(histo.GetNbinsX()):
        absDev.Fill(abs(histo.GetBinCenter(i) - median),
                    histo.GetBinContent(i))
    MADstd = MADtoSigma * getMedian(absDev)
    retVal.append(MADstd)  # MAD std
    # outliers
    outliers = histo.Integral(1, histo.GetXaxis().FindFixBin(median
                              - outlierCut * MADstd))
    outliers += histo.Integral(histo.GetXaxis().FindFixBin(median + outlierCut
                               * MADstd), histo.GetNbinsX())
    retVal.append(outliers)
    return retVal


# set up 2D analysis histos -----------------------------------------------------------------------------------------------------------------------
histos = []
resHistos = []
pullHistos = []

# fitting efficiency
h_efficiency = ROOT.TH2F(
    'h_efficiency',
    'track fitting efficiency',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_efficiency.SetMinimum(0.)
h_efficiency.SetMaximum(1.)
histos.append(h_efficiency)

# p-values
h_pValue_bu_mean = ROOT.TH2F(
    'h_pValue_bu_mean',
    'mean of p-value distribution from the backward filter',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_pValue_bu_mean.SetMinimum(pValue_mean_Ideal - pValue_mean_Range)
h_pValue_bu_mean.SetMaximum(pValue_mean_Ideal + pValue_mean_Range)
histos.append(h_pValue_bu_mean)

h_pValue_bu_RMS = ROOT.TH2F(
    'h_pValue_bu_RMS',
    'RMS of p-value distribution from the backward filter',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_pValue_bu_RMS.SetMinimum(pValue_RMS_Ideal - pValue_RMS_Range)
h_pValue_bu_RMS.SetMaximum(pValue_RMS_Ideal + pValue_RMS_Range)
histos.append(h_pValue_bu_RMS)

h_pValue_fu_mean = ROOT.TH2F(
    'h_pValue_fu_mean',
    'mean of p-value distribution from the forward filter',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_pValue_fu_mean.SetMinimum(pValue_mean_Ideal - pValue_mean_Range)
h_pValue_fu_mean.SetMaximum(pValue_mean_Ideal + pValue_mean_Range)
histos.append(h_pValue_fu_mean)

h_pValue_fu_RMS = ROOT.TH2F(
    'h_pValue_fu_RMS',
    'RMS of p-value distribution from the forward filter',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_pValue_fu_RMS.SetMinimum(pValue_RMS_Ideal - pValue_RMS_Range)
h_pValue_fu_RMS.SetMaximum(pValue_RMS_Ideal + pValue_RMS_Range)
histos.append(h_pValue_fu_RMS)

# resolutions
h_relRes_curvVertex_mean = ROOT.TH2F(
    'h_relRes_curvVertex_mean',
    'mean of relative resolution of curvature at vertex',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_relRes_curvVertex_mean.SetMinimum(-1. * pullRange * resoScaling)
h_relRes_curvVertex_mean.SetMaximum(pullRange * resoScaling)
resHistos.append(h_relRes_curvVertex_mean)

h_relRes_curvVertex_RMS = ROOT.TH2F(
    'h_relRes_curvVertex_RMS',
    'RMS of relative resolution of curvature at vertex',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_relRes_curvVertex_RMS.SetMinimum(0)
h_relRes_curvVertex_RMS.SetMaximum(4. * RMSRange * resoScaling)
resHistos.append(h_relRes_curvVertex_RMS)

h_relRes_curvVertex_median = ROOT.TH2F(
    'h_relRes_curvVertex_median',
    'median of relative resolution of curvature at vertex',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_relRes_curvVertex_median.SetMinimum(-1. * pullRange * resoScaling)
h_relRes_curvVertex_median.SetMaximum(pullRange * resoScaling)
resHistos.append(h_relRes_curvVertex_median)

h_relRes_curvVertex_MAD = ROOT.TH2F(
    'h_relRes_curvVertex_MAD',
    'MAD std of relative resolution of curvature at vertex',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_relRes_curvVertex_MAD.SetMinimum(0)
h_relRes_curvVertex_MAD.SetMaximum(4. * RMSRange * resoScaling)
resHistos.append(h_relRes_curvVertex_MAD)

h_relRes_curvVertex_outliers = ROOT.TH2F(
    'h_relRes_curvVertex_outliers',
    'number of outliers of relative resolution of curvature at vertex',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_relRes_curvVertex_outliers.SetMinimum(0)
h_relRes_curvVertex_outliers.SetMaximum(OutlierMax)
resHistos.append(h_relRes_curvVertex_outliers)

histos.extend(resHistos)

# pull distributions
for var in [
    'x',
    'y',
    'z',
    'px',
    'py',
    'pz',
    ]:
    pullHistos.append(ROOT.TH2F(
        'h_' + var + '_pull_mean',
        'mean of ' + var + ' pull distribution',
        costhN,
        costhL,
        costhU,
        ptN,
        ptL,
        ptU,
        ))
    pullHistos[-1].SetMinimum(-1. * pullRange)
    pullHistos[-1].SetMaximum(pullRange)

    pullHistos.append(ROOT.TH2F(
        'h_' + var + '_pull_RMS',
        'RMS of ' + var + ' pull distribution',
        costhN,
        costhL,
        costhU,
        ptN,
        ptL,
        ptU,
        ))
    pullHistos[-1].SetMinimum(1. - RMSRange)
    pullHistos[-1].SetMaximum(1. + RMSRange)

    pullHistos.append(ROOT.TH2F(
        'h_' + var + '_pull_median',
        'median of ' + var + ' pull distribution',
        costhN,
        costhL,
        costhU,
        ptN,
        ptL,
        ptU,
        ))
    pullHistos[-1].SetMinimum(-1. * pullRange)
    pullHistos[-1].SetMaximum(pullRange)

    pullHistos.append(ROOT.TH2F(
        'h_' + var + '_pull_MAD',
        'MAD std of ' + var + ' pull distribution',
        costhN,
        costhL,
        costhU,
        ptN,
        ptL,
        ptU,
        ))
    pullHistos[-1].SetMinimum(1. - RMSRange)
    pullHistos[-1].SetMaximum(1. + RMSRange)

    pullHistos.append(ROOT.TH2F(
        'h_' + var + '_pull_outliers',
        'number of outliers of ' + var + ' pull distribution',
        costhN,
        costhL,
        costhU,
        ptN,
        ptL,
        ptU,
        ))
    pullHistos[-1].SetMinimum(0.)
    pullHistos[-1].SetMaximum(OutlierMax)

histos.extend(pullHistos)

# set up 1D histos for storing the data for the bins of the 2D analysis histograms
anaHistos = []
anaHistos.append(ROOT.TH1F('histo_p_bu', 'p-value bu', histBins, 0, 1))  # p val
anaHistos.append(ROOT.TH1F('histo_p_fu', 'p-value fu', histBins, 0, 1))  # p val
anaHistos.append(ROOT.TH1F('histoRes', 'relative momentum resolution',
                 histBins, -1. * histRange * resoScaling, histRange
                 * resoScaling))  # res
anaHistos.append(ROOT.TH1F('histoPull0', 'Pulls x', histBins, -1. * histRange,
                 histRange))  # x
anaHistos.append(ROOT.TH1F('histoPull1', 'Pulls y', histBins, -1. * histRange,
                 histRange))  # y
anaHistos.append(ROOT.TH1F('histoPull2', 'Pulls z', histBins, -1. * histRange,
                 histRange))  # z
anaHistos.append(ROOT.TH1F('histoPull3', 'Pulls p_x', histBins, -1.
                 * histRange, histRange))  # p_x
anaHistos.append(ROOT.TH1F('histoPull4', 'Pulls p_y', histBins, -1.
                 * histRange, histRange))  # p_y
anaHistos.append(ROOT.TH1F('histoPull5', 'Pulls p_z', histBins, -1.
                 * histRange, histRange))  # p_z

absDev = ROOT.TH1D('absDev', 'absDev', histBins, 0, histRange)

# set up eventLists for event selection
evtListCosth = ROOT.TEventList('evtListCosth', 'evtListCosth')
evtListBin = ROOT.TEventList('evtListBin', 'evtListBin')
evtListBinFlag = ROOT.TEventList('evtListBinFlag', 'evtListBinFlag')

canvases = []  # canvases for plotting bin-wise data

# loop over chain ------------------------------------------------------------------------------------------------------------------------------
statusFlagCut = ROOT.TCut('genfitStatusFlag==0')

costh = costhL
binX = 1
while costh < costhU - 0.5 * costhS:  # loop over costh
    costhCUT = ROOT.TCut('trueVertexMom.CosTheta()>=' + str(costh)
                         + ' && trueVertexMom.CosTheta()<' + str(costh
                         + costhS))

    # select events in theta range
    evtListCosth.Reset()
    evtListBin.Reset()
    evtListBinFlag.Reset()
    chain.SetEventList(ROOT.NULL)
    chain.Draw('>> evtListCosth', costhCUT)
    chain.SetEventList(evtListCosth)

    print '# events in theta range: ', evtListCosth.GetN()

    binY = 1
    pt = ptL
    while pt < ptU - 0.5 * ptS:  # loop over pt
        ptCUT = ROOT.TCut('trueVertexMom.Perp()>=' + str(pt)
                          + ' && trueVertexMom.Perp()<' + str(pt + ptS))
        binCut = ROOT.TCut(ptCUT.GetTitle() + ' && ' + costhCUT.GetTitle())
        binStFlCut = ROOT.TCut(statusFlagCut.GetTitle() + ' && '
                               + binCut.GetTitle())

        # create canvas for plotting bin-wise data
        canvases.append(ROOT.TCanvas('Bin(' + str(binX) + ',' + str(binY)
                        + ');costh=' + str(costh) + ';pt=' + str(pt), 'Bin('
                        + str(binX) + ',' + str(binY) + '); costh = '
                        + str(costh) + '; pt = ' + str(pt)))
        canvases[-1].Divide(3, 3)

        # further select events in pt range
        evtListBin.Reset()
        evtListBinFlag.Reset()
        chain.SetEventList(evtListCosth)
        chain.Draw('>> evtListBin', binCut)
        chain.SetEventList(evtListBin)

        # further reduce event list to entries with statusFlag==0
        chain.Draw('>> evtListBinFlag', statusFlagCut)
        chain.SetEventList(evtListBinFlag)

        print 'Bin(', binX, binY, '), costh =', costh, '; pt =', pt, \
            '   # events in bin: ', evtListBin.GetN(), \
            '   # fitted events in bin: ', evtListBinFlag.GetN()

        # efficiency
        if evtListBin.GetN() > 0:
            h_efficiency.SetBinContent(binX, binY,
                                       float(evtListBinFlag.GetN())
                                       / float(evtListBin.GetN()))

        # p-values
        canvases[-1].cd(1)
        chain.Draw('pValue_bu >> histo_p_bu')
        if anaHistos[0].GetEntries() > 0:
            h_pValue_bu_mean.SetBinContent(binX, binY, anaHistos[0].GetMean())
            h_pValue_bu_RMS.SetBinContent(binX, binY, anaHistos[0].GetRMS())

        canvases[-1].cd(2)
        chain.Draw('pValue_fu >> histo_p_fu')
        if anaHistos[1].GetEntries() > 0:
            h_pValue_fu_mean.SetBinContent(binX, binY, anaHistos[1].GetMean())
            h_pValue_fu_RMS.SetBinContent(binX, binY, anaHistos[1].GetRMS())

        # resolutions
        iHist = 0
        canvases[-1].cd(3)
        chain.Draw('relRes_curvVertex >> histoRes')
        if anaHistos[2].GetEntries() > 0:
            est = robustEstimator(anaHistos[2])
            for val in est:
                resHistos[iHist].SetBinContent(binX, binY, val)
                iHist += 1

        # pull distributions
        for iVar in range(0, 6):  # x, y, z, px, py, pz
            iHist = 0
            canvases[-1].cd(3 + iVar + 1)
            chain.Draw('pulls_vertexPosMom[' + str(iVar) + '] >> histoPull'
                       + str(iVar))
            if anaHistos[3 + iVar].GetEntries() > 0:
                est = robustEstimator(anaHistos[3 + iVar])
                for val in est:
                    pullHistos[5 * iVar + iHist].SetBinContent(binX, binY, val)
                    iHist += 1

        orfileBin.cd()
        canvases[-1].Write()

        pt += ptS
        binY += 1
    # end loop over pt

    costh += costhS
    binX += 1
# end loop over costh

# store histograms in output file --------------------------------------------------------------------------------------------------------------
print 'Creating output file and storing histograms ...'
orfile.cd()
for h in histos:
    h.SetContour(255)
    h.GetXaxis().SetTitle('Generated cos#theta')
    h.GetYaxis().SetTitle('Generated p_{T}')
    h.Write()

