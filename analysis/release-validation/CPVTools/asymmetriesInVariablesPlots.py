#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#        For flavor tagging validation                                   #
#                                                                        #
# This script plots the asymmetries between positively                   #
# and negatively charged particles in the distributions                  #
# of all input variables used in the flavor tagger                       #
# module. As argument one needs to specify if the                        #
# Monte Carlo is Belle or Belle2 MC, if it was generated                 #
# with background or not (BGx0 or BGx1), and the                         #
# working directory of the flavor tagger                                 #
# (where the training samples are).                                      #
#                                                                        #
##########################################################################

import os
import glob
import math
from matplotlib.ticker import FormatStrFormatter
import matplotlib.pyplot as plt
import ROOT
from ROOT import Belle2
import sys
import basf2 as b2
import flavorTagger as ft
from inputVariablesPlots import variablesPlotParamsDict

import numpy as np
import matplotlib as mpl
mpl.use('Agg')
mpl.rcParams.update({'font.size': 22})
mpl.rcParams['text.usetex'] = True
mpl.rcParams['text.latex.preamble'] = [r"\usepackage{amsmath}"]


if len(sys.argv) != 5:
    sys.exit("Must provide 4 arguments: [Belle or Belle2] [BGx0 or BGx1] [training] [workingDirectory]"
             )

belleOrBelle2 = sys.argv[1]
MCtype = str(sys.argv[2])
decayChannelTrainedOn = str(sys.argv[3])  # Decay channel of the weight files "JPsiKs" or "nunubar"
workingDirectory = sys.argv[4]

filesDirectory = workingDirectory + '/FlavorTagging/TrainedMethods'

if decayChannelTrainedOn == 'JPsiKs':
    decayChannelTrainedOn = 'JpsiKs_mu'

weightFiles = 'B2' + decayChannelTrainedOn + MCtype

ROOT.TH1.SetDefaultSumw2()

allInputVariables = []

belleOrBelle2Flag = belleOrBelle2

identifiersExtraInfosDict = dict()
identifiersExtraInfosKaonPion = []

dBw = 50

pBins = 50
fBins = 100

unitImp = "mm"

if belleOrBelle2 == "Belle":
    unitImp = "cm"


# particle Label, PDG for negative, PDG for positive particle

particleConditions = {'e+:inRoe': ['e', " mcPDG > 0 ", " mcPDG < 0 "], 'mu+:inRoe': [r'\mu', " mcPDG > 0 ", " mcPDG < 0 "],
                      'lepton+:inRoe': ['l', " mcPDG > 0 ", " mcPDG < 0 "],
                      'K+:inRoe': ['K', " mcPDG < 0 ", " mcPDG > 0 "], 'pi+:inRoe': [r'\pi', " mcPDG < 0 ", " mcPDG > 0 "],
                      'h+:inRoe': ['h', " mcPDG < 0 ", " mcPDG > 0 "], 'Lambda0:inRoe': [r'\Lambda', " mcPDG < 0 ", " mcPDG > 0 "]}


if not b2.find_file('AsymmetriesInVariablesPlots', silent=True):
    os.mkdir('./AsymmetriesInVariablesPlots')

for (particleList, category, combinerVariable) in ft.eventLevelParticleLists:

    # if category != "KinLepton":
    #    continue

    if not b2.find_file('AsymmetriesInVariablesPlots/' + category, silent=True):
        os.mkdir('./AsymmetriesInVariablesPlots/' + category)

    if particleList not in identifiersExtraInfosDict and category != 'KaonPion':
        identifiersExtraInfosDict[particleList] = []

    methodPrefixEventLevel = "FlavorTagger_" + belleOrBelle2Flag + "_" + weightFiles + 'EventLevel' + category + 'FBDT'
    treeName = methodPrefixEventLevel + "_tree"
    targetVariable = 'isRightCategory(' + category + ')'

    tree = ROOT.TChain(treeName)

    workingFiles = glob.glob(filesDirectory + '/' + methodPrefixEventLevel + 'sampled*.root')
    # workingFiles = glob.glob(filesDirectory + '/' + methodPrefixEventLevel + 'sampled1?.root')

    for iFile in workingFiles:
        tree.AddFile(iFile)

    categoryInputVariables = []
    trulyUsedInputVariables = []
    for iVariable in tree.GetListOfBranches():

        managerVariableName = str(Belle2.invertMakeROOTCompatible(iVariable.GetName()))

        if managerVariableName in ft.variables[category] or managerVariableName == 'distance' or\
                managerVariableName == 'z0' or managerVariableName == 'ImpactXY' or\
                managerVariableName == 'y' or managerVariableName == 'OBoost':
            if managerVariableName in categoryInputVariables:
                continue

            categoryInputVariables.append(managerVariableName)
            if managerVariableName in ft.variables[category]:
                allInputVariables.append((category, managerVariableName))
                trulyUsedInputVariables.append((category, managerVariableName))

                if managerVariableName not in identifiersExtraInfosDict[particleList] and category != 'KaonPion':
                    identifiersExtraInfosDict[particleList].append(managerVariableName)

                elif category == 'KaonPion' and managerVariableName not in identifiersExtraInfosKaonPion:
                    identifiersExtraInfosKaonPion.append(managerVariableName)

    print("The number of variables used in " + category + " is = ", len(trulyUsedInputVariables))

    if category != 'KaonPion' and category != 'FSC' and category != 'MaximumPstar' and\
       category != 'FastHadron' and category != 'Lambda':
        categoryInputVariables.append('BtagToWBosonVariables(recoilMass)')

    for inputVariable in categoryInputVariables:

        # if not (inputVariable == 'z0' or inputVariable == 'ImpactXY' or inputVariable == 'y' or inputVariable == 'OBoost'):
        #   continue

        print(inputVariable)

        nBins = variablesPlotParamsDict[inputVariable][1]
        limXmin = variablesPlotParamsDict[inputVariable][2]
        limXmax = variablesPlotParamsDict[inputVariable][3]

        if belleOrBelle2 == "Belle2":
            if inputVariable == 'distance':
                limXmax = 1.0

            if inputVariable == 'ImpactXY':
                limXmax = 0.3

        if category == "SlowPion":
            if inputVariable == 'p' or inputVariable == 'useCMSFrame(p)' or\
                    inputVariable == 'pt' or inputVariable == 'useCMSFrame(pt)':
                nBins = 150
                limXmax = 1.5

            if inputVariable == 'distance':
                nBins = 80
                limXmax = 2.4

            if inputVariable == 'ImpactXY':
                nBins = 80
                limXmax = 0.8

        if category == "Lambda":
            if inputVariable == 'distance':
                # nBins = 25
                limXmax = 10

            if inputVariable == 'chiProb':
                nBins = 25

        additionalCond = str()

        if category == "FastHadron":
            particleList = 'h+:inRoe'

        if category == "KinLepton" or category == "IntermediateKinLepton":
            particleList = 'lepton+:inRoe'

        negativeHistogram = ROOT.TH1F("negative" + category + str(Belle2.makeROOTCompatible(inputVariable)), "",
                                      nBins,
                                      limXmin,
                                      limXmax)
        positiveHistogram = ROOT.TH1F("positive" + category + str(Belle2.makeROOTCompatible(inputVariable)), "",
                                      nBins,
                                      limXmin,
                                      limXmax)

        factorMultiplication = str()

        if belleOrBelle2 == "Belle2" and ((category != "Lambda" and inputVariable == 'distance') or inputVariable ==
                                          'z0' or inputVariable == 'ImpactXY' or inputVariable == 'y' or inputVariable == 'OBoost'):
            factorMultiplication = "*10 "

        tree.Draw(variablesPlotParamsDict[inputVariable][0] +
                  factorMultiplication +
                  ">> negative" +
                  category +
                  str(Belle2.makeROOTCompatible(inputVariable)), additionalCond +
                  "abs(" +
                  Belle2.makeROOTCompatible(targetVariable) +
                  ") > 0 && " +
                  particleConditions[particleList][1])

        tree.Draw(variablesPlotParamsDict[inputVariable][0] +
                  factorMultiplication +
                  ">> positive" +
                  category +
                  str(Belle2.makeROOTCompatible(inputVariable)), additionalCond +
                  "abs(" +
                  Belle2.makeROOTCompatible(targetVariable) +
                  ") > 0 && " +
                  particleConditions[particleList][2])

        negativeScalingFactor = negativeHistogram.Integral()
        positiveScalingFactor = positiveHistogram.Integral()

        if negativeScalingFactor == 0:
            negativeScalingFactor = 1

        if positiveScalingFactor == 0:
            positiveScalingFactor = 1

        negativeHistogram.Scale(1 / negativeScalingFactor)
        positiveHistogram.Scale(1 / positiveScalingFactor)

        negativeArray = np.zeros((negativeHistogram.GetNbinsX(), 2))
        positiveArray = np.zeros((positiveHistogram.GetNbinsX(), 2))

        asymmetryArray = np.zeros((positiveHistogram.GetNbinsX(), 2))
        asymmetryArrayUncertainties = np.zeros((positiveHistogram.GetNbinsX(), 2))

        for i in range(0, negativeHistogram.GetNbinsX()):
            negativeArray[i] = np.array([negativeHistogram.GetBinCenter(i + 1), negativeHistogram.GetBinContent(i + 1)])
            positiveArray[i] = np.array([positiveHistogram.GetBinCenter(i + 1), positiveHistogram.GetBinContent(i + 1)])

            numerator = float(positiveArray[i][1] - negativeArray[i][1])
            denominator = float(positiveArray[i][1] + negativeArray[i][1])

            if denominator == 0:
                asymmetryArray[i] = np.array([negativeHistogram.GetBinCenter(i + 1), 0])
                asymmetryArrayUncertainties[i] = np.array([negativeHistogram.GetBinCenter(i + 1), 0])
            else:
                asymmetryArray[i] = np.array([negativeHistogram.GetBinCenter(i + 1), float(numerator / denominator)])

                uncertainty = 2 * math.sqrt((negativeArray[i][1] * negativeHistogram.GetBinError(i + 1))**2 + (
                    positiveArray[i][1] * positiveHistogram.GetBinError(i + 1))**2) / (negativeArray[i][1] + positiveArray[i][1])**2

                asymmetryArrayUncertainties[i] = np.array([negativeHistogram.GetBinCenter(i + 1), uncertainty])

        fig1 = plt.figure(1, figsize=(11, 11))

        # if inputVariable == 'Kid_dEdx' or inputVariable == 'muid_dEdx':
        #    ax1 = plt.axes([0.18, 0.17, 0.75, 0.8])
        # if inputVariable == 'pi_vs_edEdxid':
        #    ax1 = plt.axes([0.18, 0.187, 0.75, 0.805])
        # else:
        ax1 = plt.axes([0.19, 0.37, 0.76, 0.60])

        # print(negativeArray.shape, negativeHistogram.GetNbinsX(), )
        # print(negativeArray)
        ax1.hist(
            negativeArray[:, 0], weights=negativeArray[:, 1], bins=negativeHistogram.GetNbinsX(),
            histtype='step', edgecolor='b', linewidth=4.5, linestyle='dashed',
            label=r'$' + particleConditions[particleList][0] + '^{-} $')

        ax1.hist(positiveArray[:, 0], weights=positiveArray[:, 1], bins=positiveHistogram.GetNbinsX(),
                 histtype='step', edgecolor='r', linewidth=4, alpha=0.7,
                 label=r'$' + particleConditions[particleList][0] + '^{+} $')  # hatch='.',

        p1, =  ax1.plot([], label=r'$' + particleConditions[particleList][0] + '^{-} $',
                        linewidth=5.5, linestyle='dashed', c='b')
        p2, =  ax1.plot([], label=r'$' + particleConditions[particleList][0] +
                        '^{+} $', linewidth=5, linestyle='solid', alpha=0.9, c='r')

        binWidth = negativeHistogram.GetBinWidth(2)

        if inputVariable == 'lambdaZError':  # or inputVariable == 'ImpactXY' or\
            #                (category != "Lambda" and inputVariable == 'distance'):
            binWidth = binWidth * 10

        if inputVariable == 'M':
            binWidth = binWidth * 1000

        if category == "Lambda" and inputVariable == 'distance':
            variablesPlotParamsDict[inputVariable][5] = r"{\rm cm}\, "

        binWidth = '{:8.2f}'.format(binWidth)

        legendLocation = 1
        ax1.set_ylabel(r'${\rm Fraction\hspace{0.25em} of\hspace{0.25em} Events}\, /\, (\, ' + binWidth + r'\, ' +
                       variablesPlotParamsDict[inputVariable][5] + r')$', fontsize=35)
        if inputVariable == 'extraInfo(isRightCategory(Kaon))' or\
           inputVariable == 'HighestProbInCat(pi+:inRoe, isRightCategory(SlowPion))':
            legendLocation = 3
            ax1.set_yscale('log', nonposy='clip')
        else:
            ax1.yaxis.set_major_formatter(FormatStrFormatter(r'$%.2f$'))

        # ax1.set_ylim(0, 1.4)
        ax1.set_xlim(limXmin, limXmax)

        locs, labels = plt.xticks()
        empty_string_labels = [''] * len(labels)
        plt.locator_params(axis='x', nbins=len(labels))
        ax1.set_xticklabels(empty_string_labels)
        ax1.tick_params(axis='y', labelsize=37)

        if inputVariable.find('ARICH') != -1 or inputVariable.find('TOP') != -1 or \
                inputVariable == 'cosTPTO' or inputVariable.find('KLM') != -1 or\
                inputVariable == 'cosTheta' or inputVariable == 'FSCVariables(cosTPTOFast)' or\
                inputVariable == 'KaonPionVariables(cosKaonPion)':
            legendLocation = 2

        elif inputVariable == 'FSCVariables(SlowFastHaveOpositeCharges)' or \
                inputVariable == 'KaonPionVariables(HaveOpositeCharges)' or inputVariable == "eid_ECL" or \
                inputVariable.find('ID') != -1 or inputVariable.find('dEdx') != -1:
            legendLocation = 9

        if inputVariable == 'muid_dEdx':
            if category != 'KinLepton':
                legendLocation = 8

        ax1.legend([p1, p2], [r'$' +
                              particleConditions[particleList][0] +
                              '^{-} $', r'$' +
                              particleConditions[particleList][0] +
                              '^{+} $'], prop={'size': 50}, loc=legendLocation, numpoints=1, handlelength=1)
        ax1.grid(True)

        ax2 = plt.axes([0.19, 0.15, 0.76, 0.2])

        # print(asymmetryArrayUncertainties)

        ax2.errorbar(asymmetryArray[:, 0], asymmetryArray[:, 1], xerr=float(negativeHistogram.GetBinWidth(2) / 2),
                     yerr=asymmetryArrayUncertainties[:, 1], elinewidth=2, mew=2, ecolor='k',
                     fmt='o', mfc='k', mec='k', markersize=6, label=r'${\rm Data}$')

        ax2.set_ylabel(r'$\frac{f^{+}\; -\;\, f^{-}}{f^{+}\; +\;\, f^{-}}$', fontsize=50, labelpad=20)
        ax2.yaxis.labelpad = 8

        plt.yticks([-0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75],
                   [r'', r'$-0.5$', r'', r'$0.0$', r'', r'$0.5$', r''], rotation=0, size=25)
        ax2.set_ylim(-0.75, 0.75)
        ax2.xaxis.grid(True)  # linestyle='--'
        plt.axhline(y=0.5, linewidth=2, color='tab:gray', linestyle='-')
        plt.axhline(y=0.25, linewidth=2, color='tab:gray', linestyle='-')
        plt.axhline(y=0, linewidth=2, color='tab:gray', linestyle='-')
        plt.axhline(y=-0.25, linewidth=2, color='tab:gray', linestyle='-')
        plt.axhline(y=-0.5, linewidth=2, color='tab:gray', linestyle='-')

        xLabel = variablesPlotParamsDict[inputVariable][4]
        if category == "FSC":
            if inputVariable == 'cosTPTO':
                xLabel = r'$\vert\cos{\theta^*_{\rm T, Slow}}\vert$'
            if inputVariable == 'useCMSFrame(p)':
                xLabel = r'$p^*_{\rm Slow}\ [{\rm GeV}/c]$'
        if category == 'Lambda':
            if inputVariable == 'useCMSFrame(p)':
                xLabel = r'$p^*_{\Lambda}\ [{\rm GeV}/c]$'
            if inputVariable == 'p':
                xLabel = r'$p_{\Lambda}\ [{\rm GeV}/c]$'

        if inputVariable == 'pi_vs_edEdxid':
            ax2.xaxis.labelpad = 5
        else:
            ax2.xaxis.labelpad = 15
        plt.locator_params(axis='x', nbins=len(labels))

        ax2.set_xlim(limXmin, limXmax)
        ax2.tick_params(axis='x', labelsize=40)
        ax2.set_xlabel(xLabel, fontsize=50)
        plt.savefig('./AsymmetriesInVariablesPlots/' + category + '/' + category +
                    "_" + str(Belle2.makeROOTCompatible(inputVariable)) + '.pdf')
        fig1.clear()

        negativeHistogram.Delete()
        positiveHistogram.Delete()

totalNumberOfVariables = 0

for category in ft.variables:
    totalNumberOfVariables += len(ft.variables[category])

print("Total number of variables = ", totalNumberOfVariables)

totalNumberOfCalculatedVariables = len(identifiersExtraInfosKaonPion)

print("Calculations for Kaon-Pion Category = ", totalNumberOfCalculatedVariables)

print("Variables per particle list:")
for particleList in identifiersExtraInfosDict:
    print(particleList)
    print(identifiersExtraInfosDict[particleList])
    totalNumberOfCalculatedVariables += len(identifiersExtraInfosDict[particleList])

print("Total number of calculated variables = ", totalNumberOfCalculatedVariables)
