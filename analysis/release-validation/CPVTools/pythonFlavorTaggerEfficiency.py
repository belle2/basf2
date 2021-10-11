#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# **********************************  Flavor Tagging   *******************************
# * This scripts calculates the effective efficiency of the flavor tagger based      *
# * on python histograms. It serves as a crosscheck for the script                   *
# * flavorTaggerEfficiency.py, which uses root histograms, and produces nicer plots. *
# * This script also produces plots for different amounts of true categories         *
# * for the combiner outputs. It produces also plots for individual categories       *
# * checking when they are true categories and when they are not true.               *
# * True here means that the target (or targets) of the category are found in a      *
# * certain event. For more information check Sec. 4.5.3 in BELLE2-PTHESIS-2018-003  *
#                                                                                    *
# ************************************************************************************

from matplotlib.colors import colorConverter
from matplotlib.collections import PolyCollection
from ROOT import PyConfig
import glob
import sys
import math
import matplotlib.pyplot as plt
import ROOT
from basf2 import B2INFO
import flavorTagger as ft
from defaultEvaluationParameters import categories, Quiet, rbins
from array import array

import numpy as np
import matplotlib as mpl
mpl.use('Agg')
mpl.rcParams.update({'font.size': 22})
mpl.rcParams['text.usetex'] = True
mpl.rcParams['text.latex.preamble'] = [r"\usepackage{amsmath}"]

PyConfig.IgnoreCommandLineOptions = True
PyConfig.StartGuiThread = False

ROOT.RooMsgService.instance().setGlobalKillBelow(ROOT.RooFit.WARNING)


def efficiencyCalculator(data, total_notTagged, SimpleOutput=False, v='notVerbose'):

    # Calculate Efficiency

    dataB0 = data[np.where(data[:, 0] > 0)]
    dataB0bar = data[np.where(data[:, 0] < 0)]

    totaldataB0 = np.absolute(dataB0[:, 1])
    totaldataB0bar = np.absolute(dataB0bar[:, 1])

    rvalueB0 = (np.histogram(totaldataB0, rbins, weights=totaldataB0)[0] / np.histogram(totaldataB0, rbins)[0])
    rvalueB0MeanSquared = (
        np.histogram(
            totaldataB0,
            rbins,
            weights=totaldataB0 *
            totaldataB0)[0] /
        np.histogram(
            totaldataB0,
            rbins)[0])

    entriesB0 = np.histogram(totaldataB0, rbins)[0]

    rvalueB0bar = (np.histogram(totaldataB0bar, rbins, weights=totaldataB0bar)[0] / np.histogram(totaldataB0bar, rbins)[0])
    rvalueB0barMeanSquared = (
        np.histogram(
            totaldataB0bar,
            rbins,
            weights=totaldataB0bar *
            totaldataB0bar)[0] /
        np.histogram(
            totaldataB0bar,
            rbins)[0])
    entriesB0bar = np.histogram(totaldataB0bar, rbins)[0]

    for row in dataB0:
        if row[0] == 1 and abs(row[1]) > 1:
            print(row)

    total_entries_B0 = totaldataB0.shape[0] + total_notTagged / 2
    tot_eff_eff_B0 = 0
    event_fraction_B0 = entriesB0.astype(float) / total_entries_B0

    total_entries_B0bar = totaldataB0bar.shape[0] + total_notTagged / 2
    tot_eff_eff_B0bar = 0
    event_fraction_B0bar = entriesB0bar.astype(float) / total_entries_B0bar

    tot_eff_eff_Avg = 0

    arrayShape = len(rbins) - 1

    event_fractionTotal = np.zeros(arrayShape)
    event_fractionB0 = np.zeros(arrayShape)
    event_fractionB0bar = np.zeros(arrayShape)
    event_fractionDiff = np.zeros(arrayShape)
    event_fractionDiffUncertainty = np.zeros(arrayShape)
    event_fractionTotalUncertainty = np.zeros(arrayShape)

    wvalue = np.zeros(arrayShape)
    wvalueB0 = np.zeros(arrayShape)
    wvalueB0bar = np.zeros(arrayShape)
    wvalueDiff = np.zeros(arrayShape)
    wvalueDiffUncertainty = np.zeros(arrayShape)
    wvalueUncertainty = np.zeros(arrayShape)

    rvalueStdB0 = np.zeros(arrayShape)
    rvalueStdB0bar = np.zeros(arrayShape)

    muParam = np.zeros(arrayShape)
    muParamUncertainty = np.zeros(arrayShape)

    # Wrong tag fraction from MC counts

    NwrongB0 = np.histogram(
        np.absolute(
            data[np.where((np.sign(data[:, 0]) != np.sign(data[:, 1])) & (data[:, 0] == 1))][:, 1]), rbins)[0].astype(float)

    wvalueB0 = (NwrongB0 / entriesB0)

    wvalueB0Uncertainty = np.sqrt(NwrongB0 * (entriesB0 - NwrongB0) / (entriesB0**3))

    NwrongB0bar = np.histogram(
        np.absolute(
            data[np.where((np.sign(data[:, 0]) != np.sign(data[:, 1])) & (data[:, 0] == -1))][:, 1]), rbins)[0].astype(float)

    wvalueB0bar = (NwrongB0bar / entriesB0bar)

    wvalueB0barUncertainty = np.sqrt(NwrongB0bar * (entriesB0bar - NwrongB0bar) / (entriesB0bar**3))

    wvalueDiff = wvalueB0 - wvalueB0bar
    wvalueDiffUncertainty = np.sqrt(wvalueB0Uncertainty**2 + wvalueB0barUncertainty**2)

    for i in range(0, len(rbins) - 1):

        event_fractionB0[i] = entriesB0[i] / total_entries_B0
        event_fractionB0bar[i] = entriesB0bar[i] / total_entries_B0bar

        event_fractionTotal[i] = (event_fractionB0[i] + event_fractionB0bar[i]) / 2
        event_fractionDiff[i] = event_fractionB0[i] - event_fractionB0bar[i]

        event_fractionDiffUncertainty[i] = math.sqrt(entriesB0[i] *
                                                     (total_entries_B0 -
                                                      entriesB0[i]) /
                                                     total_entries_B0**3 +
                                                     entriesB0bar[i] *
                                                     (total_entries_B0bar -
                                                      entriesB0bar[i]) /
                                                     total_entries_B0bar**3)

        event_fractionTotalUncertainty[i] = event_fractionDiffUncertainty[i] / 2

        muParam[i] = event_fractionDiff[i] / (2 * event_fractionTotal[i])
        muParamUncertainty[i] = event_fractionDiffUncertainty[i] / (2 * event_fractionTotal[i]) * math.sqrt(muParam[i]**2 + 1)

        rvalueStdB0[i] = math.sqrt(rvalueB0MeanSquared[i] - (rvalueB0[i])**2) / math.sqrt(entriesB0[i] - 1)
        rvalueStdB0bar[i] = math.sqrt(rvalueB0barMeanSquared[i] - (rvalueB0bar[i])**2) / math.sqrt(entriesB0bar[i] - 1)

        # wvalueB0[i] = (1 - rvalueB0[i]) / 2
        # wvalueB0bar[i] = (1 - rvalueB0bar[i]) / 2
        # wvalueDiff[i] = wvalueB0[i] - wvalueB0bar[i]
        # wvalueDiffUncertainty[i] = math.sqrt((rvalueStdB0[i] / 2)**2 + (rvalueStdB0bar[i] / 2)**2)
        wvalue[i] = (wvalueB0[i] + wvalueB0bar[i]) / 2
        wvalueUncertainty[i] = wvalueDiffUncertainty[i] / 2

        tot_eff_eff_B0 += event_fraction_B0[i] * (1 - 2 * wvalueB0[i])**2
        tot_eff_eff_B0bar += event_fraction_B0bar[i] * (1 - 2 * wvalueB0bar[i])**2

        eff = event_fractionTotal[i] * (1 - 2 * wvalue[i])**2
        # (event_fraction_B0[i] * rvalueB0[i] * rvalueB0[i] + event_fraction_B0bar[i] * rvalueB0bar[i] * rvalueB0bar[i]) / 2
        tot_eff_eff_Avg += eff

        eff2 = (event_fraction_B0[i] * (1 - 2 * wvalueB0[i])**2 + event_fraction_B0bar[i] * (1 - 2 * wvalueB0bar[i])**2) / 2

        rval = (rvalueB0[i] + rvalueB0bar[i]) / 2
        if v == 'verbose':
            print(
                "r = ",
                "{: 6.3f}".format(rval),
                "Eff1 = ",
                "{: 6.3f}".format(eff),
                "Eff2 = ",
                "{: 6.3f}".format(eff2),
                "w = ",
                "{: 8.3f}".format(
                    float(
                        wvalue[i] *
                        100)),
                "Delta w = ",
                "{: 6.3f}".format(
                    float(
                        wvalueDiff[i] *
                        100)) +
                " +-" +
                "{: 1.3f}".format(
                    float(
                        wvalueDiffUncertainty[i] *
                        100)),
                "epsilon = ",
                "{: 1.3f}".format(
                    float(
                        event_fractionTotal[i] *
                        100)),
                "Delta epsilon = ",
                "{: 1.3f}".format(
                    float(
                        event_fractionDiff[i] *
                        100)) +
                " +-" +
                "{: 1.3f}".format(
                    float(
                        event_fractionDiffUncertainty[i] *
                        100)),
                "mu = ",
                "{: 1.3f}".format(
                    float(
                        muParam[i] *
                        100)) +
                " +-" +
                "{: 1.3f}".format(
                    float(
                        muParamUncertainty[i] *
                        100)))

    tot_eff_eff = (tot_eff_eff_B0 + tot_eff_eff_B0bar) / 2
    tot_eff_diff = tot_eff_eff_B0 - tot_eff_eff_B0bar
    efficiency = 100 * tot_eff_eff_Avg
    efficiencyDiff = 100 * tot_eff_diff
    efficiencyB0 = 100 * tot_eff_eff_B0
    efficiencyB0bar = 100 * tot_eff_eff_B0bar

    if v == 'verbose':
        print("Total Efficiency from Avr. rB0 and rB0bar  ", "{: 6.3f}".format(float(100 * tot_eff_eff)))
        print("Total Efficiency from Avr. wB0 and wB0bar  ", "{: 6.3f}".format(float(100 * tot_eff_eff_Avg)))

    if SimpleOutput:

        return efficiency, efficiencyDiff, efficiencyB0, efficiencyB0bar

    else:

        return efficiency, efficiencyDiff, efficiencyB0, efficiencyB0bar, wvalue, wvalueB0, \
            wvalueB0bar, wvalueDiff, wvalueDiffUncertainty, event_fractionB0, event_fractionB0bar, \
            event_fractionDiffUncertainty, event_fractionTotalUncertainty, muParam, muParamUncertainty


def categoriesEfficiencyCalculator(data, v='notVerbose'):

    # Calculate Efficiency

    r_subsample = array('d', [
        0.0,
        0.1,
        0.25,
        0.5,
        0.625,
        0.75,
        0.875,
        1.0])

    hist_aver_rB0 = ROOT.TH1F('AverageR' + category, 'A good one (B0)' +
                              category, 6, r_subsample)
    hist_aver_rB0bar = ROOT.TH1F('AverageRB0bar_' + category, 'A good one (B0bar)' +
                                 category, 6, r_subsample)

    hist_absqrB0 = ROOT.TH1F('AbsQR' + category, 'Abs(qr)(B0) (' + category + ')', 6, r_subsample)
    hist_absqrB0bar = ROOT.TH1F('AbsQRB0bar_' + category, 'Abs(qr) (B0bar) (' + category + ')', 6, r_subsample)

    dataB0 = data[np.where(data[:, 0] > 0)]
    dataB0bar = data[np.where(data[:, 0] < 0)] * (-1)

    # -----bins
    bins = list(range(-25, 27, 1))
    for i in range(0, len(bins)):
        bins[i] = float(bins[i]) / 25
    # ------

    histoB0 = np.histogram(dataB0, bins=bins)[0]
    histoB0bar = np.histogram(dataB0bar, bins=bins)[0]
    histoB0bar = histoB0bar[0:len(histoB0bar) - 1]
    histoB0bar = histoB0bar[::-1]

    dilutionB0 = np.zeros(50)
    dilutionB0bar = np.zeros(50)

    for i in range(0, 50):
        if (histoB0[i] + histoB0bar[i]) != 0:
            dilutionB0[i] = -1 + 2 * histoB0[i] / (histoB0[i] + histoB0bar[i])
            dilutionB0bar[i] = -1 + 2 * histoB0bar[i] / (histoB0[i] + histoB0bar[i])
        hist_absqrB0.Fill(abs(dilutionB0[i]), histoB0[i])
        hist_absqrB0bar.Fill(abs(dilutionB0bar[i]), histoB0bar[i])
        hist_aver_rB0.Fill(abs(dilutionB0[i]), abs(dilutionB0[i]) * histoB0[i])
        hist_aver_rB0bar.Fill(abs(dilutionB0bar[i]), abs(dilutionB0bar[i]) * histoB0bar[i])

    hist_aver_rB0.Divide(hist_absqrB0)
    hist_aver_rB0bar.Divide(hist_absqrB0bar)

    tot_entriesB0 = dataB0[:, 1].shape[0]
    tot_entriesB0bar = dataB0bar[:, 1].shape[0]

    tot_eff_effB0 = 0
    tot_eff_effB0bar = 0
    rvalueB0 = np.zeros(rbins.shape[0])
    rvalueB0bar = np.zeros(rbins.shape[0])
    entriesB0 = np.zeros(rbins.shape[0])
    entriesB0bar = np.zeros(rbins.shape[0])
    event_fractionB0 = np.zeros(rbins.shape[0])
    event_fractionB0bar = np.zeros(rbins.shape[0])

    for i in range(1, rbins.shape[0]):
        rvalueB0[i] = hist_aver_rB0.GetBinContent(i)
        rvalueB0bar[i] = hist_aver_rB0bar.GetBinContent(i)
        entriesB0[i] = hist_absqrB0.GetBinContent(i)
        entriesB0bar[i] = hist_absqrB0bar.GetBinContent(i)
        event_fractionB0[i] = entriesB0[i] / tot_entriesB0
        event_fractionB0bar[i] = entriesB0bar[i] / tot_entriesB0bar
        tot_eff_effB0 = tot_eff_effB0 + event_fractionB0[i] * rvalueB0[i] \
            * rvalueB0[i]
        tot_eff_effB0bar = tot_eff_effB0bar + event_fractionB0bar[i] * rvalueB0bar[i] \
            * rvalueB0bar[i]
    effDiff = tot_eff_effB0 - tot_eff_effB0bar
    effAverage = (tot_eff_effB0 + tot_eff_effB0bar) / 2

    efficiency = 100 * effAverage
    efficiencyDiff = 100 * effDiff
    efficiencyB0 = 100 * tot_eff_effB0
    efficiencyB0bar = 100 * tot_eff_effB0bar

    return efficiency, efficiencyDiff, efficiencyB0, efficiencyB0bar


ROOT.gROOT.SetBatch(True)

if len(sys.argv) != 3:
    sys.exit("Must provide 2 arguments: [input_sim_file] or ['input_sim_file*'] with wildcards and [treeName]"
             )
workingFile = sys.argv[1]
workingFiles = glob.glob(str(workingFile))
treeName = str(sys.argv[2])

if len(workingFiles) < 1:
    sys.exit("No file name or file names " + str(workingFile) + " found.")

workingDirectory = '.'

methods = []


tree = ROOT.TChain(treeName)

mcstatus = array('d', [-511.5, 0.0, 511.5])
ROOT.TH1.SetDefaultSumw2()

for iFile in workingFiles:
    tree.AddFile(iFile)

totalBranches = []
for branch in tree.GetListOfBranches():
    totalBranches.append(branch.GetName())

existsDeltaT = False

if 'FBDT_qrCombined' in totalBranches:
    methods.append("FBDT")

if 'FANN_qrCombined' in totalBranches:
    methods.append("FANN")

if 'DNN_qrCombined' in totalBranches:
    methods.append("DNN")

if 'DeltaT' in totalBranches:
    existsDeltaT = True

usedCategories = []
for cat in categories:
    catBranch = 'qp' + cat
    if catBranch in totalBranches:
        usedCategories.append(cat)

YmaxForQrPlot = 0
YmaxForDeltaTPlot = 0

total_notTagged = 0

for method in methods:

    if method == "FBDT":
        label = method
    elif method == "DNN":
        label = method
    elif method == "FANN":
        label = "MLP"

    with Quiet(ROOT.kError):
        qpMaximumPstar = ROOT.RooRealVar('qpMaximumPstar', 'qpMaximumPstar', 0, -2.1, 1.1)
        qrCombined = ROOT.RooRealVar('' + method + '_qrCombined', '' + method + '_qrCombined', 0, -1.1, 1.1)
        qrMC = ROOT.RooRealVar('qrMC', 'qrMC', 0, -10.0, 10.0)
        isSignal = ROOT.RooRealVar('isSignal', 'isSignal', 0, -10.0, 10.0)

        DeltaT = ROOT.RooRealVar("DeltaT", "DeltaT", 0., -100000., 100000.)

        argSet = ROOT.RooArgSet(qrCombined, qrMC, isSignal, qpMaximumPstar)

        if existsDeltaT:

            argSet.add(DeltaT)

        for category in usedCategories:

            exec(
                "%s = %s" %
                ("hasTrueTarget" +
                 category,
                 "ROOT.RooRealVar('hasTrueTarget' + category, 'hasTrueTarget' + category, 0, -2.1, 1.1)"))
            # exec("%s = %s" % ("isTrueCategory" + category,
            # "ROOT.RooRealVar('isRightCategory' + category, 'isRightCategory' +
            # category, 0, -1.0, 1.1)"))
            exec("%s" % "argSet.add(hasTrueTarget" + category + ")")

        rooDataSet = ROOT.RooDataSet("data", "data", tree, argSet, "")

    dataAll = []
    numberOfTrueCatagories = []
    deltaT = []
    # data = []

    for i in range(rooDataSet.numEntries()):
        row = rooDataSet.get(i)
        tqrCombined = row.getRealValue('' + method + '_qrCombined', 0, ROOT.kTRUE)
        tqrMC = row.getRealValue('qrMC', 0, ROOT.kTRUE)
        tisSignal = row.getRealValue('isSignal', 0, ROOT.kTRUE)

        if tisSignal == 1 and tqrCombined < -1 and abs(tqrMC) == 0:
            total_notTagged += 1

        # tqpMaximumPstar = row.getRealValue('qpMaximumPstar', 0, ROOT.kTRUE)

        if tqrCombined >= 1:
            tqrCombined = 0.999

        if tqrCombined <= -1:
            tqrCombined = -0.999

        tNumberOfTrueCategories = 0

        for category in usedCategories:

            if category == "MaximumPstar":
                continue
            # tisTrueCategory = row.getRealValue('isRightCategory' + category, 0, ROOT.kTRUE)
            thasTrueTarget = row.getRealValue('hasTrueTarget' + category, 0, ROOT.kTRUE)

            if thasTrueTarget > 0:
                tNumberOfTrueCategories += 1
        if abs(tqrMC) == 1:  # !=0:
            # data.append([tqrMC, tqrCombined])
            numberOfTrueCatagories.append(tNumberOfTrueCategories)
            # if tNumberOfTrueCategories == 0:
            #     dataNoTrueCategory.append([tqrMC, tqrCombined])
            if existsDeltaT:
                tDT = row.getRealValue("DeltaT", 0, ROOT.kTRUE)
                dataAll.append([tqrMC, tqrCombined, tNumberOfTrueCategories, tDT])
            else:
                dataAll.append([tqrMC, tqrCombined, tNumberOfTrueCategories])

    # data = np.array(data)
    dataAll = np.array(dataAll)
    dataNoTrueCategory = dataAll[(np.where(dataAll[:, 2] == 0))][:, 0:4]
    data = dataAll[:, 0:4]
    numberOfTrueCatagories = np.array(numberOfTrueCatagories)
    # Calculate Efficiency

    dataB0 = data[np.where(data[:, 0] > 0)]
    dataB0bar = data[np.where(data[:, 0] < 0)]

    efficiency, efficiencyDiff, efficiencyB0, efficiencyB0bar, wvalue, wvalueB0, wvalueB0bar, wvalueDiff, wvalueDiffUncertainty, \
        event_fractionB0, event_fractionB0bar, event_fractionDiffUncertainty, event_fractionTotalUncertainty, muParam, \
        muParamUncertainty = efficiencyCalculator(data, total_notTagged, False, 'verbose')

    efficiency = '% .2f' % efficiency
    efficiencyDiff = '%.2f' % efficiencyDiff
    efficiencyB0 = '% .2f' % efficiencyB0
    efficiencyB0bar = '% .2f' % efficiencyB0bar

    N = data.shape[0]
    NB0 = dataB0.shape[0] / N
    NB0 = 100 * NB0
    NB0 = '% .2f' % NB0
    NB0bar = dataB0bar.shape[0] / N
    NB0bar = 100 * NB0bar
    NB0bar = '% .2f' % NB0bar
    NnoTrueCat = dataNoTrueCategory.shape[0]
    FracNoFTInfo = NnoTrueCat / N

    print('The total efficiency for B0: ', efficiencyB0, '%')
    print('The total efficiency for B0bar: ', efficiencyB0bar, '%')
    print('The total efficiency for method ' + method + ' is: ', efficiency, '%')
    print('The total efficiency Diff for method ' + method + ' is: ', efficiencyDiff, '%')

    print('N : ', N)
    print('Percentage B0/ N: ', NB0, '%')
    print('Percentage  B0bar / N : ', NB0bar, '%')

    print("   ")
    print("N without trueCats = ", NnoTrueCat)
    print("Fraction Of Events without trueCats Info = ", '{: 4.2f}'.format(float(FracNoFTInfo * 100)), "% ")

    # ----------------Delta T Plots -------------------------------------------------------------------------------

    if existsDeltaT:

        # -----DeltaTbins
        DeltaTbins = list(range(-1000, 1000, 1))
        for i in range(0, len(DeltaTbins)):
            DeltaTbins[i] = float(DeltaTbins[i]) / 10
        # ------

        ax = plt.axes([0.21, 0.15, 0.75, 0.8])
        y, x, _ = ax.hist(data[:, 3], bins=DeltaTbins, facecolor='r', histtype='stepfilled', edgecolor='r')  # histtype='step'

        YDTmax = y.max()
        YDTmax = YDTmax - YDTmax / 5

        if YmaxForDeltaTPlot < YDTmax:
            YmaxForDeltaTPlot = YDTmax

        figTmc = plt.figure(1, figsize=(11, 8))
        axTmc = plt.axes([0.21, 0.15, 0.76, 0.8])

        axTmc.hist(data[np.where(data[:, 0] == -1)][:, 3], bins=DeltaTbins, histtype='step',
                   edgecolor='b', linewidth=4.5, linestyle='dashed', label=r'$\bar{B}^0_{\rm tag}$')  # hatch='.', 'dotted'

        axTmc.hist(data[np.where(data[:, 0] == 1)][:, 3], bins=DeltaTbins, histtype='step',
                   edgecolor='r', linewidth=4, alpha=0.9, label=r'$B^0_{\rm tag}$')  # histtype='step', hatch='\\'

        p1, =  axTmc.plot([], label=r'$B^0_{\rm tag}$', linewidth=4, linestyle='solid', alpha=0.9, c='r')
        p2, =  axTmc.plot([], label=r'$\bar{B}^0_{\rm tag}$', linewidth=4.5, linestyle='dashed', c='b')

        axTmc.set_ylabel(r'${\rm Number\ \ of\ \ Events /\ (\, 0.1\ ps\, )}$', fontsize=35)
        axTmc.set_xlabel(r'$\Delta t\ /\ {\rm ps}$', fontsize=35)

        plt.xticks([-10, -7.5, -5, -2.5, 0, 2.5, 5, 7.5, 10], [r'$-10$', r'', r'$5$',
                                                               r'', r'$0$', r'', r'$5$', r'', r'$10$'], rotation=0, size=40)
        axTmc.tick_params(axis='y', labelsize=35)
        axTmc.legend([p1, p2], [r'$B^0_{\rm tag}$', r'$\bar{B}^0_{\rm tag}$'], prop={'size': 35})
        axTmc.grid(True)
        axTmc.set_ylim(0, YmaxForDeltaTPlot)
        axTmc.set_xlim(-10, 10)
        plt.savefig(workingDirectory + '/' + method + 'DeltaTMC.pdf')
        figTmc.clear()

        figTft = plt.figure(2, figsize=(11, 8))
        axTft = plt.axes([0.21, 0.15, 0.76, 0.8])

        axTft.hist(data[np.where(data[:, 1] < 0)][:, 3], bins=DeltaTbins, histtype='step',
                   edgecolor='b', linewidth=4.5, linestyle='dashed', label=r'$\bar{B}^0_{\rm tag}$')  # hatch='.', 'dotted'

        axTft.hist(data[np.where(data[:, 1] > 0)][:, 3], bins=DeltaTbins, histtype='step',
                   edgecolor='r', linewidth=4, alpha=0.9, label=r'$B^0_{\rm tag}$')  # histtype='step', hatch='\\'

        p1, =  axTft.plot([], label=r'$B^0_{\rm tag}$', linewidth=4, linestyle='solid', alpha=0.9, c='r')
        p2, =  axTft.plot([], label=r'$\bar{B}^0_{\rm tag}$', linewidth=4.5, linestyle='dashed', c='b')

        axTft.set_ylabel(r'${\rm Number\ \ of\ \ Events /\ (\, 0.1\ ps\, )}$', fontsize=35)
        axTft.set_xlabel(r'$\Delta t\ /\ {\rm ps}$', fontsize=35)

        plt.xticks([-10, -7.5, -5, -2.5, 0, 2.5, 5, 7.5, 10], [r'$-10$', r'', r'$5$',
                                                               r'', r'$0$', r'', r'$5$', r'', r'$10$'], rotation=0, size=40)
        axTft.tick_params(axis='y', labelsize=35)
        axTft.legend([p1, p2], [r'$B^0_{\rm tag}$', r'$\bar{B}^0_{\rm tag}$'], prop={'size': 35})
        axTft.grid(True)
        axTft.set_ylim(0, YmaxForDeltaTPlot)
        axTft.set_xlim(-10, 10)
        plt.savefig(workingDirectory + '/' + method + 'DeltaTFT.pdf')
        figTft.clear()

    # ----------------QR and Calibration Plots ---------------------------------------------------------------------

    # -----bins
    bins = list(range(-50, 51, 1))
    for i in range(0, len(bins)):
        bins[i] = float(bins[i]) / 50
    # ------

    # -----bins2
    bins2 = list(range(-25, 26, 1))
    for i in range(0, len(bins2)):
        bins2[i] = float(bins2[i]) / 25
    # ------

    fig1 = plt.figure(2, figsize=(11, 8))
    ax1 = plt.axes([0.21, 0.15, 0.75, 0.8])
    y, x, _ = ax1.hist(data[:, 1], bins=bins, facecolor='r', histtype='stepfilled', edgecolor='r')  # histtype='step'

    Ymax = y.max()
    Ymax = Ymax + Ymax / 4

    if YmaxForQrPlot < Ymax:
        YmaxForQrPlot = Ymax

    ax1.set_ylabel(r'${\rm Number\ \ of\ \ Events /\ (\, 0.02\, )}$', fontsize=35)
    ax1.set_xlabel(r'$(q\cdot r)_{\rm ' + label + ' }$', fontsize=42)
    plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1],
               [r'$-1$', r'', r'$-0.5$', r'', r'$0$', r'', r'$0.5$', r'', r'$1$'], rotation=0, size=20)
    ax1.tick_params(axis='y', labelsize=20)
    ax1.grid(True)
    ax1.set_ylim(0, YmaxForQrPlot)
    plt.savefig(workingDirectory + '/' + method + 'QR_Output.pdf')
    fig1.clear()

    # ---------------------------  QR Plot  ----------------- ----------------------

    fig2 = plt.figure(3, figsize=(11, 8))
    ax2 = plt.axes([0.21, 0.15, 0.76, 0.8])

    ax2.hist(data[np.where(abs(data[:, 0]) == 1)][:, 1], bins=bins, histtype='step', edgecolor='k',
             linewidth=4, linestyle='dashdot', alpha=0.9, label=r'${\rm Both}$')

    ax2.hist(data[np.where(data[:, 0] == 1)][:, 1], bins=bins, histtype='step', edgecolor='r',
             linewidth=4, alpha=0.9, label=r'$B^0$')  # histtype='step', hatch='\\'

    ax2.hist(data[np.where(data[:, 0] == -1)][:, 1], bins=bins, histtype='step',
             edgecolor='b', linewidth=4.5, linestyle='dashed', label=r'$\bar{B}^0$')  # hatch='.', 'dotted'

    p3, =  ax2.plot([], label=r'${\rm Both}$', linewidth=4, linestyle='dashdot', alpha=0.9, c='k')

    p2, =  ax2.plot([], label=r'$\bar{B}^0$', linewidth=4.5, linestyle='dashed', c='b')

    p1, =  ax2.plot([], label=r'$B^0$', linewidth=4, linestyle='solid', alpha=0.9, c='r')

    ax2.set_ylabel(r'${\rm Number\ \ of\ \ Events\ /\ (\, 0.02\, )}$', fontsize=35)
    ax2.set_xlabel(r'$(q\cdot r)_{\rm ' + label + '}$', fontsize=42)
    # plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1], [r'$-1$', r'',
    # r'$-0.5$', r'', r'$0$', r'', r'$0.5$', r'', r'$1$'], rotation=0,
    # size=40)
    ax2.tick_params(axis='y', labelsize=35)
    ax2.legend([p3, p2, p1], [r'${\rm Both}$', r'$\bar{B}^0$', r'$B^0$'], prop={'size': 35}, loc=9)
    ax2.grid(True)
    plt.xticks([-1, -0.875, -0.75, -0.625, -0.5, -0.25, -0.1, 0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1],
               [r'${-1.0}$', r'', r'', r'', r'${-0.5}$', r'', r'', r'$0$', r'',
                r'', r'$0.5$', r'', r'', r'', r'$1.0$'], rotation=0, size=35)

    ax2.set_ylim(0, YmaxForQrPlot)
    yLocs, yLabels = plt.yticks()
    plt.savefig(workingDirectory + '/' + method + 'QR_B0bar.pdf')
    fig2.clear()

    # --- No true category plot

    fig2b = plt.figure(4, figsize=(10, 8))
    ax2b = plt.axes([0.21, 0.15, 0.76, 0.8])
    p2b = ax2b.hist(dataNoTrueCategory[np.where(dataNoTrueCategory[:, 0] == -1)][:, 1], bins=bins, histtype='step',
                    edgecolor='b', linewidth=3.5, linestyle='dotted', label=r'$\bar{B}^0$')  # hatch='.',
    p1b = ax2b.hist(
        dataNoTrueCategory[
            np.where(
                dataNoTrueCategory[
                    :,
                    0] == 1)][
            :,
            1],
        bins=bins,
        histtype='step',
        edgecolor='r',
        linewidth=2,
        alpha=0.9,
        label=r'$B^0$')  # histtype='step', hatch='\\'
    ax2b.set_ylabel(r'${\rm Number\ \ of\ \ Events\ /\ (\, 0.02\, )}$', fontsize=35)
    ax2b.set_xlabel(r'$(q\cdot r)_{\rm ' + label + '}$', fontsize=42)
    plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1],
               [r'$-1$', r'', r'$-0.5$', r'', r'$0$', r'', r'$0.5$', r'', r'$1$'], rotation=0, size=20)
    ax2b.tick_params(axis='y', labelsize=20)
    ax2b.legend([r'$\bar{B}^0$', r'$B^0$'], prop={'size': 20})
    ax2b.grid(True)
    # ax2b.set_ylim(0, Ymax)
    plt.savefig(workingDirectory + '/' + method + 'QR_B0barNoTrueCategory.pdf')
    fig2b.clear()

    # rbins = np.array([0.0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1.0])

    # -----bins
    # rbins=range(0,100,1)
    # for i in xrange(len(rbins)):
    # bins[i]=float(rbins[i])/100
    # ------

    # ------------------------ Calibration plots ----------------------##

    # data_entries = np.histogram(np.absolute(data[:, 1]), rbins)[0]

    data_entriesB0 = np.histogram(np.absolute(data[np.where(data[:, 0] == 1)][:, 1]), rbins)[0]

    rvalueB0 = (
        np.histogram(
            np.absolute(data[np.where(data[:, 0] == 1)][:, 1]),
            rbins,
            weights=np.absolute(data[np.where(data[:, 0] == 1)][:, 1]))[0] / data_entriesB0)
    delta_rvalueB0 = (
        np.histogram(
            np.absolute(data[np.where(data[:, 0] == 1)][:, 1]), rbins, weights=np.absolute(data[np.where(data[:, 0] == 1)][:, 1]) *
            np.absolute(data[np.where(data[:, 0] == 1)][:, 1]))[0] / data_entriesB0)
    delta_rvalueB0 = np.sqrt((delta_rvalueB0 - rvalueB0 * rvalueB0) / (data_entriesB0))

    wrongTaggedB0 = np.histogram(
        np.absolute(
            data[np.where((np.sign(data[:, 0]) != np.sign(data[:, 1])) & (data[:, 0] == 1))][:, 1]), rbins)[0].astype(float)

    wrong_fractionB0 = (wrongTaggedB0 / data_entriesB0)

    delta_wrong_fractionB0 = np.sqrt(wrongTaggedB0 * (data_entriesB0 - wrongTaggedB0) / (data_entriesB0**3))

    rmvaluesB0 = 1 - 2 * wrong_fractionB0
    delta_rmvaluesB0 = 2 * delta_wrong_fractionB0

    data_entriesB0bar = np.histogram(np.absolute(data[np.where(data[:, 0] == -1)][:, 1]), rbins)[0]

    rvalueB0bar = (
        np.histogram(
            np.absolute(data[np.where(data[:, 0] == -1)][:, 1]),
            rbins,
            weights=np.absolute(data[np.where(data[:, 0] == -1)][:, 1]))[0] / data_entriesB0bar)
    delta_rvalueB0bar = (
        np.histogram(
            np.absolute(data[np.where(data[:, 0] == -1)][:, 1]), rbins,
            weights=np.absolute(data[np.where(data[:, 0] == -1)][:, 1]) *
            np.absolute(data[np.where(data[:, 0] == -1)][:, 1]))[0] / data_entriesB0bar)
    delta_rvalueB0bar = np.sqrt((delta_rvalueB0bar - rvalueB0bar * rvalueB0bar) / (data_entriesB0bar))

    wrongTaggedB0bar = np.histogram(
        np.absolute(
            data[np.where((np.sign(data[:, 0]) != np.sign(data[:, 1])) & (data[:, 0] == -1))][:, 1]), rbins)[0].astype(float)

    wrong_fractionB0bar = (wrongTaggedB0bar / data_entriesB0bar)

    delta_wrong_fractionB0bar = np.sqrt(wrongTaggedB0bar * (data_entriesB0bar - wrongTaggedB0bar) / (data_entriesB0bar**3))

    rmvaluesB0bar = 1 - 2 * wrong_fractionB0bar
    delta_rmvaluesB0bar = 2 * delta_wrong_fractionB0bar

    rvalue = (rvalueB0 + rvalueB0bar) / 2
    rmvalues = (rmvaluesB0 + rmvaluesB0bar) / 2

    delta_rvalue = np.sqrt(delta_rvalueB0**2 + delta_rvalueB0bar**2) / 2
    delta_rmvalues = np.sqrt(delta_rmvaluesB0**2 + delta_rmvaluesB0bar**2) / 2

    fig3a = plt.figure(5, figsize=(11, 8))
    ax3a = plt.axes([0.21, 0.15, 0.74, 0.8])  # plt.axes([0.14, 0.1, 0.8, 0.8])

    line = range(0, 2, 1)
    ax3a.plot(line, line, linewidth=4, color='r')

    (p1ca, capsB0, _) = ax3a.errorbar(rvalueB0, np.absolute(rmvaluesB0), xerr=None, yerr=None,
                                      # xerr=delta_rvalueB0, yerr=delta_rmvaluesB0,
                                      elinewidth=4, capsize=10, ecolor='r', fmt='o', mfc='r',
                                      mec='r', markersize=14)  # histtype='step'

    (p2ca, capsB0bar, _) = ax3a.errorbar(rvalueB0bar, np.absolute(rmvaluesB0bar), xerr=None, yerr=None,
                                         # xerr=delta_rvalueB0bar, yerr=delta_rmvaluesB0bar,
                                         elinewidth=4, capsize=10, ecolor='b', fmt='o', mfc='b',
                                         mec='b', markersize=14)  # histtype='step'

    for cap in capsB0:
        cap.set_color('r')
        cap.set_alpha(0.9)
        cap.set_markeredgewidth(4)

    for cap in capsB0bar:
        cap.set_color('b')
        cap.set_markeredgewidth(4)

    (p3ca, caps, _) = ax3a.errorbar(rvalue, rmvalues, xerr=None, yerr=None,
                                    # xerr=delta_rvalue, yerr=delta_rmvalues,
                                    elinewidth=4, capsize=10, ecolor='k', fmt='o', mfc='k', mec='k', markersize=14)

    for cap in caps:
        cap.set_color('k')
        cap.set_markeredgewidth(4)

    ax3a.legend([p3ca, p2ca, p1ca], [r'${\rm Average}$', r'$\bar{B}^0$', r'$B^0$'], prop={'size': 35}, loc=2)

    ax3a.set_ylabel(r'$r_{\rm \small MC} = \vert 1-2\cdot w_{\small \rm MC} \vert $', fontsize=42)
    ax3a.set_xlabel(r'$\langle r_{\rm ' + label + '}$' + r'$\rangle$', fontsize=42)
    # plt.xticks([0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1], [r'$0$', r'', r'$0.2$',
    # r'', r'$0.4$', r'', r'$0.6$', r'', r'$0.8$', r'', r'$1.0$'],
    # rotation=0, size=35)
    plt.xticks([0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1],
               [r'$0$', r'$0.1$', r'$0.25$', r'$0.5$', r'', r'$0.75$', r'', r'$1.0$'], rotation=0, size=35)
    plt.yticks([0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1],
               [r'$0$', r'$0.1$', r'$0.25$', r'$0.5$', r'', r'$0.75$', r'', r'$1.0$'], rotation=0, size=35)
    ax3a.set_xlim(0, 1)
    ax3a.set_ylim(0, 1)
    # ax3.tick_params(axis='y', labelsize=35)
    ax3a.xaxis.grid(True, linewidth=2)
    plt.savefig(workingDirectory + '/' + method + 'CalibrationB0B0bar.pdf')
    fig3a.clear()

    fig3 = plt.figure(6, figsize=(11, 8))
    ax3 = plt.axes([0.21, 0.15, 0.74, 0.8])  # plt.axes([0.14, 0.1, 0.8, 0.8])

    line = range(0, 2, 1)
    ax3.plot(line, line, linewidth=4, color='r')

    (_, caps, _) = ax3.errorbar(rvalue, rmvalues, xerr=None, yerr=None,
                                # xerr=delta_rvalue, yerr=delta_rmvalues,
                                elinewidth=4, capsize=10, ecolor='b', fmt='o', markersize=14)  # histtype='step'
    for cap in caps:
        # cap.set_color('red')
        cap.set_markeredgewidth(4)

    ax3.set_ylabel(r'$r_{\rm \small MC} = 1-2\cdot w_{\small \rm MC}$', fontsize=42)
    ax3.set_xlabel(r'$\langle r_{\rm ' + label + '}$' + r'$\rangle$', fontsize=42)
    # plt.xticks([0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1], [r'$0$', r'', r'$0.2$',
    # r'', r'$0.4$', r'', r'$0.6$', r'', r'$0.8$', r'', r'$1.0$'],
    # rotation=0, size=35)
    plt.xticks([0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1],
               [r'$0$', r'$0.1$', r'$0.25$', r'$0.5$', r'', r'$0.75$', r'', r'$1.0$'], rotation=0, size=35)
    plt.yticks([0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1],
               [r'$0$', r'$0.1$', r'$0.25$', r'$0.5$', r'', r'$0.75$', r'', r'$1.0$'], rotation=0, size=35)
    ax3.set_xlim(0, 1)
    ax3.set_ylim(0, 1)
    # ax3.tick_params(axis='y', labelsize=35)
    ax3.xaxis.grid(True, linewidth=2)
    plt.savefig(workingDirectory + '/' + method + 'Calibration.pdf')
    fig3.clear()

    # -- R output plot

    fig4 = plt.figure(7, figsize=(11, 8))
    ax4 = plt.axes([0.21, 0.15, 0.76, 0.8])
    ax4.hist(np.absolute(data[:, 1]), bins=bins, facecolor='r', histtype='stepfilled', edgecolor='r')  # histtype='step'
    ax4.set_ylabel(r'${\rm Number\ \ of\ \ Events\ /\ (\, 0.02\, )}$', fontsize=35)
    ax4.set_xlabel(r'$r_{\rm ' + label + ' }$', fontsize=42)
    ax4.tick_params(axis='y', labelsize=30)
    # ax4.axvline(0.1, linestyle='--', color='k', linewidth=2)
    # ax4.axvline(0.25, linestyle='--', color='k', linewidth=2)
    # ax4.axvline(0.5, linestyle='--', color='k', linewidth=2)
    # ax4.axvline(0.625, linestyle='--', color='k', linewidth=2)
    # ax4.axvline(0.75, linestyle='--', color='k', linewidth=2)
    # ax4.axvline(0.875, linestyle='--', color='k', linewidth=2)
    ax4.set_xlim(0, 1)
    # ax4.set_ylim(0, 1.4)
    plt.xticks([0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1],
               [r'$0$', r'$0.1$', r'$0.25$', r'$0.5$', r'$0.625$', r'$0.75$', r'$0.875$', r'$1$'], rotation=0, size=30)
    ax4.yaxis.grid(True)
    ax4.xaxis.grid(True, linewidth=2)
    plt.savefig(workingDirectory + '/' + method + 'R_Output.pdf')
    fig4.clear()

    # ----

    # QR Plot folded

    fig2a = plt.figure(8, figsize=(11, 8))
    ax2a = plt.axes([0.21, 0.15, 0.76, 0.8])

    ax2a.hist(data[np.where((abs(data[:, 0]) == 1) & (data[:, 1] >= 0))][:, 1], bins=bins, histtype='step', edgecolor='k',
              linewidth=4, linestyle='dashdot', alpha=0.9, label=r'$q \geq 0 $')

    ax2a.hist(data[np.where((abs(data[:, 0]) == 1) & (data[:, 1] < 0))][:, 1] * (-1), bins=bins, histtype='step', edgecolor='g',
              linewidth=4, linestyle='dashed', alpha=0.9, label=r'$q < 0 $')

    p3a, =  ax2a.plot([], label=r'$q \geq 0 $', linewidth=4, linestyle='dashdot', alpha=0.9, c='k')

    p2a, =  ax2a.plot([], label=r'$q < 0 $', linewidth=4.5, linestyle='dashed', c='g')

    ax2a.set_ylabel(r'${\rm Number\ \ of\ \ Events\ /\ (\, 0.02\, )}$', fontsize=35)
    ax2a.set_xlabel(r'$(q\cdot q\cdot r)_{\rm ' + label + '}$', fontsize=42)
    # plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1], [r'$-1$', r'',
    # r'$-0.5$', r'', r'$0$', r'', r'$0.5$', r'', r'$1$'], rotation=0,
    # size=40)
    ax2a.tick_params(axis='y', labelsize=35)
    ax2a.legend([p3a, p2a], [r'$q \geq 0 $', r'$q < 0 $'], prop={'size': 35}, loc=2)
    ax2a.grid(True)
    plt.xticks([0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1],
               [r'$0$', r'$0.1$', r'$0.25$', r'$0.5$', r'', r'$0.75$', r'', r'$1.0$'], rotation=0, size=35)

    ax2a.set_xlim(0, 1)
    ax2a.set_ylim(0, YmaxForQrPlot)
    plt.savefig(workingDirectory + '/' + method + 'Folded_QR_Both.pdf')
    fig2a.clear()

    # QR Plot folded for each MC flavor

    #    fig2c = plt.figure(2, figsize=(11, 8))
    #    ax2c = plt.axes([0.21, 0.15, 0.76, 0.8])
    #
    #    nB0bar, nbins, patches = ax2c.hist(data[np.where(data[:, 0] == -1)][:, 1]*(-1), bins=bins, histtype='step',
    #             edgecolor='b', linewidth=4.5, linestyle='dashed', label=r'$\bar{B}^0$')  # hatch='.', 'dotted'
    #
    #    nB0, nbins, patches = ax2c.hist(data[np.where(data[:, 0] == 1)][:, 1], bins=bins, histtype='step', edgecolor='r',
    #             linewidth=4, alpha=0.9, label=r'$B^0$')  # histtype='step', hatch='\\'
    #
    #    p2c, =  ax2c.plot([], label=r'$\bar{B}^0$', linewidth=4.5, linestyle='dashed', c='b')
    #
    #    p1c, =  ax2c.plot([], label=r'$B^0$', linewidth=4, linestyle='solid', alpha=0.9, c='r')
    #
    #    ax2c.set_ylabel(r'${\rm Number\ \ of\ \ Events\ /\ (\, 0.02\, )}$', fontsize=35)
    #    ax2c.set_xlabel(r'$q_{\rm MC}\cdot(q\cdot r)_{\rm ' + label + '}$', fontsize=42)
    #    #plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1],
    #                [r'$-1$', r'', r'$-0.5$', r'', r'$0$', r'', r'$0.5$', r'', r'$1$'], rotation=0, size=40)
    #    ax2c.tick_params(axis='y', labelsize=35)
    #    ax2c.legend([ p2c, p1c], [r'$\bar{B}^0\, (q_{\rm MC} = -1)$', r'$B^0\, (q_{\rm MC} = +1)$'], prop={'size': 35}, loc=2)
    #    ax2c.grid(True)
    #    plt.xticks([-1, -0.875, -0.75, -0.625, -0.5, -0.25, -0.1, 0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1],
    #               [r'${-1.0}$', r'',  r'', r'', r'${-0.5}$', r'', r'', r'$0$', r'',
    #               r'', r'$0.5$', r'', r'', r'', r'$1.0$'], rotation=0, size=35)
    #
    #    ax2c.set_ylim(0, YmaxForQrPlot)
    #    plt.savefig(workingDirectory + '/' + method + '_Folded_QR_B0bar.pdf')
    #    fig2c.clear()

    # QR Plot folded for each MC flavor with Asymmetry -----------------------------

    fig2e = plt.figure(9, figsize=(11, 11))
    ax2e = plt.axes([0.21, 0.37, 0.76, 0.60])

    nB0bar, nbins, patches = ax2e.hist(data[np.where(data[:, 0] == -1)][:, 1] * (-1), bins=bins, histtype='step',
                                       edgecolor='b', linewidth=4.5, linestyle='dashed', label=r'$\bar{B}^0$')

    nB0, nbins, patches = ax2e.hist(data[np.where(data[:, 0] == 1)][:, 1], bins=bins, histtype='step', edgecolor='r',
                                    linewidth=4, alpha=0.7, label=r'$B^0$')  # histtype='step', hatch='\\'

    p2c, =  ax2e.plot([], label=r'$\bar{B}^0$', linewidth=4, linestyle='dashed', c='b')

    p1c, =  ax2e.plot([], label=r'$B^0$', linewidth=4.5, linestyle='solid', alpha=0.9, c='r')

    ax2e.set_ylabel(r'${\rm Events\ /\ (\, 0.02\, )}$', fontsize=35)

    # plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1], [r'$-1$', r'',
    # r'$-0.5$', r'', r'$0$', r'', r'$0.5$', r'', r'$1$'], rotation=0,
    # size=40)
    ax2e.tick_params(axis='y', labelsize=35)
    ax2e.legend([p2c, p1c], [r'$\bar{B}^0\, (q_{\rm MC} = -1)$', r'$B^0\, (q_{\rm MC} = +1)$'], prop={'size': 35}, loc=2)
    ax2e.grid(True)
    plt.xticks([-1, -0.875, -0.75, -0.625, -0.5, -0.25, -0.1, 0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1],
               [r'', r'', r'', r'', r'', r'', r'', r'', r'',
                r'', r'', r'', r'', r'', r''], rotation=0, size=35)

    plt.yticks(yLocs, yLabels)
    ax2e.set_ylim(0, YmaxForQrPlot)

    # Asymm  --

    np.seterr(divide='ignore', invalid='ignore')
    asymmetryArray = (nB0 - nB0bar) / (nB0 + nB0bar)
    asymmetryArrayUncertainties = np.zeros(asymmetryArray.shape)

    for i in range(0, nB0.shape[0]):
        denominator = nB0[i] + nB0bar[i]
        if denominator != 0:
            asymmetryArrayUncertainties[i] = 2 * math.sqrt(
                (nB0[i] * math.sqrt(nB0[i])) ** 2 +
                (nB0bar[i] * math.sqrt(nB0bar[i]))**2) / (denominator)**2
    # print(asymmetryArray)

    nBinCenters = 0.5 * (nbins[1:] + nbins[:-1])

    ax2eA = plt.axes([0.21, 0.15, 0.76, 0.2])

    # ax2eA.plot(nBinCenters, asymmetryArray, color='k',
    #        linewidth=4, linestyle='dashdot', alpha=0.9, drawstyle='steps')

    ax2eA.errorbar(nBinCenters, asymmetryArray, xerr=float(0.01),
                   yerr=asymmetryArrayUncertainties, elinewidth=2, mew=2, ecolor='k',
                   fmt='o', mfc='k', mec='k', markersize=3, label=r'${\rm Data}$')

    ax2eA.set_ylabel(r'$\frac{N^{B^0}\; -\;\, N^{\overline{B}^0}}{N^{B^0}\; +\;\, N^{\overline{B}^0}}$', fontsize=44, labelpad=20)
    ax2eA.yaxis.labelpad = 0
    plt.yticks([-0.4, -0.2, 0, 0.2, 0.4],
               [r'', r'$-0.2$', r'$0.0$', r'$0.2$', r''], rotation=0, size=28)

    ax2eA.set_ylim(-0.4, 0.4)
    ax2eA.set_xlim(ax2e.get_xlim())
    # ax2.tick_params(axis='y', labelsize=30)
    ax2eA.xaxis.grid(True)  # linestyle='--'
    ax2eA.yaxis.grid(True)
    # plt.axhline(y= 4, xmin=-1.005, xmax=1.005, linewidth=1, color = 'k', linestyle = '-')
    # plt.axhline(y= 0.75, linewidth=2, color = 'tab:gray', linestyle = '-')
    # plt.axhline(y= 2, xmin=limXmin, xmax=limXmax, linewidth=1, color = 'k', linestyle = '-')
    # plt.axhline(y= 0.5, linewidth=2, color = 'tab:gray', linestyle = '-')
    # plt.axhline(y= 0.25, linewidth=2, color = 'tab:gray', linestyle = '-')
    # plt.axhline(y= 0, linewidth=2, color = 'tab:gray', linestyle = '-')
    # plt.axhline(y= -0.25, linewidth=2, color = 'tab:gray', linestyle = '-')
    # plt.axhline(y= -0.5, linewidth=2, color = 'tab:gray', linestyle = '-')

    plt.xticks([-1, -0.875, -0.75, -0.625, -0.5, -0.25, -0.1, 0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1],
               [r'${-1.0}$', r'', r'', r'', r'${-0.5}$', r'', r'', r'$0$', r'',
                r'', r'$0.5$', r'', r'', r'', r'$1.0$'], rotation=0, size=35)

    ax2eA.set_xlabel(r'$q_{\rm MC}\cdot(q\cdot r)_{\rm ' + label + '}$', fontsize=42)
    plt.savefig(workingDirectory + '/' + method + '_Folded_QR_B0barWithAsymm.pdf')
    fig2e.clear()

    # R Plot for B0 and B0bar ----------------------------------------------------------

    fig2d = plt.figure(11, figsize=(11, 8))
    ax2d = plt.axes([0.21, 0.15, 0.76, 0.8])

    # ax2d.hist(np.absolute(data[np.where(abs(data[:, 0]) == 1)][:, 1]), bins=bins, histtype='step', edgecolor='k',
    #        linewidth=4, linestyle='dashdot', alpha=0.9, label=r'${\rm Both}$')

    ax2d.hist(np.absolute(data[np.where(data[:, 0] == -1)][:, 1]), bins=bins, histtype='step',
              edgecolor='b', linewidth=4.5, linestyle='dashed', label=r'$\bar{B}^0$')  # hatch='.', 'dotted'

    ax2d.hist(np.absolute(data[np.where(data[:, 0] == 1)][:, 1]), bins=bins, histtype='step', edgecolor='r',
              linewidth=4, alpha=0.9, label=r'$B^0$')  # histtype='step', hatch='\\'

    # p3d, =  ax2.plot([], label=r'${\rm Both}$', linewidth=4, linestyle='dashdot', alpha=0.9, c='k')

    p2d, =  ax2d.plot([], label=r'$\bar{B}^0$', linewidth=4.5, linestyle='dashed', c='b')

    p1d, =  ax2d.plot([], label=r'$B^0$', linewidth=4, linestyle='solid', alpha=0.9, c='r')

    ax2d.set_ylabel(r'${\rm Number\ \ of\ \ Events\ /\ (\, 0.02\, )}$', fontsize=35)
    ax2d.set_xlabel(r'$r_{\rm ' + label + '}$', fontsize=42)
    # plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1], [r'$-1$', r'',
    # r'$-0.5$', r'', r'$0$', r'', r'$0.5$', r'', r'$1$'], rotation=0,
    # size=40)
    ax2d.tick_params(axis='y', labelsize=35)
    ax2d.legend([p2d, p1d], [r'$\bar{B}^0$', r'$B^0$'], prop={'size': 35}, loc=2)
    ax2d.grid(True)
    plt.xticks([0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1],
               [r'$0$', r'$0.1$', r'$0.25$', r'$0.5$', r'', r'$0.75$', r'', r'$1.0$'], rotation=0, size=35)

    ax2d.set_xlim(0, 1)
    ax2d.set_yscale('log', nonposy='clip')
    # ax2d.set_ylim(0, YmaxForQrPlot)
    plt.savefig(workingDirectory + '/' + method + '_Folded_R_B0bar.pdf')
    fig2d.clear()

    # --- Wrong tag fraction plot ----------------------------- ##

    fig5 = plt.figure(12, figsize=(11, 15))
    ax5 = plt.axes([0.21, 0.53, 0.76, 0.45])

    rBinCenters = 0.5 * (rbins[1:] + rbins[:-1])
    rBinWidths = 0.5 * (rbins[1:] - rbins[:-1])

    eb = ax5.errorbar(rBinCenters, 50 * (wvalueB0bar + wvalueB0), xerr=rBinWidths, yerr=50 * wvalueDiffUncertainty,
                      label=r'${\rm Average}$', elinewidth=4.5, ecolor='k', fmt='o', mfc='k', mec='k',
                      mew=0, markersize=0)
    eb[-1][0].set_linestyle('dashdot')
    eb[-1][0].set_alpha(0.9)

    ebB0bar = ax5.errorbar(rBinCenters, 100 * wvalueB0, xerr=rBinWidths, yerr=50 * wvalueDiffUncertainty,
                           label=r'$B^0$', elinewidth=4, ecolor='r', fmt='o', mfc='k', mec='k',
                           mew=0, markersize=0)
    ebB0bar[-1][0].set_alpha(0.9)

    ebB0bar = ax5.errorbar(rBinCenters, 100 * wvalueB0bar, xerr=rBinWidths, yerr=50 * wvalueDiffUncertainty,
                           label=r'$\bar{B}^0$', elinewidth=4.5, ecolor='b', fmt='o', mfc='k', mec='k',
                           mew=0, markersize=0)
    ebB0bar[-1][0].set_linestyle('--')

    ax5.set_ylabel(r'$w\ [\%]$', fontsize=45)
    ax5.yaxis.labelpad = 36
    ax5.set_ylim(0, 55)
    ax5.set_xlim(0, 1)
    plt.yticks([0, 10, 20, 30, 40, 50], [r'$0$',
                                         r'$10$', r'$20$', r'$30$', r'$40$', r'$50$'], rotation=0, size=44)
    plt.xticks([0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1], [r'', r'',
                                                            r'', r'', r'', r'', r'', r''], rotation=0, size=44)

    p3d, =  ax5.plot([], label=r'${\rm Average}$', linewidth=4, linestyle='dashdot', alpha=0.9, c='k')

    p2d, =  ax5.plot([], label=r'$\bar{B}^0$', linewidth=4.5, linestyle='dashed', c='b')

    p1d, =  ax5.plot([], label=r'$B^0$', linewidth=4, linestyle='solid', alpha=0.9, c='r')

    ax5.legend([p3d, p2d, p1d], [r'${\rm Average}$', r'$\bar{B}^0$', r'$B^0$'], prop={'size': 38}, loc=1)
    ax5.grid(True)

    #  Asymm plot ----

    ax5A = plt.axes([0.21, 0.08, 0.76, 0.43])

    ax5A.errorbar(rBinCenters, 100 * wvalueDiff, xerr=rBinWidths,
                  yerr=100 * wvalueDiffUncertainty, elinewidth=3, mew=2, ecolor='k',
                  fmt='o', mfc='k', mec='k', markersize=3, label=r'${\rm Data}$')

    ax5A.set_ylim(-14, 14)
    ax5A.set_xlim(0, 1)
    plt.yticks([-14, -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14],
               [r'', r'$-12$', r'', r'$-8$', r'', r'$-4$', r'', r'$0$',
                r'', r'$4$', r'', r'$8$', r'', r'$12$', r''], rotation=0, size=44)

    plt.xticks([0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1],
               [r'$0$', r'$0.1$', r'$0.25$', r'$0.5$', r'', r'$0.75$', r'', r'$1.0$'], rotation=0, size=42)

    ax5A.xaxis.grid(True)  # linestyle='--'
    ax5A.yaxis.grid(True)
    ax5A.set_ylabel(r'$\Delta w\ [\%]$', fontsize=45)
    ax5A.set_xlabel(r'$r_{\rm ' + label + '}$', fontsize=45)

    plt.savefig(workingDirectory + '/' + method + '_WrongTagFraction.pdf')
    fig5.clear()

    # --- Effiency and Mu plot ----------------------------- ##

    fig6 = plt.figure(13, figsize=(11, 15))
    ax6 = plt.axes([0.21, 0.53, 0.76, 0.45])

    rBinCenters = 0.5 * (rbins[1:] + rbins[:-1])
    rBinWidths = 0.5 * (rbins[1:] - rbins[:-1])

    eb = ax6.errorbar(rBinCenters, 50 * (event_fractionB0bar + event_fractionB0), xerr=rBinWidths, yerr=50 * muParamUncertainty,
                      label=r'${\rm Average}$', elinewidth=4.5, ecolor='k', fmt='o', mfc='k', mec='k',
                      mew=0, markersize=0)
    eb[-1][0].set_linestyle('dashdot')
    eb[-1][0].set_alpha(0.9)

    ebB0bar = ax6.errorbar(rBinCenters, 100 * event_fractionB0bar, xerr=rBinWidths, yerr=50 * muParamUncertainty,
                           label=r'$B^0$', elinewidth=4, ecolor='r', fmt='o', mfc='k', mec='k',
                           mew=0, markersize=0)
    ebB0bar[-1][0].set_alpha(0.9)

    ebB0bar = ax6.errorbar(rBinCenters, 100 * event_fractionB0, xerr=rBinWidths, yerr=50 * muParamUncertainty,
                           label=r'$\bar{B}^0$', elinewidth=4.5, ecolor='b', fmt='o', mfc='k', mec='k',
                           mew=0, markersize=0)
    ebB0bar[-1][0].set_linestyle('--')

    ax6.set_ylabel(r'$\varepsilon\ [\%]$', fontsize=45)
    ax6.yaxis.labelpad = 36
    ax6.set_ylim(0, 30)
    ax6.set_xlim(0, 1)
    plt.yticks([0, 5, 10, 15, 20, 25, 30], [r'$0$', r'',
                                            r'$10$', r'', r'$20$', r'', r'$30$'], rotation=0, size=45)
    plt.xticks([0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1], [r'', r'',
                                                            r'', r'', r'', r'', r'', r''], rotation=0, size=44)

    p3d, =  ax6.plot([], label=r'${\rm Average}$', linewidth=4, linestyle='dashdot', alpha=0.9, c='k')

    p2d, =  ax6.plot([], label=r'$\bar{B}^0$', linewidth=4.5, linestyle='dashed', c='b')

    p1d, =  ax6.plot([], label=r'$B^0$', linewidth=4, linestyle='solid', alpha=0.9, c='r')

    ax6.legend([p3d, p2d, p1d], [r'${\rm Average}$', r'$\bar{B}^0$', r'$B^0$'], prop={'size': 38}, loc=1)
    ax6.grid(True)

    #  Asymm plot ----

    ax6A = plt.axes([0.21, 0.08, 0.76, 0.43])

    ax6A.errorbar(rBinCenters, 100 * muParam, xerr=rBinWidths,
                  yerr=100 * muParamUncertainty, elinewidth=3, mew=2, ecolor='k',
                  fmt='o', mfc='k', mec='k', markersize=3, label=r'${\rm Data}$')

    ax6A.set_ylim(-4, 4)
    ax6A.set_xlim(0, 1)
    plt.yticks([-3.5, -3, -2.5, -2, -1.5, -1, -0.5, 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5],
               [r'', r'$-3$', r'', r'$-2$', r'', r'$-1$', r'', r'$0$', r'', r'$1$', r'', r'$2$', r'', r'$3$', r''],
               rotation=0, size=45)

    plt.xticks([0, 0.1, 0.25, 0.5, 0.625, 0.75, 0.875, 1],
               [r'$0$', r'$0.1$', r'$0.25$', r'$0.5$', r'', r'$0.75$', r'', r'$1.0$'], rotation=0, size=42)

    ax6A.xaxis.grid(True)  # linestyle='--'
    ax6A.yaxis.grid(True)
    ax6A.set_ylabel(r'$\mu\ [\%]$', fontsize=45)
    ax6A.set_xlabel(r'$r_{\rm ' + label + '}$', fontsize=45)

    ax6A.yaxis.labelpad = 25

    plt.savefig(workingDirectory + '/' + method + '_EfficiencyAndMu.pdf')
    fig6.clear()

    # ----  TrueCategories ------------------------------------------------

    fig7 = plt.figure(14, figsize=(11, 8))
    ax7 = plt.axes([0.21, 0.15, 0.76, 0.8])
    weights = np.ones_like(numberOfTrueCatagories) * 100 / float(len(numberOfTrueCatagories))
    hi5, w, _ = ax4.hist(numberOfTrueCatagories, weights=weights, bins=list(
        range(0, 14, 1)), facecolor='r', histtype='stepfilled', edgecolor='k')
    # print(hi5)
    ax7.bar(np.array(range(0, 13, 1)) + 0.1, hi5, 0.8, color='r', ecolor='k')
    ax7.set_ylabel(r'${\rm Percentage\ of\ Events\ /\ (\, 0.02\, )}$', fontsize=25)
    ax7.set_xlabel(r'${\rm True\ Categories}$', fontsize=25)
    plt.xticks([0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5],
               [r'$0$', r'$1$', r'$2$', r'$3$', r'$4$', r'$5$', r'$6$', r'$7$', r'$8$'],
               rotation=0, horizontalalignment='center', size=25)
    ax7.tick_params(axis='y', labelsize=25)
    ax7.set_xlim(0, 8)
    ax7.set_ylim(0, 40)
    # ax7.grid(True)
    plt.savefig(workingDirectory + '/' + method + 'TrueCategories.pdf')
    fig7.clear()

    fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(111, projection='3d')

    # -----bins
    bins2 = list(range(-51, 51, 1))
    for i in range(0, len(bins2)):
        bins2[i] = float(bins2[i]) / 50
    # ------
    bins2 = np.array(bins2)
    # bins2 = np.arange(-1, 0.5, 0.02)

    def cc(arg):
        return colorConverter.to_rgba(arg, alpha=0.5)

    vertsa = []
    vertsb = []
    colorsa = []
    colorsb = []
    zs = list(range(0, 7, 1))

    Zmax = 0

    iEfficiencies = []

    for z in range(0, 7, 1):
        dataTrueCats = dataAll[(np.where(dataAll[:, 2] == z))][:, 0:2]
        weightsa = np.ones_like(dataTrueCats[np.where(dataTrueCats[:, 0] == -1)][:, 1]) * \
            (hi5[z] * dataAll.shape[0] / 100) / float(len(dataTrueCats[:, 1]))
        weightsb = np.ones_like(dataTrueCats[np.where(dataTrueCats[:, 0] == 1)][:, 1]) * \
            (hi5[z] * dataAll.shape[0] / 100) / float(len(dataTrueCats[:, 1]))
        ya, wa, _ = ax2.hist(dataTrueCats[np.where(dataTrueCats[:, 0] == -1)][:, 1], weights=weightsa, bins=bins, histtype='step',
                             edgecolor='b', linewidth=3.5, linestyle='dotted', label=r'$\bar{B}^0$')
        yb, wb, _ = ax2.hist(dataTrueCats[np.where(dataTrueCats[:, 0] == 1)][:, 1], weights=weightsb, bins=bins, histtype='step',
                             edgecolor='r', linewidth=2, label=r'$B^0$')
        ya = np.insert(ya, ya.shape[0], 0)
        ya = np.insert(ya, 0, 0)
        yb = np.insert(yb, yb.shape[0], 0)
        yb = np.insert(yb, 0, 0)
        if ya.max() > Zmax:
            Zmax = ya.max()
        if yb.max() > Zmax:
            Zmax = yb.max()
        colorsa.append(cc('b'))
        colorsb.append(cc('r'))
        vertsa.append(list(zip(bins2, ya)))
        vertsb.append(list(zip(bins2, yb)))

        iEfficiency, iEfficiencyDiff, iEfficiencyB0, iEfficiencyB0bar = efficiencyCalculator(dataTrueCats, total_notTagged, True)

        iEfficiencies.append([iEfficiencyB0, iEfficiencyB0bar])

    polya = PolyCollection(vertsa, facecolors=colorsa)
    polyb = PolyCollection(vertsb, facecolors=colorsb)
    polya.set_alpha(0.5)
    polyb.set_alpha(0.5)
    ax.add_collection3d(polya, zs=zs, zdir='y')
    ax.add_collection3d(polyb, zs=zs, zdir='y')

    # for z in zs:
    #    ya, wa , _  = ax2b.hist(dataNoTrueCategory[np.where(dataNoTrueCategory[:, 0] == -1)][:, 1], bins=bins, histtype='step',
    #              edgecolor='b', linewidth=2, label=r'$\bar{B}^0$')
    #    ax.bar(bins2, bins2, zs=z, zdir = 'y', color='b', ecolor = 'b', alpha=0.75)
    # print(bins2)

    ax.set_xlim3d(-1, 1)
    ax.set_ylim3d(-0.5, 7)
    ax.set_yticks([0, 1, 2, 3, 4, 5, 6])
    ax.xaxis.labelpad = 18
    ax.yaxis.labelpad = 16
    ax.zaxis.labelpad = 22
    # ax.tick_params(axis = 'y', right = 'on')
    ax.set_yticklabels(sorted({r'$0$', r'$1$', r'$2$', r'$3$', r'$4$', r'$5$', r'$6$'}),
                       verticalalignment='baseline',
                       horizontalalignment='left')

    ax.set_xlabel(r'$(q\cdot r)_{\rm ' + label + ' }$')
    ax.set_ylabel(r'${\rm True\ Categories}$')
    ax.set_zlabel(r'${\rm Number\ \ of\ \ Events\ /\ (\, 0.02\, )}$', rotation=-90)
    Zmax = Zmax + Zmax / 5
    ax.set_zlim3d(0, Zmax)
    p1 = plt.Rectangle((0, 0), 1, 1, fc="b", alpha=0.5)
    p2 = plt.Rectangle((0, 0), 1, 1, fc="r", alpha=0.5)
    ax.legend([p1, p2], [r'$\bar{B}^0$', r'$B^0$'], prop={'size': 20}, loc=(0.523, 0.687))  # (0.639, 0.71
    plt.savefig(workingDirectory + '/3DTrueCategorieQR' + method + '.pdf')
    fig.clear()

    iEfficiencies = np.array(iEfficiencies)

    fig = plt.figure(15, figsize=(12, 8))
    ax = plt.axes([0.18, 0.15, 0.8, 0.8])
    # print(hi5)
    width = 0.4
    p1 = ax.bar(np.array(range(0, 7, 1)) + width / 4, iEfficiencies[:, 1], width, color='b', ecolor='k', label=r'$\bar{B}^0$')
    p2 = ax.bar(np.array(range(0, 7, 1)) + width + width / 4, iEfficiencies[:, 0], width, color='r', ecolor='k', label=r'$B^0$')
    ax.set_ylabel(r'$ \varepsilon_{\rm eff}/\  \% $', fontsize=35)
    ax.set_xlabel(r'${\rm True\ Categories}$', fontsize=35)
    plt.xticks([0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5],
               [r'$0$', r'$1$', r'$2$', r'$3$', r'$4$', r'$5$', r'$6$', r'$7$', r'$8$'],
               rotation=0, horizontalalignment='center', size=35)
    ax.tick_params(axis='y', labelsize=35)
    ax.set_xlim(0, 8)
    ax.set_ylim(0, 100)
    ax.legend([r'$\bar{B}^0$', r'$B^0$'], prop={'size': 35})

    plt.savefig(workingDirectory + '/EfficienciesTrueCats' + method + '.pdf')
    fig.clear()


NoTargetEfficiencies = []
categoryLabel = []

categoryLabelsDict = {'Electron': r'${\rm Electron}$',
                      'IntermediateElectron': r'${\rm Int. Electron}$',
                      'Muon': r'${\rm Muon}$',
                      'IntermediateMuon': r'${\rm Int. Muon}$',
                      'KinLepton': r'${\rm KinLepton}$',
                      'IntermediateKinLepton': r'${\rm Int. KinLepton}$',
                      'Kaon': r'${\rm Kaon}$',
                      'SlowPion': r'${\rm Slow Pion}$',
                      'FastHadron': r'${\rm Fast Hadron}$',
                      'Lambda': r'${\rm Lambda}$',
                      'FSC': r'${\rm FSC}$',
                      'MaximumPstar': r'${\rm MaximumPstar}$',
                      'KaonPion': r'${\rm Kaon-Pion}$'}

# eventLevelParticleLists = [ ('Lambda0:inRoe', 'Lambda',
# 'weightedQpOf(Lambda0:inRoe, isRightCategory(Lambda),
# isRightCategory(Lambda))') ]

# print(eventLevelParticleLists)

# eventLevelParticleLists = [
#     ('e+:inRoe', 'Electron',
#      'QpOf(e+:inRoe, isRightCategory(Electron), isRightCategory(Electron))'),
#     ('e+:inRoe', 'IntermediateElectron',
#      'QpOf(e+:inRoe, isRightCategory(IntermediateElectron), isRightCategory(IntermediateElectron))'),
#     ('mu+:inRoe', 'Muon',
#      'QpOf(mu+:inRoe, isRightCategory(Muon), isRightCategory(Muon))'),
#     ('mu+:inRoe', 'IntermediateMuon',
#      'QpOf(mu+:inRoe, isRightCategory(IntermediateMuon), isRightCategory(IntermediateMuon))'),
#     ('mu+:inRoe', 'KinLepton',
#      'QpOf(mu+:inRoe, isRightCategory(KinLepton), isRightCategory(KinLepton))'),
#     ('mu+:inRoe', 'IntermediateKinLepton',
#      'QpOf(mu+:inRoe, isRightCategory(IntermediateKinLepton), isRightCategory(IntermediateKinLepton))'),
#     ('K+:inRoe', 'Kaon',
#      'weightedQpOf(K+:inRoe, isRightCategory(Kaon), isRightCategory(Kaon))'),
#     ('K+:inRoe', 'KaonPion',
#      'QpOf(K+:inRoe, isRightCategory(KaonPion), isRightCategory(Kaon))'),
#     ('pi+:inRoe', 'SlowPion', 'QpOf(pi+:inRoe, isRightCategory(SlowPion), isRightCategory(SlowPion))'),
#     ('pi+:inRoe', 'FSC', 'QpOf(pi+:inRoe, isRightCategory(FSC), isRightCategory(SlowPion))'),
#     ('pi+:inRoe', 'MaximumPstar',
#      'QpOf(pi+:inRoe, isRightCategory(MaximumPstar), isRightCategory(MaximumPstar))'),
#     ('pi+:inRoe', 'FastHadron',
#      'QpOf(pi+:inRoe, isRightCategory(FastHadron), isRightCategory(FastHadron))'),
#     ('Lambda0:inRoe', 'Lambda',
#      'weightedQpOf(Lambda0:inRoe, isRightCategory(Lambda), isRightCategory(Lambda))')]


qrMC = ROOT.RooRealVar('qrMC', 'qrMC', 0, -1.0, 1.0)
argSet = ROOT.RooArgSet(qrMC)

for iCategory in usedCategories:
    #
    with Quiet(ROOT.kError):
        exec(
            "%s = %s" %
            ("hasTrueTarget" +
             iCategory,
             "ROOT.RooRealVar('hasTrueTarget' + iCategory, 'hasTrueTarget' + iCategory, 0, -2.1, 1.1)"))
        exec(
            "%s = %s" %
            ("qp" +
             iCategory,
             "ROOT.RooRealVar('qp' + iCategory, 'hasTrueTarget' + iCategory, 0, -2.1, 1.1)"))
        # exec("%s = %s" % ("isTrueCategory" + iCategory,
        # "ROOT.RooRealVar('isRightCategory' + iCategory, 'isRightCategory'
        # + iCategory, 0, -2.0, 1.1)"))
        exec("%s" % "argSet.add(hasTrueTarget" + iCategory + ")")
        exec("%s" % "argSet.add(qp" + iCategory + ")")
rooDataSet = ROOT.RooDataSet("data", "data", tree, argSet, "")

for category in usedCategories:

    print(category)
    # qrCombined=ROOT.RooRealVar('qp' + category, 'qp' + category, 0, -1.0, 1.1)

    data = []

    dataTruth = []

    dataNoTarget = []

    for i in range(rooDataSet.numEntries()):
        row = rooDataSet.get(i)
        tqp = row.getRealValue('qp' + category, 0, ROOT.kTRUE)
        thasTrueTarget = row.getRealValue('hasTrueTarget' + category, 0, ROOT.kTRUE)
        tqrMC = row.getRealValue('qrMC', 0, ROOT.kTRUE)

        tNumberOfTrueCategories = 0
        noMCAssociated = False

        for iCategory in usedCategories:

            if iCategory == "MaximumPstar":
                continue
            ihasTrueTarget = row.getRealValue('hasTrueTarget' + iCategory, 0, ROOT.kTRUE)
            # print(ihasTrueTarget)

            if ihasTrueTarget > 0:
                tNumberOfTrueCategories += 1
            if ihasTrueTarget < 0:
                noMCAssociated = True
                break

        if tqp > 1:
            tqp = 1.0

        # if thasTrueTarget < 1:
            # print(thasTrueTarget)

        if abs(tqrMC) == 1:  # !=0:
            data.append([tqrMC, tqp])
            if thasTrueTarget > 0:
                dataTruth.append([tqrMC, tqp])
            if tNumberOfTrueCategories < 1 and not noMCAssociated:
                # thasTrueTarget < 1:#if thasTrueTarget == 0 and not noMCAssociated:
                if category == "Lambda" and tqp == 0:
                    continue
                # print(thasTrueTarget)
                dataNoTarget.append([tqrMC, tqp])

    data = np.array(data)
    dataTruth = np.array(dataTruth)
    dataNoTarget = np.array(dataNoTarget)

    # print(dataNoTarget)

    noTargetEfficiency, noTargetEfficiencyDiff, noTargetEfficiencyB0, noTargetEfficiencyB0bar = categoriesEfficiencyCalculator(data)
    print("Efficiencies for B0, B0bar = ", '{: 6.2f}'.format(noTargetEfficiencyB0),
          " %, ", '{: 6.2f}'.format(noTargetEfficiencyB0bar), " %")

    noTargetEfficiency, noTargetEfficiencyDiff, noTargetEfficiencyB0, noTargetEfficiencyB0bar = categoriesEfficiencyCalculator(
        dataNoTarget)
    print("Efficiencies for B0, B0bar If Not Target = ", '{: 6.2f}'.format(
        noTargetEfficiencyB0), " %, ", '{: 6.2f}'.format(noTargetEfficiencyB0bar), " %")

    NoTargetEfficiencies.append([noTargetEfficiencyB0, noTargetEfficiencyB0bar])
    categoryLabel.append(categoryLabelsDict[category])

    trueTargetEfficiency, trueTargetEfficiencyDiff, trueTargetEfficiencyB0, \
        trueTargetEfficiencyB0bar = categoriesEfficiencyCalculator(dataTruth)
    print("Efficiencies for B0, B0bar If True Target= ", '{: 6.2f}'.format(
        trueTargetEfficiencyB0), " %, ", '{: 6.2f}'.format(trueTargetEfficiencyB0bar), " %")

    # catLabel = r"${\rm " + category + "}$"
    # categoryLabel.append(catLabel)

    # -----bins
    bins = list(range(-50, 51, 1))
    for i in range(0, len(bins)):
        bins[i] = float(bins[i]) / 50
    # ------

    title = str()
    location = 1
    if category != 'Lambda' and category != 'MaximumPstar' and category != 'Kaon':
        title = r'$q_{\rm cand}\cdot y_{\rm ' + category + '}$'
    elif category == 'MaximumPstar':
        title = r'$q_{\rm cand}\cdot y_{{\rm Maximum}\, p^*}$'
        location = 9
    elif category == 'Kaon':
        location = 8
        title = r'$(q_{\rm cand}\cdot y_{\rm Kaon})_{\rm eff}$'
    elif category == 'Lambda':
        title = r'$(q_{\rm \Lambda}\cdot y_{\rm Lambda})_{\rm eff}$'

    if category == 'IntermediateKinLepton':
        location = 8
        title = r'$q_{\rm cand}\cdot y_{\rm Int.\, Kin.\, Lepton}$'
    elif category == 'KinLepton':
        title = r'$q_{\rm cand}\cdot y_{\rm Kin.\, Lepton}$'
    elif category == 'IntermediateMuon':
        title = r'$q_{\rm cand}\cdot y_{\rm Int.\, Muon}$'
    elif category == 'IntermediateElectron':
        title = r'$q_{\rm cand}\cdot y_{\rm Int.\, Electron}$'
    elif category == 'KaonPion':
        title = r'$q_{\rm cand}\cdot y_{\rm Kaon-Pion}$'
    elif category == 'FastHadron':
        location = 8
        title = r'$q_{\rm cand}\cdot y_{\rm Fast\, Hadron}$'
    elif category == 'SlowPion':
        title = r'$q_{\rm cand}\cdot y_{\rm Slow\, Pion}$'

    fig2 = plt.figure(2, figsize=(11, 8))
    ax2 = plt.axes([0.21, 0.16, 0.75, 0.81])  # plt.axes([0.14, 0.1, 0.8, 0.8])
    ax2.hist(data[np.where(data[:, 0] == -1)][:, 1], bins=bins, histtype='step',
             edgecolor='b', linewidth=4.5, linestyle='dashed', label=r'$\bar{B}^0$')  # hatch='.',
    ax2.hist(
        data[np.where(data[:, 0] == 1)][:, 1],
        bins=bins,
        histtype='step',
        edgecolor='r',
        linewidth=4,
        alpha=0.9,
        label=r'$B^0$')  # histtype='step', hatch='\\'

    p2, =  ax2.plot([], label=r'$\bar{B}^0$', linewidth=4.5, linestyle='dashed', c='b')
    p1, =  ax2.plot([], label=r'$B^0$', linewidth=4, linestyle='solid', alpha=0.9, c='r')

    ax2.set_ylabel(r'${\rm Number\ \ of\ \ Events\ /\ (\, 0.02\, )}$', fontsize=37)
    ax2.set_xlabel(title, fontsize=48)
    ax2.set_yscale('log', nonposy='clip')
    plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1],
               [r'$-1$', r'', r'$-0.5$', r'', r'$0$', r'', r'$0.5$', r'', r'$1$'], rotation=0, size=37)
    ax2.tick_params(axis='y', labelsize=37)
    ax2.legend([p2, p1], [r'$\bar{B}^0$', r'$B^0$'], prop={'size': 35}, loc=location)
    ax2.grid(True)
    plt.savefig(workingDirectory + '/' + 'pyPIC_' + category + '_Input_Combiner.pdf')
    fig2.clear()

    fig2b = plt.figure(2, figsize=(11, 8))
    ax2b = plt.axes([0.21, 0.16, 0.75, 0.81])   # plt.axes([0.14, 0.1, 0.8, 0.8])
    ax2b.hist(dataTruth[np.where(dataTruth[:, 0] == -1)][:, 1], bins=bins, histtype='step',
              edgecolor='b', linewidth=4.5, linestyle='dashed', label=r'$\bar{B}^0$')  # hatch='.',
    ax2b.hist(
        dataTruth[np.where(dataTruth[:, 0] == 1)][:, 1],
        bins=bins,
        histtype='step',
        edgecolor='r',
        linewidth=4,
        alpha=0.9,
        label=r'$B^0$')  # histtype='step', hatch='\\'

    p2, =  ax2b.plot([], label=r'$\bar{B}^0$', linewidth=4.5, linestyle='dashed', c='b')
    p1, =  ax2b.plot([], label=r'$B^0$', linewidth=4, linestyle='solid', alpha=0.9, c='r')

    ax2b.set_ylabel(r'${\rm Number\ \ of\ \ Events\ /\ (\, 0.02\, )}$', fontsize=37)
    ax2b.set_xlabel(title, fontsize=48)
    ax2b.set_yscale('log', nonposy='clip')
    plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1],
               [r'$-1$', r'', r'$-0.5$', r'', r'$0$', r'', r'$0.5$', r'', r'$1$'], rotation=0, size=37)
    ax2b.tick_params(axis='y', labelsize=37)
    ax2b.legend([p2, p1], [r'$\bar{B}^0$', r'$B^0$'], prop={'size': 35}, loc=location)
    ax2b.grid(True)
    plt.savefig(workingDirectory + '/' + 'pyPIC_' + category + '_Input_CombinerTruth.pdf')
    fig2b.clear()

    fig2c = plt.figure(2, figsize=(11, 8))
    ax2c = plt.axes([0.21, 0.16, 0.75, 0.81])   # plt.axes([0.14, 0.1, 0.8, 0.8])
    ax2c.hist(dataNoTarget[np.where(dataNoTarget[:, 0] == -1)][:, 1], bins=bins, histtype='step',
              edgecolor='b', linewidth=4.5, linestyle='dashed', label=r'$\bar{B}^0$')  # hatch='.',
    ax2c.hist(
        dataNoTarget[np.where(dataNoTarget[:, 0] == 1)][:, 1],
        bins=bins,
        histtype='step',
        edgecolor='r',
        linewidth=4,
        alpha=0.9,
        label=r'$B^0$')  # histtype='step', hatch='\\'

    p2, =  ax2c.plot([], label=r'$\bar{B}^0$', linewidth=4.5, linestyle='dashed', c='b')
    p1, =  ax2c.plot([], label=r'$B^0$', linewidth=4, linestyle='solid', alpha=0.9, c='r')

    ax2c.set_ylabel(r'${\rm Number\ \ of\ \ Events\ /\ (\, 0.02\, )}$', fontsize=37)
    ax2c.set_xlabel(title, fontsize=48)
    ax2c.set_yscale('log', nonposy='clip')
    plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1],
               [r'$-1$', r'', r'$-0.5$', r'', r'$0$', r'', r'$0.5$', r'', r'$1$'], rotation=0, size=37)
    ax2c.tick_params(axis='y', labelsize=37)
    ax2c.legend([p2, p1], [r'$\bar{B}^0$', r'$B^0$'], prop={'size': 35}, loc=location)
    ax2c.grid(True)
    plt.savefig(workingDirectory + '/' + 'pyPIC_' + category + '_Input_CombinerNoTarget.pdf')
    fig2c.clear()

    percentageCategory = dataTruth.shape[0] / data.shape[0] * 100
    percentageCategory = '% .2f' % percentageCategory

    print("Percentage of Category " + category + " is = " + percentageCategory + " %")

    # print("Data size = " + str(data.shape[0]))

NoTargetEfficiencies = np.array(NoTargetEfficiencies)
fig = plt.figure(5, figsize=(29, 18))
ax = plt.axes([0.15, 0.3, 0.81, 0.67])
# print(hi5)
width = 0.4
p1 = ax.bar(np.array(range(0, 13, 1)) + width / 4, NoTargetEfficiencies[:, 1], width, color='b', ecolor='k', label=r'$\bar{B}^0$')
p2 = ax.bar(np.array(range(0, 13, 1)) + width + width / 4, NoTargetEfficiencies[:, 0], width, color='r', ecolor='k', label=r'$B^0$')
ax.set_ylabel(r'$ \varepsilon_{\rm eff}/\  \% $', fontsize=30)
plt.xticks([0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5],
           categoryLabel, rotation=50, horizontalalignment='right', size=30)
plt.yticks([5, 10, 15, 20, 25, 50, 100], [r"$5$", r"", r"$15$", r"", r"$25$", r"$50$", r"$100$"], size=30)
ax.set_xlim(0, 13)
ax.set_ylim(0, 100)
ax.legend([r'$\bar{B}^0$', r'$B^0$'], prop={'size': 25})
ax.grid(True)
plt.savefig(workingDirectory + '/EfficienciesNoTarget.pdf')
fig.clear()

print('*                                                                                                                 *')
print('*******************************************************************************************************************')
B2INFO('qr Output Histograms in pdf format saved at: ' + workingDirectory)
