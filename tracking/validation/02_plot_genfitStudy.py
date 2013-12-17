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

##################################################################
coarse = 1

##################################################################

# median and MAD can be slightly inaccurate since they are calculated from binned data!
# However, errors of these values should in general be less than 1E-2

import ROOT
import glob
import math
import sys
import os
import array
import time
from ROOT import std, TCut, TMath, TNamed

ROOT.gROOT.SetBatch(True)

# get input file: newest file that matches the output name of the steering script
filelist = glob.glob('../genfit_*StatData.root')
filelist = filter(lambda x: not os.path.isdir(x), filelist)
filelist.sort(key=lambda f: os.path.getmtime(f))  # sort by modification date
infile = filelist[-1]

outFileName = 'TrackFitCheckerPlots_' + infile[infile.find('genfit_')
    + 7:infile.find('StatData.root')]
outFileNameBins = outFileName + '_Bins.root'
outFileName += '.root'
print 'Analysis input file: ', infile
print 'Analysis output file: ', outFileName
print 'Analysis output file with histograms for each bin: ', outFileNameBins

# check if FULL test
full = False
if '_FULL' in infile:
    full = True

# input chain
chain = ROOT.TChain('m_statDataTreePtr')
chain.Add(infile)

# check if tree is there
chain.GetEntry()
if chain.GetEntriesFast() < 1:
    print 'Error: m_statDataTreePtr tree has no entries'
    sys.exit(1)

# output files
outFile = ROOT.TFile(outFileName, 'recreate')
outFile.cd()

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
histBins = 250  # number of bins for the analysis histogram
if full:
    histBins = 1000
iterRange = 15  # number of bins for the number of iterations histogram

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
h_fitted = ROOT.TH2F(
    'h_fitted',
    'tracks fitted',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_fitted.SetMinimum(0.)
h_fitted.SetMaximum(1.)
description = 'Fraction of tracks which could be fitted.'
check = 'Should be close to 1.'
h_fitted.GetListOfFunctions().Add(TNamed('Description', description))
h_fitted.GetListOfFunctions().Add(TNamed('Check', check))
histos.append(h_fitted)

# fitting efficiency
h_fittedConverged = ROOT.TH2F(
    'h_fittedConverged',
    'tracks fitted and converged',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_fittedConverged.SetMinimum(0.)
h_fittedConverged.SetMaximum(1.)
description = \
    'Fraction of track which could be fitted and where the fit is converged.'
check = 'Should be close to 1.'
h_fittedConverged.GetListOfFunctions().Add(TNamed('Description', description))
h_fittedConverged.GetListOfFunctions().Add(TNamed('Check', check))
histos.append(h_fittedConverged)

h_NumIterations = ROOT.TH2F(
    'h_NumIterations',
    'number of iterations',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_NumIterations.SetMinimum(0.)
h_NumIterations.SetMaximum(20.)
histos.append(h_NumIterations)
description = 'Number of iterations.'
# check = ''
h_NumIterations.GetListOfFunctions().Add(TNamed('Description', description))
# h_NumIterations.GetListOfFunctions().Add(TNamed('Check', check))

# p-values
h_pValue_b_mean = ROOT.TH2F(
    'h_pValue_b_mean',
    'mean of p-value distribution from the backward filter',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_pValue_b_mean.SetMinimum(pValue_mean_Ideal - pValue_mean_Range)
h_pValue_b_mean.SetMaximum(pValue_mean_Ideal + pValue_mean_Range)
description = 'Mean of p-Value distributions of backward fit.'
check = 'Should be 0.5.'
h_pValue_b_mean.GetListOfFunctions().Add(TNamed('Description', description))
h_pValue_b_mean.GetListOfFunctions().Add(TNamed('Check', check))
histos.append(h_pValue_b_mean)

h_pValue_b_RMS = ROOT.TH2F(
    'h_pValue_b_RMS',
    'RMS of p-value distribution from the backward filter',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_pValue_b_RMS.SetMinimum(pValue_RMS_Ideal - pValue_RMS_Range)
h_pValue_b_RMS.SetMaximum(pValue_RMS_Ideal + pValue_RMS_Range)
description = 'RMS of p-Value distributions of backward fit.'
check = 'Should be ' + str(pValue_RMS_Ideal) + '.'
h_pValue_b_RMS.GetListOfFunctions().Add(TNamed('Description', description))
h_pValue_b_RMS.GetListOfFunctions().Add(TNamed('Check', check))
histos.append(h_pValue_b_RMS)

h_pValue_f_mean = ROOT.TH2F(
    'h_pValue_f_mean',
    'mean of p-value distribution from the forward filter',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_pValue_f_mean.SetMinimum(pValue_mean_Ideal - pValue_mean_Range)
h_pValue_f_mean.SetMaximum(pValue_mean_Ideal + pValue_mean_Range)
description = 'Mean of p-Value distributions of forward fit.'
check = 'Should be 0.5.'
h_pValue_f_mean.GetListOfFunctions().Add(TNamed('Description', description))
h_pValue_f_mean.GetListOfFunctions().Add(TNamed('Check', check))
histos.append(h_pValue_f_mean)

h_pValue_f_RMS = ROOT.TH2F(
    'h_pValue_f_RMS',
    'RMS of p-value distribution from the forward filter',
    costhN,
    costhL,
    costhU,
    ptN,
    ptL,
    ptU,
    )
h_pValue_f_RMS.SetMinimum(pValue_RMS_Ideal - pValue_RMS_Range)
h_pValue_f_RMS.SetMaximum(pValue_RMS_Ideal + pValue_RMS_Range)
description = 'RMS of p-Value distributions of forward fit.'
check = 'Should be ' + str(pValue_RMS_Ideal) + '.'
h_pValue_f_RMS.GetListOfFunctions().Add(TNamed('Description', description))
h_pValue_f_RMS.GetListOfFunctions().Add(TNamed('Check', check))
histos.append(h_pValue_f_RMS)

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
    description = 'mean of ' + var + ' pull distribution.'
    check = 'Should be 0.'
    pullHistos[-1].GetListOfFunctions().Add(TNamed('Description', description))
    pullHistos[-1].GetListOfFunctions().Add(TNamed('Check', check))
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
    description = 'RMS of ' + var + ' pull distribution.'
    check = 'Should be 1.'
    pullHistos[-1].GetListOfFunctions().Add(TNamed('Description', description))
    pullHistos[-1].GetListOfFunctions().Add(TNamed('Check', check))
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
    description = 'Median of ' + var + ' pull distribution.'
    check = 'Should be 0.'
    pullHistos[-1].GetListOfFunctions().Add(TNamed('Description', description))
    pullHistos[-1].GetListOfFunctions().Add(TNamed('Check', check))
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
    description = 'Scaled median absolute deviation of ' + var \
        + ' pull distribution.'
    check = 'Should be 1.'
    pullHistos[-1].GetListOfFunctions().Add(TNamed('Description', description))
    pullHistos[-1].GetListOfFunctions().Add(TNamed('Check', check))
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
    description = 'number of outliers of ' + var \
        + ' pull distribution. It is the number of entries which are outside of ' \
        + str(outlierCut) + ' times the scaled MAD of the distribution.'
    check = 'Should be small.'
    pullHistos[-1].GetListOfFunctions().Add(TNamed('Description', description))
    pullHistos[-1].GetListOfFunctions().Add(TNamed('Check', check))
    pullHistos[-1].SetMinimum(0.)
    pullHistos[-1].SetMaximum(OutlierMax)

histos.extend(pullHistos)

# set up 1D histos for storing the data for the bins of the 2D analysis histograms
anaHistos = []
anaHistos.append(ROOT.TH1F('histo_p_bu', 'p-value bu', histBins, 0, 1))  # p val
description = 'p-Value distribution of backward fit.'
check = 'Should be flat.'
anaHistos[-1].GetListOfFunctions().Add(TNamed('Description', description))
anaHistos[-1].GetListOfFunctions().Add(TNamed('Check', check))

anaHistos.append(ROOT.TH1F('histo_p_fu', 'p-value fu', histBins, 0, 1))  # p val
description = 'p-Value distribution of forward fit.'
check = 'Should be flat.'
anaHistos[-1].GetListOfFunctions().Add(TNamed('Description', description))
anaHistos[-1].GetListOfFunctions().Add(TNamed('Check', check))

anaHistos.append(ROOT.TH1F('histoRes', 'relative momentum resolution',
                 histBins, -1. * histRange * resoScaling, histRange
                 * resoScaling))  # res
anaHistos.append(ROOT.TH1F('histoPull0', 'Pulls x', histBins, -1. * histRange,
                 histRange))  # x
description = 'Pull distribution in x.'
check = 'Should a Gaussian with width 1 centered at 0.'
anaHistos[-1].GetListOfFunctions().Add(TNamed('Description', description))
anaHistos[-1].GetListOfFunctions().Add(TNamed('Check', check))

anaHistos.append(ROOT.TH1F('histoPull1', 'Pulls y', histBins, -1. * histRange,
                 histRange))  # y
description = 'Pull distribution in y.'
anaHistos[-1].GetListOfFunctions().Add(TNamed('Description', description))
anaHistos[-1].GetListOfFunctions().Add(TNamed('Check', check))

anaHistos.append(ROOT.TH1F('histoPull2', 'Pulls z', histBins, -1. * histRange,
                 histRange))  # z
description = 'Pull distribution in z.'
anaHistos[-1].GetListOfFunctions().Add(TNamed('Description', description))
anaHistos[-1].GetListOfFunctions().Add(TNamed('Check', check))

anaHistos.append(ROOT.TH1F('histoPull3', 'Pulls p_x', histBins, -1.
                 * histRange, histRange))  # p_x
description = 'Pull distribution in p_x.'
anaHistos[-1].GetListOfFunctions().Add(TNamed('Description', description))
anaHistos[-1].GetListOfFunctions().Add(TNamed('Check', check))

anaHistos.append(ROOT.TH1F('histoPull4', 'Pulls p_y', histBins, -1.
                 * histRange, histRange))  # p_y
description = 'Pull distribution in p_y.'
anaHistos[-1].GetListOfFunctions().Add(TNamed('Description', description))
anaHistos[-1].GetListOfFunctions().Add(TNamed('Check', check))

anaHistos.append(ROOT.TH1F('histoPull5', 'Pulls p_z', histBins, -1.
                 * histRange, histRange))  # p_z
description = 'Pull distribution in p_z.'
anaHistos[-1].GetListOfFunctions().Add(TNamed('Description', description))
anaHistos[-1].GetListOfFunctions().Add(TNamed('Check', check))

anaHistos.append(ROOT.TH1F('histoNIter', 'number of iterations', iterRange, 0,
                 iterRange))
description = 'Number of iterations.'
anaHistos[-1].GetListOfFunctions().Add(TNamed('Description', description))
# anaHistos[-1].GetListOfFunctions().Add(TNamed('Check', check))

absDev = ROOT.TH1D('absDev', 'absDev', histBins, 0, histRange)

# loop over chain ------------------------------------------------------------------------------------------------------------------------------
if full:
    outFileBins = ROOT.TFile(outFileName + '.bins.root', 'recreate')
    outFile.cd()

    # set up eventLists for event selection
    evtListCosth = ROOT.TEventList('evtListCosth', 'evtListCosth')
    evtListBin = ROOT.TEventList('evtListBin', 'evtListBin')
    evtListBinFitted = ROOT.TEventList('evtListBinFitted', 'evtListBinFitted')
    evtListBinFittedConverged = ROOT.TEventList('evtListBinFittedConverged',
            'evtListBinFittedConverged')

    canvases = []  # canvases for plotting bin-wise data

    fittedCut = ROOT.TCut('fitted==1')
    fittedConvergedCut = ROOT.TCut('fittedConverged==1')

    costh = costhL
    binX = 1
    while costh < costhU - 0.5 * costhS:  # loop over costh
        costhCUT = ROOT.TCut('trueVertexMom.CosTheta()>=' + str(costh)
                             + ' && trueVertexMom.CosTheta()<' + str(costh
                             + costhS))

        # select events in theta range
        evtListCosth.Reset()
        evtListBin.Reset()
        evtListBinFitted.Reset()
        evtListBinFittedConverged.Reset()
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
            binFittedCut = ROOT.TCut(fittedCut.GetTitle() + ' && '
                                     + binCut.GetTitle())

            # create canvas for plotting bin-wise data
            canvases.append(ROOT.TCanvas('Bin(' + str(binX) + ',' + str(binY)
                            + ');costh=' + str(costh) + ';pt=' + str(pt),
                            'Bin(' + str(binX) + ',' + str(binY)
                            + '); costh = ' + str(costh) + '; pt = '
                            + str(pt)))
            canvases[-1].Divide(3, 3)

            # further select events in pt range
            evtListBin.Reset()
            evtListBinFitted.Reset()
            evtListBinFittedConverged.Reset()
            chain.SetEventList(evtListCosth)
            chain.Draw('>> evtListBin', binCut)
            chain.SetEventList(evtListBin)

            # number of iterations
            chain.Draw('numIterations >> histoNIter')
            if anaHistos[9].GetEntries() > 0:
                h_NumIterations.SetBinContent(binX, binY,
                        anaHistos[9].GetMean())

            # further reduce event list to fitted tracks
            chain.Draw('>> evtListBinFitted', fittedCut)
            chain.Draw('>> evtListBinFittedConverged', fittedConvergedCut)
            chain.SetEventList(evtListBinFitted)

            print 'Bin(', binX, binY, '), costh =', costh, '; pt =', pt, \
                '   # events in bin: ', evtListBin.GetN(), \
                '   # fitted events in bin: ', evtListBinFitted.GetN()

            # fitted tracks
            if evtListBin.GetN() > 0:
                h_fitted.SetBinContent(binX, binY,
                                       float(evtListBinFitted.GetN())
                                       / float(evtListBin.GetN()))

            # fitted and converged tracks
            if evtListBin.GetN() > 0:
                h_fittedConverged.SetBinContent(binX, binY,
                        float(evtListBinFittedConverged.GetN())
                        / float(evtListBin.GetN()))

            # from here on, only fitted tracks will be taken into account

            # p-values
            canvases[-1].cd(1)
            chain.Draw('pValue_b >> histo_p_bu')
            if anaHistos[0].GetEntries() > 0:
                h_pValue_b_mean.SetBinContent(binX, binY,
                        anaHistos[0].GetMean())
                h_pValue_b_RMS.SetBinContent(binX, binY, anaHistos[0].GetRMS())

            canvases[-1].cd(2)
            chain.Draw('pValue_f >> histo_p_fu')
            if anaHistos[1].GetEntries() > 0:
                h_pValue_f_mean.SetBinContent(binX, binY,
                        anaHistos[1].GetMean())
                h_pValue_f_RMS.SetBinContent(binX, binY, anaHistos[1].GetRMS())

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
                        pullHistos[5 * iVar + iHist].SetBinContent(binX, binY,
                                val)
                        iHist += 1

            outFileBins.cd()
            canvases[-1].Write()
            outFile.cd()

            pt += ptS
            binY += 1
        # end loop over pt

        costh += costhS
        binX += 1
    # end loop over costh

    # store histograms in output file --------------------------------------------------------------------------------------------------------------
    print 'Creating output file and storing histograms ...'
    outFile.cd()
    ROOT.gROOT.ProcessLine('gStyle.SetPaintTextFormat("1.4f")')
    for h in histos:
        h.SetContour(255)
        h.GetXaxis().SetTitle('Generated cos#theta')
        h.GetYaxis().SetTitle('Generated p_{T}')
        h.SetOption('colz')
        h.Write()
else:

       # full == false

    nt_Fitted = ROOT.TNtuple('nt_Fitted', 'Fitted tracks', 'number')

    nt_FittedConverged = ROOT.TNtuple('nt_FittedConverged',
                                      'Fitted and converged tracks', 'number')

    evtListFitted = ROOT.TEventList('evtListFitted', 'evtListFitted')
    evtListFittedConverged = ROOT.TEventList('evtListFittedConverged',
            'evtListFittedConverged')

    fittedCut = ROOT.TCut('fitted==1')
    fittedConvergedCut = ROOT.TCut('fittedConverged==1')

    chain.Draw('>> evtListFitted', fittedCut)

    fitted = float(evtListFitted.GetN()) / float(chain.GetEntries())
    nt_Fitted.Fill(fitted)

    chain.Draw('>> evtListFittedConverged', fittedConvergedCut)

    fittedConverged = float(evtListFittedConverged.GetN()) \
        / float(chain.GetEntries())
    nt_FittedConverged.Fill(fittedConverged)

    # number of iterations
    chain.Draw('numIterations >> histoNIter')

    chain.SetEventList(evtListFitted)

    # from here on, only fitted tracks will be taken into account

    # p-values
    chain.Draw('pValue_b >> histo_p_bu')
    anaHistos[0].Fit('pol1', 'M')

    chain.Draw('pValue_f >> histo_p_fu')
    anaHistos[1].Fit('pol1', 'M')

    # resolutions
    iHist = 0
    chain.Draw('relRes_curvVertex >> histoRes')
    anaHistos[2].Fit('gaus', 'LM')
    # pull distributions
    for iVar in range(0, 6):  # x, y, z, px, py, pz
        iHist = 0
        chain.Draw('pulls_vertexPosMom[' + str(iVar) + '] >> histoPull'
                   + str(iVar))
        anaHistos[3 + iVar].Fit('gaus', 'LM')

    outFile.cd()
    nt_Fitted.Write()
    nt_FittedConverged.Write()
    for h in anaHistos:
        h.Write()

outFile.Close()

