#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ------------------------------------------------------------------------
# Tools to fit the laser timing distribution and plot the results.
# This code is intended for studying the TOP time resolution
#
# Contributors: Umberto Tamponi (tamponi@to.infn.it)
#
# ------------------------------------------------------------------------


from basf2 import *
import sys
import glob
from ROOT import Belle2
from ROOT import TH1F, TH2F, TF1, TFile, TGraphErrors, TSpectrum, TCanvas, TTree, TMath
import ROOT
import pylab
import numpy
import time
from array import array


def fitLaserResolution(
        mcCorrectionsFile='/group/belle2/group/detector/TOP/calibration/MCreferences/t0MC.root',
        dataFile='',
        outputFile='laserResolutionResults.root',
        pdfType='cb',
        maxPeaks=2,
        saveFits=False,
        includeBackscatter=True,
        useSinglePDF=True,
        useMCPeaks=True):
    """
    Function that implements the fit of the laser time distribution correcting for the propagation time in the prism.
    The inputs to the fitter are hit time distributions per channel, in the form of TH2F.

    Two files containing the containing the hit timing VS channels distribution must be provided:

    * The first file is used to estimate the light propagation correction (aka MC correction),
      the number of peaks and the separation bewteen these peaks.
      Since the correction is the same for all the slots, the corresponding TH2F can have only 512 bins on the x-axis.
      A default file is available, where the hit time is taken from
      the TOPSimHits, and the laser jitter has been set to zero in the simulation.
    * The second file contains the distribution obtained from the data do be used to estimate the resolution.
      In this case the histrogram must have
      (512 x 16) bins on the x-axis, to store the time distribution for each channel of the detector.

    Parameters:
      mcCorrectionsFile (str): File with the TH2F containing the TOPSImHits time distribution.
       Used to calculate the MC corrections
      dataFile (str): File with the TH2F containing the TOPDigit time distribution on which the resolutions
       must be evaluated
      outputFile (str): rootFile containing the outupt trees and plots
      pdfType (str): PDF used in the data fit: cb for a CrystalBall, gaus for a Gaussian or
       gausExpo for an exponentially modified Gaussian
      maxPeaks (int): maximum number of peaks that can be used in the data fit. Note that the
       actual value is decided channel-by-channel
       according to the MC simulation, this parameter is only a maximum cap.
       In the current implementation only 1 or 2 peaks are supported, therefore maxPeaks can only go up to 2
      saveFits (bool): set to True to save the histrogram and the fit for eavey single channel
      includeBackscatter (bool): set to True to add an extra peak in the fit to account for the
       MCP backscattering
      useMCPeaks (bool): ste to True to fix the light path time difference from the MC. Set to False
       to let this parameter floating in the fit
    Returns:
      void
    """

    if maxPeaks > 2 or maxPeaks < 1:
        B2FATAL('Usupported value for the maximum number for peaks (maxPeaks = ' +
                str(maxPeaks) + '). Please set maxPeak o either 1 or 2.')
    if pdfType is not 'cb' and pdfType is not 'gaus' and pdfType is not 'gausExpo':
        B2FATAL(
            'Unknown PDF type ' +
            pdfType +
            ". The possible options are cb for the Crystal Ball,"
            "gaus for the Gaussian and gausExpo for exponentially modified Gaussian")

    # First loops over the MC file to get the time propagation corrections
    # Only the two highest peaks are stored

    # List with the position fo the MC peaks (2 per channel). This is where the
    # MC peak positions are stored to be used later to correct the data
    mcPeaks = [[-1. for second in range(2)] for first in range(512)]

    # Opens the file containg the MC distribution and loads it
    tFileMC = TFile(mcCorrectionsFile)
    histoMC = tFileMC.Get('LaserTimingVSChannelOneSlot')

    # Loop over the channels of a slot (i.e the bins on the x axis of histoMC)
    for kCh in range(512):
        # Gets a one-channel wide slice and adjusts the range
        timeProjectionMC = histoMC.ProjectionY('projectionMC', kCh + 1, kCh + 1)
        timeProjectionMC.GetXaxis().SetRangeUser(0., 1.)
        # Initializes TSpectrum
        spectrum = TSpectrum()
        # By default dentifies a peak only if it is 1-sigma apart from another peak
        # and has an aplitud of at least 10% of the largest peak.
        numPeaks = spectrum.Search(timeProjectionMC, 1., 'nobackground', 0.1)
        # This is a bit ugl way to store the peak locations and amplitudes
        # in a list. At this stage the maxima are no granted to be sorted
        peaksX_ = spectrum.GetPositionX()
        peaksY_ = spectrum.GetPositionY()
        peaksX = [peaksX_[i] for i in range(numPeaks)]
        peaksY = [peaksY_[j] for j in range(numPeaks)]

        # Sorts the peaks starting form the largest one and stores the first
        # two in the mcPeaks matrix
        for iMax in range(numPeaks):
            if iMax < 2:
                maxPosition = int(peaksY.index(max(peaksY)))
                peaksY[maxPosition] = -1.  # get rid of the maximum
                mcPeaks[kCh][iMax] = peaksX[maxPosition]

    tFileMC.Close()

    # End of the MC corrections part

    # Opens the file with the data and loads the timing histogram
    tFileData = TFile(dataFile)
    histoData = tFileData.Get('LaserTimingVSChannel')

    # Prepares the output file and tree
    tFileOut = TFile(outputFile, "RECREATE")
    outTree = TTree("tree", "tree")

    # Slot ID (1-based)
    slotID = array('i', [0])
    # Hardware channle number (0-based)
    hwChannel = array('i', [0])
    # Number of peaks used in the fit
    nPeaks = array('i', [0])
    # Degrees of freedom of the fit
    ndf = array('f', [0])
    # Chi2 of the fit
    chi2 = array('f', [0.])
    # Sigma of the Gaussian core
    sigma = array('f', [0.])
    # Statistical error on sigma
    errSigma = array('f', [0.])
    # Time of the primary peak (maximum of the fit fuction)
    peakTime = array('f', [0.])
    # Statistical error on manPeak
    errPeakTime = array('f', [0.])

    # Time of the spurious peak
    extraTime = array('f', [0.])
    # Statistical error on extraTime
    errExtraTime = array('f', [0.])

    # scale factor for Sigma of the Gaussian core
    sigmaExtra = array('f', [0.])
    # Statistical error on sigma scale factor
    errSigmaExtra = array('f', [0.])

    # Alpha parameter of the CB functions
    alpha = array('f', [0.])
    # Statistical error on alpha
    errAlpha = array('f', [0.])
    # n parameter of the CB functions
    n = array('f', [0.])
    # statistical error on n
    errN = array('f', [0.])
    # lambda parameter of the gauss exponiential
    lam = array('f', [0.])
    # Statistical error on lam
    errLam = array('f', [0.])
    # Normalization of the primary peak
    norm1 = array('f', [0.])
    # Statistical error on norm1
    errNorm1 = array('f', [0.])
    # Normalization of the secondary peak
    norm2 = array('f', [0.])
    # Statistical error on norm2
    errNorm2 = array('f', [0.])

    # Normalization of the spurious peak
    normExtra = array('f', [0.])
    # Statistical error on normExtra
    errNormExtra = array('f', [0.])

    # Separation between primary and secondary peak from the MC
    deltaMC = array('f', [0.])
    # Laser time corrected for the propagation time in the prism
    correctedTime = array('f', [0.])
    # Time of the late peak in the MC
    latePeak = array('f', [0.])
    # Time of the earlier peak in the MC
    earlyPeak = array('f', [0.])

    # Sets the tree branches
    outTree.Branch('slotID', slotID, 'slotID/I')
    outTree.Branch('hwChannel', hwChannel, 'hwChannel/I')
    outTree.Branch('ndf', ndf, 'ndf/F')
    outTree.Branch('chi2', chi2, 'chi2/F')
    outTree.Branch('nPeaks', nPeaks, 'nPeaks/I')
    outTree.Branch('sigma', sigma, 'sigma/F')
    outTree.Branch('errSigma', errSigma, 'errSigma/F')
    outTree.Branch('peakTime', peakTime, 'peakTime/F')
    outTree.Branch('errPeakTime', errPeakTime, 'errPeakTime/F')
    outTree.Branch('extraTime', extraTime, 'extraTime/F')
    outTree.Branch('errExtraTime', errExtraTime, 'errExtraTime/F')
    outTree.Branch('alpha', alpha, 'alpha/F')
    outTree.Branch('errAlpha', errAlpha, 'errAlpha/F')
    outTree.Branch('n', n, 'n/F')
    outTree.Branch('errN', errN, 'errN/F')
    outTree.Branch('lam', lam, 'lam/F')
    outTree.Branch('errLam', errLam, 'errLam/F')
    outTree.Branch('norm1', norm1, 'norm1/F')
    outTree.Branch('errNorm1', errNorm1, 'errNorm1/F')
    outTree.Branch('norm2', norm2, 'norm2/F')
    outTree.Branch('errNorm2', errNorm2, 'errNorm2/F')
    outTree.Branch('normExtra', normExtra, 'normExtra/F')
    outTree.Branch('errNormExtra', errNormExtra, 'errNormExtra/F')
    outTree.Branch('sigmaExtra', sigmaExtra, 'sigmaExtra/F')
    outTree.Branch('errSigmaExtra', errSigmaExtra, 'errSigmaExtra/F')
    outTree.Branch('deltaMC', deltaMC, 'deltaMC/F')
    outTree.Branch('correctedTime', correctedTime, 'correctedTime/F')
    outTree.Branch('latePeak', latePeak, 'latePeak/F')
    outTree.Branch('earlyPeak', earlyPeak, 'earlyPeak/F')

    # Main loop_: takes the histogram, slices it and fits each slide according to the number of peaks found in the MC and the
    # pdf chosen by te user
    for iSlot in range(0, 16):
        print('Fitting slot ' + str(iSlot))
        for k in range(0, 512):
            timeProjection = histoData.ProjectionY('projection_' + str(iSlot + 1) + '_' +
                                                   str(k), k + iSlot * 512 + 1, k + iSlot * 512 + 1)
            maximum = timeProjection.GetXaxis().GetBinCenter(timeProjection.GetMaximumBin())
            if timeProjection.Integral() < 10:
                continue
            timeProjection.GetXaxis().SetRangeUser(maximum - 1., maximum + 2.)

            slotID[0] = iSlot + 1
            hwChannel[0] = k

            # Saves the MC information in the tree variables
            if(mcPeaks[k][1] > 0):
                # By convention the first component of the PDF represents the later peak,
                # which in the MC appears to be also the main one. However this may change with
                # different MC simulations, therefore we have to check which one is
                # the first peak and which one is the second peak
                if mcPeaks[k][1] > mcPeaks[k][0]:
                    earlyPeak[0] = mcPeaks[k][0]
                    latePeak[0] = mcPeaks[k][1]
                else:
                    earlyPeak[0] = mcPeaks[k][1]
                    latePeak[0] = mcPeaks[k][0]
                deltaMC[0] = earlyPeak[0] - latePeak[0]
                nPeaks[0] = 2
            else:
                nPeaks[0] = 1
                deltaMC[0] = 0.
                latePeak[0] = mcPeaks[k][0]
                earlyPeak[0] = 0.

            # fit with the CB
            if pdfType == 'cb':
                cb = TF1(
                    "cb",
                    "[0]*ROOT:: Math: : crystalball_function(x, [2], [3], [4], [5]) + "
                    "[1]*[0]*ROOT::Math::crystalball_function(x, [2], [3], [4], [5]+[6]) + "
                    "[7]*[0]*ROOT:: Math::crystalball_function(x, [2], [3], [9]*[4], [5]+[8]) ",
                    maximum -
                    1.,
                    maximum +
                    2.)
                # main peak
                cb.SetParameter(0, 10.)  # norm 1
                cb.SetParameter(2, -1.)  # alpha
                cb.SetParLimits(2, -30., -0.01)  # alpha
                cb.SetParameter(3, 3.)  # n
                cb.SetParLimits(3, 0.01, 6.)  # n must be on the right side
                cb.SetParameter(4, 0.05)  # sigma
                cb.SetParLimits(4, 0.03, 0.200)  # range for sigma
                cb.SetParameter(5, maximum)  # maximum of the latest peak
                cb.SetParLimits(5, maximum - 0.1, maximum + 0.1)  # maximum of the latest peak
                # backscatter peak
                cb.SetParameter(7, 0.01)  # yield of the spurious peak
                cb.SetParLimits(7, 0., .1)  # yield of the spurious peak
                cb.SetParameter(8, 1.)  # delay of the spurious peak (~ 1 ns)
                cb.SetParLimits(8, 0.2, 2.)  # delay of the spurious peak (~ 1 ns)
                cb.SetParameter(9, 1.5)  # scale factro for sigma of the spurious peak (~2)
                cb.SetParLimits(9, 1.1, 3.)  # scale factro for sigma of the spurious peak (~2)
                if not includeBackscatter:
                    cb.FixParameter(7, 0.)  # yield of the spurious peak
                    cb.FixParameter(8, 1.)  # delay of the spurious peak (~ 1 ns)
                    cb.FixParameter(9, 1.5)  # scale factro for sigma of the spurious peak (~2)

                if(mcPeaks[k][1] > 0 and not useSinglePDF):
                    cb.SetParameter(1, 0.3)  # norm 2
                    cb.SetParLimits(1, 0., 100.)  # norm 1
                    if useMCPeaks:
                        cb.FixParameter(6, deltaMC[0])
                    else:
                        cb.SetParameter(6, deltaMC[0])
                        cb.SetParLimits(6, deltaMC[0] - 0.2, -0.001)
                else:
                    cb.FixParameter(1, 0.)
                    cb.FixParameter(6, 0.)

                cb.SetNpx(1000)
                timeProjection.Fit('cb', 'RQS')
                timeProjection.Fit('cb', 'RQS')
                result = timeProjection.Fit('cb', 'RQS')

                # Dumps the fit results into the tree variables
                chi2[0] = result.Chi2()
                ndf[0] = result.Ndf()
                alpha[0] = cb.GetParameter(2)
                errAlpha[0] = cb.GetParError(2)
                n[0] = cb.GetParameter(3)
                errN[0] = cb.GetParError(3)
                lam[0] = 0.
                errLam[0] = 0.
                peakTime[0] = cb.GetParameter(5)
                errPeakTime[0] = cb.GetParError(5)
                sigma[0] = cb.GetParameter(4)
                errSigma[0] = cb.GetParError(4)
                norm1[0] = cb.GetParameter(0)
                errNorm1[0] = cb.GetParError(0)
                norm2[0] = cb.GetParameter(1)
                errNorm2[0] = cb.GetParError(1)
                normExtra[0] = cb.GetParameter(7)
                errNormExtra[0] = cb.GetParError(7)
                extraTime[0] = cb.GetParameter(8)
                errExtraTime[0] = cb.GetParError(8)
                sigmaExtra[0] = cb.GetParameter(9)
                errSigmaExtra[0] = cb.GetParError(9)

            # models each peak as an exponentially-modified gaussian
            if pdfType == 'gausExpo':

                gausExpo = TF1(
                    "gausExpo",
                    "[0]*(([4]/2.)*TMath::Exp(([4]/2.)*(2*[2] + [4]*[3]*[3]- 2.*x)))*"
                    "ROOT::Math::erfc(([2] + [4]*[3]*[3] - x)/(TMath::Sqrt(2.)*[3])) + "
                    "[1]*[0]*(([4]/2.)*TMath::Exp(([4]/2.)*(2*([2]+[5])+[4]*[3]*[3]-2.*x)))*"
                    "ROOT::Math::erfc((([2]+[5]) + [4]*[3]*[3] - x)/(TMath::Sqrt(2.)*[3])) +"
                    "[6]*[0]*(([4]/2.)*TMath::Exp(([4]/2.)*(2*([2]+[7])+[4]*[8]*[3]*[8]*[3]-2.*x)))*"
                    "ROOT::Math::erfc((([2]+[7]) + [4]*[8]*[3]*[8]*[3] - x)/(TMath::Sqrt(2.)*[8]*[3]))",
                    maximum -
                    1.,
                    maximum +
                    2.)

                # main peak
                gausExpo.SetParameter(0, 1000.)  # norm 1
                gausExpo.SetParameter(2, maximum)  # maximum of the latest peak
                gausExpo.SetParLimits(2, maximum - 0.1, maximum + 0.1)  # maximum of the latest peak
                gausExpo.SetParameter(3, 0.05)  # sigma
                gausExpo.SetParLimits(3, 0.03, 0.200)  # range for sigma
                gausExpo.SetParameter(4, 1.2)  # lambda
                # backscatter peak
                gausExpo.SetParameter(6, 0.01)  # norm extra
                gausExpo.SetParLimits(6, 0., 0.1)  # norm extra
                gausExpo.SetParameter(7, 1.)  # delay of the spurious peak (~ 1 ns)
                gausExpo.SetParLimits(7, 0.2, 2.)  # delay of the spurious peak (~ 1 ns)
                gausExpo.SetParameter(8, 1.5)  # scale factro for sigma of the spurious peak (~2)
                gausExpo.SetParLimits(8, 1.1, 3.)  # scale factro for sigma of the spurious peak (~2)
                if not includeBackscatter:
                    gausExpo.FixParameter(6, 0.)  # yield of the spurious peak
                    gausExpo.FixParameter(7, 1.)  # delay of the spurious peak (~ 1 ns)
                    gausExpo.FixParameter(8, 1.5)  # scale factor for sigma of the spurious peak (~2)

                if(mcPeaks[k][1] > 0 and not useSinglePDF):
                    gausExpo.SetParameter(1, 0.3)  # norm 2
                    gausExpo.SetParLimits(1, 0., 100.)  # norm 1
                    if useMCPeaks:
                        gausExpo.FixParameter(5, deltaMC[0])
                    else:
                        gausExpo.SetParameter(5, deltaMC[0])
                        gausExpo.SetParLimits(5, deltaMC[0] - 0.2, -0.001)
                else:
                    gausExpo.FixParameter(1, 0.)
                    gausExpo.FixParameter(5, 0.)

                # Perform the fit
                gausExpo.SetNpx(1000)
                timeProjection.Fit('gausExpo', 'RQS')
                timeProjection.Fit('gausExpo', 'RQS')
                result = timeProjection.Fit('gausExpo', 'RQS')

                # save the results
                chi2[0] = result.Chi2()
                ndf[0] = result.Ndf()
                alpha[0] = 0.
                errAlpha[0] = 0.
                n[0] = 0.
                errN[0] = 0.
                lam[0] = gausExpo.GetParameter(4)
                errLam[0] = gausExpo.GetParError(4)
                peakTime[0] = gausExpo.GetParameter(2)
                errPeakTime[0] = gausExpo.GetParError(2)
                sigma[0] = gausExpo.GetParameter(3)
                errSigma[0] = gausExpo.GetParError(3)
                norm1[0] = gausExpo.GetParameter(0)
                errNorm1[0] = gausExpo.GetParError(0)
                norm2[0] = gausExpo.GetParameter(1)
                errNorm2[0] = gausExpo.GetParError(1)
                normExtra[0] = gausExpo.GetParameter(6)
                errNormExtra[0] = gausExpo.GetParError(6)
                extraTime[0] = gausExpo.GetParameter(7)
                errExtraTime[0] = gausExpo.GetParError(7)
                sigmaExtra[0] = gausExpo.GetParameter(8)
                errSigmaExtra[0] = gausExpo.GetParError(8)

            # fit using a simple gaussian
            if pdfType == 'gaus':
                gaussian = TF1(
                    "gaussian",
                    "[0]*TMath::Gaus([2], [3], kTRUE)) + "
                    "[0]*[1]*TMath::Gaus([2]+[4], [3], kTRUE)) +"
                    "[0]*[5]*TMath::Gaus([2]+[6], [3]*[7], kTRUE))",
                    maximum -
                    1.,
                    maximum +
                    2.)
                # main peak
                gaussian.SetParameter(0, 10.)  # norm 1
                gaussian.SetParameter(2, maximum)  # maximum of the latest peak
                gaussian.SetParLimits(2, maximum - 0.1, maximum + 0.1)  # maximum of the latest peak
                gaussian.SetParameter(3, 0.05)  # sigma
                gaussian.SetParLimits(3, 0.03, 0.200)  # range for sigma
                # backscatter peak
                gaussian.SetParameter(5, 0.01)  # yield of the spurious peak
                gaussian.SetParLimits(5, 0., .1)  # yield of the spurious peak
                gaussian.SetParameter(6, 1.)  # delay of the spurious peak (~ 1 ns)
                gaussian.SetParLimits(6, 0.2, 2.)  # delay of the spurious peak (~ 1 ns)
                gaussian.SetParameter(7, 1.5)  # scale factro for sigma of the spurious peak (~2)
                gaussian.SetParLimits(7, 1.1, 3.)  # scale factro for sigma of the spurious peak (~2)
                if not includeBackscatter:
                    gaussian.FixParameter(5, 0.)  # yield of the spurious peak
                    gaussian.FixParameter(6, 1.)  # delay of the spurious peak (~ 1 ns)
                    gaussian.FixParameter(7, 1.5)  # scale factro for sigma of the spurious peak (~2)

                if(mcPeaks[k][1] > 0 and not useSinglePDF):
                    gaussian.SetParameter(1, 0.3)  # norm 2
                    gaussian.SetParLimits(1, 0., 100.)  # norm 1
                    if useMCPeaks:
                        gaussian.FixParameter(4, deltaMC[0])
                    else:
                        gaussian.SetParameter(4, deltaMC[0])
                        gaussian.SetParLimits(4, deltaMC[0] - 0.2, -0.001)
                else:
                    gaussian.FixParameter(1, 0.)
                    gaussian.FixParameter(4, 0.)

                gaussian.SetNpx(1000)
                timeProjection.Fit('gaussian', 'RQS')
                timeProjection.Fit('gaussian', 'RQS')
                result = timeProjection.Fit('gaussian', 'RQS')

                # Dumps the fit results into the tree variables
                chi2[0] = result.Chi2()
                ndf[0] = result.Ndf()
                alpha[0] = 0.
                errAlpha[0] = 0.
                n[0] = 0.
                errN[0] = 0.
                lam[0] = 0.
                errLam[0] = 0.
                peakTime[0] = gaussian.GetParameter(2)
                errPeakTime[0] = gaussian.GetParError(2)
                sigma[0] = gaussian.GetParameter(3)
                errSigma[0] = gaussian.GetParError(3)
                norm1[0] = gaussian.GetParameter(0)
                errNorm1[0] = gaussian.GetParError(0)
                norm2[0] = gaussian.GetParameter(1)
                errNorm2[0] = gaussian.GetParError(1)
                normExtra[0] = gaussian.GetParameter(5)
                errNormExtra[0] = gaussian.GetParError(5)
                extraTime[0] = gaussian.GetParameter(6)
                errExtraTime[0] = gaussian.GetParError(6)
                sigmaExtra[0] = gaussian.GetParameter(7)
                errSigmaExtra[0] = gaussian.GetParError(7)

            # calculate the MC-corrected time sobtracting the timing of the main peak in the MC
            correctedTime[0] = peakTime[0] - mcPeaks[k][0]

            outTree.Fill()

            if saveFits:
                timeProjection.Write()

    histoData.Write()
    outTree.Write()


def plotLaserResolution(fitResultsFile='laserResolutionResults.root'):
    """
    Function that creates some standard plots from the tree containg the flaset fit results
    """

    # Opens the file with the data and loads the timing histogram
    tFileData = TFile(fitResultsFile)
    treeData = tFileData.Get('tree')
    histoData = tFileData.Get('LaserTimingVSChannel')

    # Findes the average value of correctedTime to optimize the hisotgram axis ranges
    h_test = TH1F("h_test", "a test histogram to determine the histogram boundaries", 1000, 0., 100.)
    treeData.Draw("correctedTime>>h_test")
    meanCorrTime = h_test.GetMean()

    # laser peak time in each slot, after the MC correction (10 ps binnig)
    h_localT0Resolution = [TH1F("h_localT0Resolution_slot" + str(iSlot + 1),
                                "Laser peak position after the MC correction in slot " + str(iSlot + 1),
                                400,
                                meanCorrTime - 2.,
                                meanCorrTime + 2.) for iSlot in range(16)]
    # laser peak resolution in the whole detector (10 ps binnig)
    h_singleChannelResolution = TH1F("h_singleChannelResolution", "Resolution of the laser peak.", 40, 0., 0.4)

    # laser peak resolution as function of the channel (10 ps binnig)
    h_singleChannelResolutionVSChannel = TH1F(
        "singleChannelResolutionVSChannel",
        "Resolution of the laser peak VS channel number",
        512 * 16,
        0,
        512 * 16)

    # laser peak position as function of the channel (10 ps binnig)
    h_localT0ResolutionVSChannel = TH1F(
        "localT0ResolutionVSChannel",
        "Resolution of the laser peak VS channel number",
        512 * 16,
        0,
        512 * 16)

    # Distribution of the late peak
    h_latePeakVSChannel = TH1F("latePeakVSChannel", "Late Peak position", 512 * 16, 0, 512 * 16)
    # Distribution of the early peak
    h_earlyPeakVSChannel = TH1F("earlyPeakVSChannel", "Early Peak position", 512 * 16, 0, 512 * 16)

    for entry in treeData:
        globalChannel = entry.hwChannel + entry.slotID * 512

        h_localT0Resolution[entry.slotID - 1].Fill(entry.correctedTime)
        h_singleChannelResolution.Fill(entry.sigma)

        h_singleChannelResolutionVSChannel.SetBinContent(globalChannel + 1, entry.sigma)
        h_singleChannelResolutionVSChannel.SetBinError(globalChannel + 1, entry.errSigma)

        h_localT0ResolutionVSChannel.SetBinContent(globalChannel + 1, entry.correctedTime)
        h_localT0ResolutionVSChannel.SetBinError(globalChannel + 1, entry.errPeakTime)

        h_latePeakVSChannel.SetBinContent(globalChannel + 1, entry.latePeak)
        if entry.earlyPeak > 0:
            h_earlyPeakVSChannel.SetBinContent(globalChannel + 1, entry.earlyPeak)

    # loops over the MC distributions for shift the according to the time offset of each slot
    for iSlot in range(16):
        laserTime = histoData.GetBinContent(iSlot * 512 + 1)  # channle 0 of the slot
        mcTime = h_latePeakVSChannel.GetBinContent(iSlot * 512 + 1)  # channle 0 of the slot
        timeShift = laserTime - mcTime
        for iBin in range(512):
            lateTime = h_latePeakVSChannel.GetBinContent(iBin + iSlot * 512 + 1) + timeShift
            earlyTime = h_earlyPeakVSChannel.GetBinContent(iBin + iSlot * 512 + 1) + timeShift
            h_latePeakVSChannel.SetBinContent(iBin + iSlot * 512 + 1, lateTime)
            h_earlyPeakVSChannel.SetBinContent(iBin + iSlot * 512 + 1, earlyTime)

    # Prepares the output file and tree
    tFileOut = TFile('plots.root', "RECREATE")

    histoData.Write()
    h_latePeakVSChannel.Write()
    h_earlyPeakVSChannel.Write()
    h_singleChannelResolution.Write()
    h_singleChannelResolutionVSChannel.Write()
    h_localT0ResolutionVSChannel.Write()

    for i in range(16):
        h_localT0Resolution[i].Write()

    tFileOut.Close()
