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
# This script plots the distributions                                    #
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
import sys
from matplotlib.ticker import FormatStrFormatter
import matplotlib.pyplot as plt
import ROOT
from ROOT import Belle2
import basf2 as b2
import flavorTagger as ft
from defaultEvaluationParameters import categories

import numpy as np
import matplotlib as mpl
mpl.use('Agg')
mpl.rcParams.update({'font.size': 22})
mpl.rcParams['text.usetex'] = True
mpl.rcParams['text.latex.preamble'] = [r"\usepackage{amsmath}"]

ROOT.gROOT.SetBatch(True)

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


ft.WhichCategories(categories)
ft.setVariables()

belleOrBelle2Flag = belleOrBelle2

identifiersExtraInfosDict = dict()
identifiersExtraInfosKaonPion = []

if belleOrBelle2 == "Belle":
    unitImp = "cm"

dBw = 50

pBins = 50
fBins = 100

unitImp = "mm"

variablesPlotParamsDict = {
    'useCMSFrame(p)': ['useCMSFrame__bop__bc', pBins, 0, 3, r'$p^*\ [{\rm GeV}/c]$', r"{\rm GeV}/c\, "],
    'useCMSFrame(pt)': ['useCMSFrame__bopt__bc', pBins, 0, 3, r'$p_{\rm t}^*\ [{\rm GeV}/c]$', r"{\rm GeV}/c\, "],
    'p': ['p', pBins, 0, 3, r'$p\ [{\rm GeV}/c]$', r"{\rm GeV}/c\, "],
    'pt': ['pt', pBins, 0, 3, r'$p_{\rm t}\ [{\rm GeV}/c]$', r"{\rm GeV}/c\, "],
    'cosTheta': ['cosTheta', dBw, -1, 1.01, r'$\cos{\theta}$', ""],
    ft.eId[ft.getBelleOrBelle2()]: [Belle2.makeROOTCompatible(ft.eId[ft.getBelleOrBelle2()]),
                                    dBw, 0, 1.01, r'$\mathcal{L}_{e}$', ""],
    'eid_dEdx': ['eid_dEdx', dBw, 0, 1.01, r'$\mathcal{L}_{e}^{{\rm d}E/{\rm d}x}$', ""],
    'eid_TOP': ['eid_TOP', dBw, 0, 1.01, r'$\mathcal{L}_{e}^{\rm TOP}$', ""],
    'eid_ARICH': ['eid_ARICH', dBw, 0, 1.01, r'$\mathcal{L}_{e}^{\rm ARICH}$', ""],
    'eid_ECL': ['eid_ECL', dBw, 0, 1.01, r'$\mathcal{L}_{e}^{\rm ECL}$', ""],
    'BtagToWBosonVariables(recoilMassSqrd)': ['BtagToWBosonVariables__borecoilMassSqrd__bc', fBins,
                                              0, 100, r'$M_{\rm rec}^2\ [{\rm GeV}^2/c^4]$', r"{\rm GeV}^2/c^4"],
    'BtagToWBosonVariables(pMissCMS)': [
        'BtagToWBosonVariables__bopMissCMS__bc', 60, 0, 3.6,
        r'$p^*_{\rm miss}\ [{\rm GeV}/c]$', r"{\rm GeV}/c\, "],
    'BtagToWBosonVariables(cosThetaMissCMS)': ['BtagToWBosonVariables__bocosThetaMissCMS__bc',
                                               dBw, -1, 1.01, r'$\cos{\theta^*_{\rm miss}}$', ""],
    'BtagToWBosonVariables(EW90)': ['BtagToWBosonVariables__boEW90__bc',
                                    dBw, 0, 4, r'$E_{90}^{W}\ [{\rm GeV}]$', r"{\rm GeV}\, "],
    'BtagToWBosonVariables(recoilMass)': ['sqrt(abs(BtagToWBosonVariables__borecoilMassSqrd__bc))',
                                          dBw, 0, 12, r'$M_{\rm rec}\ [{\rm GeV}/c^2]$', r"{\rm GeV}/c^2\, "],
    'cosTPTO': ['cosTPTO', dBw, 0, 1.01, r'$\vert\cos{\theta^*_{\rm T}}\vert$', ""],
    'ImpactXY': ['ImpactXY', dBw, 0, 0.5, r'$d_0\ [{\rm ' + unitImp + '}]$', r"{\rm " + unitImp + r"}\, "],
    'z0': ['z0', dBw, 0, 1.0, r'$z_0\ [{\rm ' + unitImp + '}]$', r"{\rm " + unitImp + r"}\, "],
    'y': ['y', dBw, -0.15, 0.15, r'$y_0\ [{\rm ' + unitImp + '}]$', r"{\rm " + unitImp + r"}\, "],
    'OBoost': ['OBoost', dBw, -0.15, 0.15, r'$d_0^\prime\ [{\rm ' + unitImp + '}]$', r"{\rm " + unitImp + r"}\, "],
    'distance': ['distance', dBw, 0, 1.5, r'$\xi_0\ [{\rm ' + unitImp + '}]$', r"{\rm " + unitImp + r"}\, "],
    'chiProb': ['chiProb', dBw, 0, 1.01, r'$p$-${\rm value}$', ""],
    ft.muId[ft.getBelleOrBelle2()]: [Belle2.makeROOTCompatible(ft.muId[ft.getBelleOrBelle2()]),
                                     dBw, 0, 1.01, r'$\mathcal{L}_{\mu}$', ""],
    'muid_dEdx': ['muid_dEdx', dBw, 0, 1.01, r'$\mathcal{L}_{\mu}^{{\rm d}E/{\rm d}x}$', ""],
    'muid_TOP': ['muid_TOP', dBw, 0, 1.01, r'$\mathcal{L}_{\mu}^{\rm TOP}$', ""],
    'muid_ARICH': ['muid_ARICH', dBw, 0, 1.01, r'$\mathcal{L}_{\mu}^{\rm ARICH}$', ""],
    'muid_KLM': ['muid_KLM', dBw, 0, 1.01, r'$\mathcal{L}_{\mu}^{\rm KLM}$', ""],
    ft.KId[ft.getBelleOrBelle2()]: [Belle2.makeROOTCompatible(ft.KId[ft.getBelleOrBelle2()]),
                                    dBw, 0, 1.01, r'$\mathcal{L}_{K}$', ""],
    'Kid_dEdx': ['Kid_dEdx', dBw, 0, 1.01, r'$\mathcal{L}_{K}^{{\rm d}E/{\rm d}x}$', ""],
    'Kid_TOP': ['Kid_TOP', dBw, 0, 1.01, r'$\mathcal{L}_{K}^{\rm TOP}$', ""],
    'Kid_ARICH': ['Kid_ARICH', dBw, 0, 1.01, r'$\mathcal{L}_{K}^{\rm ARICH}$', ""],
    'NumberOfKShortsInRoe': ['NumberOfKShortsInRoe', dBw, 0, 12, r'$n_{K^0_S}$', ""],
    'ptTracksRoe': ['ptTracksRoe', fBins, 0, 6, r'$\Sigma\, p_{\rm t}^2\ [{\rm GeV^2}/c^2]$',
                    r"{\rm GeV^2}/c^2"],
    'extraInfo(isRightCategory(Kaon))': ['extraInfo__boisRightCategory__boKaon__bc__bc',
                                         dBw, 0, 1.01, r"$y_{\rm Kaon}$", ""],
    'HighestProbInCat(pi+:inRoe, isRightCategory(SlowPion))': [
        'HighestProbInCat__bopi__pl__clinRoe__cm__spisRightCategory__boSlowPion__bc__bc',
        dBw, 0, 1.01, r"$y_{\rm SlowPion}$", ""],
    'KaonPionVariables(cosKaonPion)': ['KaonPionVariables__bocosKaonPion__bc',
                                       dBw, -1, 1.01, r'$\cos{\theta^*_{K\pi}}$', ""],
    'KaonPionVariables(HaveOpositeCharges)': ['KaonPionVariables__boHaveOpositeCharges__bc',
                                              dBw, 0, 1.01, r'$\frac{1 - q_{K} \cdot q_\pi}{2}$', ""],
    'pionID': ['pionID', dBw, 0, 1.01, r'$\mathcal{L}_{\pi}$', ""],
    'piid_dEdx': ['piid_dEdx', dBw, 0, 1.01, r'$\mathcal{L}_{\pi}^{{\rm d}E/{\rm d}x}$', ""],
    'piid_TOP': ['piid_TOP', dBw, 0, 1.01, r'$\mathcal{L}_{\pi}^{\rm TOP}$', ""],
    'piid_ARICH': ['piid_ARICH', dBw, 0, 1.01, r'$\mathcal{L}_{\pi}^{\rm ARICH}$', ""],
    'pi_vs_edEdxid': ['pi_vs_edEdxid', dBw, 0, 1.01, r'$\mathcal{L}_{\pi/e}^{{\rm d}E/{\rm d}x}$', ""],
    'FSCVariables(pFastCMS)': ['FSCVariables__bopFastCMS__bc', pBins, 0, 3, r'$p^*_{\rm Fast}\ [{\rm GeV}/c]$', r"{\rm GeV}/c\, "],
    'FSCVariables(cosSlowFast)': ['FSCVariables__bocosSlowFast__bc', dBw, -1, 1.01, r'$\cos{\theta^*_{\rm SlowFast}}$', ''],
    'FSCVariables(cosTPTOFast)': ['FSCVariables__bocosTPTOFast__bc', dBw, 0, 1.01, r'$\vert\cos{\theta^*_{\rm T, Fast}}\vert$', ''],
    'FSCVariables(SlowFastHaveOpositeCharges)': ['FSCVariables__boSlowFastHaveOpositeCharges__bc',
                                                 dBw, 0, 1.01, r'$\frac{1 - q_{\rm Slow} \cdot q_{\rm Fast}}{2}$', ""],
    'lambdaFlavor': ['lambdaFlavor', dBw, -1, 1.01, r'$q_{\Lambda}$', ""],
    'M': ['M', dBw, 1.08, 1.22, r'$M_{\Lambda}\ [{\rm GeV}/c^2]$', r"{\rm MeV}/c^2\, "],
    'cosAngleBetweenMomentumAndVertexVector': ['cosAngleBetweenMomentumAndVertexVector',
                                               dBw, -1, 1.01,
                                               r'$\cos{\theta_{\boldsymbol{x}_{\Lambda},\boldsymbol{p}_{\Lambda}}}$', ""],
    'lambdaZError': ['lambdaZError', dBw, 0, 0.05, r'$\sigma_{\Lambda}^{zz}$', r"{\rm mm}\, "],
    'daughter(0,p)': ['daughter__bo0__cmp__bc', dBw, 0, 1, r'$p_{\pi}\ [{\rm GeV}/c]$', r"{\rm GeV}/c\, "],
    'daughter(0,useCMSFrame(p))': ['daughter__bo0__cmuseCMSFrame__bop__bc__bc',
                                   dBw, 0, 1, r'$p^*_{\pi}\ [{\rm GeV}/c]$', r"{\rm GeV}/c\, "],
    'daughter(1,p)': ['daughter__bo1__cmp__bc', dBw, 0, 2, r'$p_{p}\ [{\rm GeV}/c]$', r"{\rm GeV}/c"],
    'daughter(1,useCMSFrame(p))': ['daughter__bo1__cmuseCMSFrame__bop__bc__bc',
                                   dBw, 0, 2, r'$p^*_{p}\ [{\rm GeV}/c]$', r"{\rm GeV}/c\, "],
    'daughter(1,protonID)': ['daughter__bo1__cmprotonID__bc', dBw, 0, 1.01, r'$\mathcal{L}_{p}$', ""],
    'daughter(0,pionID)': ['daughter__bo0__cmpionID__bc', dBw, 0, 1.01, r'$\mathcal{L}_{\pi}$', ""]}

if not b2.find_file('InputVariablesPlots', silent=True):
    os.mkdir('./InputVariablesPlots')


def plotInputVariablesOfFlavorTagger():
    """
    Makes plots of the distribution of the input variables of the flavor tagger
    for each category distinguishing between the target particles of the category (signal)
    and all the other (bkg.)
    """

    for (particleList, category, combinerVariable) in ft.eventLevelParticleLists:

        # if category != "SlowPion":
        #     continue

        if not b2.find_file('InputVariablesPlots/' + category, silent=True):
            os.mkdir('./InputVariablesPlots/' + category)

        if particleList not in identifiersExtraInfosDict and category != 'KaonPion':
            identifiersExtraInfosDict[particleList] = []

        methodPrefixEventLevel = "FlavorTagger_" + belleOrBelle2Flag + "_" + weightFiles + 'EventLevel' + category + 'FBDT'
        treeName = methodPrefixEventLevel + "_tree"
        targetVariable = 'isRightCategory(' + category + ')'

        tree = ROOT.TChain(treeName)

        workingFiles = glob.glob(filesDirectory + '/' + methodPrefixEventLevel + 'sampled*.root')
        # print("workingFiles = ", workingFiles)
        # workingFiles = glob.glob(filesDirectory + '/' + methodPrefixEventLevel + 'sampled1?.root')

        for iFile in workingFiles:
            tree.AddFile(iFile)

        categoryInputVariables = []
        trulyUsedInputVariables = []
        for iVariable in tree.GetListOfBranches():

            managerVariableName = str(Belle2.invertMakeROOTCompatible(iVariable.GetName()))

            if managerVariableName in ft.variables[category] or managerVariableName == 'distance' or \
               managerVariableName == 'z0' or managerVariableName == 'ImpactXY' or \
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

                    # if managerVariableName not in variablesPlotParamsDict:
                    #     variablesPlotParamsDict[managerVariableName] =
                    #                     [iVariable.GetName(), 100, 0, 2, iVariable.GetName(), "unit"]

        print("The number of variables used in " + category + " is = ", len(trulyUsedInputVariables))

        if category != 'KaonPion' and category != 'FSC' and category != 'MaximumPstar' and \
           category != 'FastHadron' and category != 'Lambda':
            categoryInputVariables.append('BtagToWBosonVariables(recoilMass)')

        for inputVariable in categoryInputVariables:

            print(inputVariable)

            nBins = variablesPlotParamsDict[inputVariable][1]
            limXmin = variablesPlotParamsDict[inputVariable][2]
            limXmax = variablesPlotParamsDict[inputVariable][3]

            if category == "SlowPion":
                if inputVariable == 'p' or inputVariable == 'useCMSFrame(p)' or \
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

            signalHistogram = ROOT.TH1F("signal" + category + str(Belle2.makeROOTCompatible(inputVariable)), "",
                                        nBins,
                                        limXmin,
                                        limXmax)
            backgroundHistogram = ROOT.TH1F("bkg" + category + str(Belle2.makeROOTCompatible(inputVariable)), "",
                                            nBins,
                                            limXmin,
                                            limXmax)

            factorMultiplication = str()

            if belleOrBelle2 == "Belle2" and ((category != "Lambda" and inputVariable == 'distance') or inputVariable ==
                                              'z0' or inputVariable == 'ImpactXY' or inputVariable ==
                                              'y' or inputVariable == 'OBoost'):
                factorMultiplication = "*10 "

            tree.Draw(variablesPlotParamsDict[inputVariable][0] + factorMultiplication + ">> signal" + category +
                      str(Belle2.makeROOTCompatible(inputVariable)), Belle2.makeROOTCompatible(targetVariable) + " > 0")

            tree.Draw(variablesPlotParamsDict[inputVariable][0] + factorMultiplication + ">> bkg" + category +
                      str(Belle2.makeROOTCompatible(inputVariable)), Belle2.makeROOTCompatible(targetVariable) + " < 1")

            signalScalingFactor = signalHistogram.Integral()
            backgroundScalingFactor = backgroundHistogram.Integral()

            if signalScalingFactor == 0:
                signalScalingFactor = 1

            if backgroundScalingFactor == 0:
                backgroundScalingFactor = 1

            signalHistogram.Scale(1 / signalScalingFactor)
            backgroundHistogram.Scale(1 / backgroundScalingFactor)

            signalArray = np.zeros((signalHistogram.GetNbinsX(), 2))
            backgroundArray = np.zeros((backgroundHistogram.GetNbinsX(), 2))

            for i in range(0, signalHistogram.GetNbinsX()):
                signalArray[i] = np.array([signalHistogram.GetBinCenter(i + 1), signalHistogram.GetBinContent(i + 1)])
                backgroundArray[i] = np.array([backgroundHistogram.GetBinCenter(i + 1), backgroundHistogram.GetBinContent(i + 1)])

            fig1 = plt.figure(1, figsize=(11, 10))

            # if inputVariable == 'Kid_dEdx' or inputVariable == 'muid_dEdx':
            #     ax1 = plt.axes([0.18, 0.17, 0.75, 0.8])
            # if inputVariable == 'pi_vs_edEdxid':
            #     ax1 = plt.axes([0.18, 0.187, 0.75, 0.805])
            # else:
            ax1 = plt.axes([0.18, 0.2, 0.76, 0.705])

            # print(signalArray.shape, signalHistogram.GetNbinsX(), )
            # print(signalArray)
            ax1.hist(
                signalArray[:, 0], weights=signalArray[:, 1], bins=signalHistogram.GetNbinsX(),
                histtype='step',
                edgecolor='r',
                linewidth=4,
                alpha=0.9,
                label=r'${\rm Signal}$')

            ax1.hist(backgroundArray[:, 0], weights=backgroundArray[:, 1], bins=backgroundHistogram.GetNbinsX(),
                     histtype='step',
                     edgecolor='b', linewidth=4.5, linestyle='dashed', label=r'${\rm Background}$')  # hatch='.',

            p1, =  ax1.plot([], label=r'${\rm Signal}$', linewidth=5, linestyle='solid', alpha=0.9, c='r')
            p2, =  ax1.plot([], label=r'${\rm Background}$', linewidth=5.5, linestyle='dashed', c='b')

            binWidth = signalHistogram.GetBinWidth(2)

            if inputVariable == 'lambdaZError':  # or inputVariable == 'ImpactXY' or\
                # (category != "Lambda" and inputVariable == 'distance'):
                binWidth = binWidth * 10

            if inputVariable == 'M':
                binWidth = binWidth * 1000

            if category == "Lambda" and inputVariable == 'distance':
                variablesPlotParamsDict[inputVariable][5] = r"{\rm cm}\, "

            binWidth = '{:8.2f}'.format(binWidth)

            xLabel = variablesPlotParamsDict[inputVariable][4]
            legendLocation = 1

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

            ax1.set_ylabel(r'${\rm Fraction\hspace{0.25em} of\hspace{0.25em} Events}\, /\, (\, ' + binWidth + r'\, ' +
                           variablesPlotParamsDict[inputVariable][5] + r')$', fontsize=46)
            ax1.set_xlabel(xLabel, fontsize=65)
            # plt.xticks([-1, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1],
            #           [r'$-1$', r'', r'$-0.5$', r'', r'$0$', r'', r'$0.5$', r'', r'$1$'], rotation=0, size=40)
            if inputVariable == 'extraInfo(isRightCategory(Kaon))' or \
               inputVariable == 'HighestProbInCat(pi+:inRoe, isRightCategory(SlowPion))':
                legendLocation = 3
                ax1.set_yscale('log', nonposy='clip')
            else:
                ax1.yaxis.set_major_formatter(FormatStrFormatter(r'$%.2f$'))

            ax1.tick_params(axis='x', labelsize=50)
            ax1.tick_params(axis='y', labelsize=40)

            if inputVariable == 'pi_vs_edEdxid':
                ax1.xaxis.labelpad = 5
            else:
                ax1.xaxis.labelpad = 15

            if inputVariable.find('ARICH') != -1 or inputVariable.find('TOP') != -1 or \
                    inputVariable == 'cosTPTO' or inputVariable.find('KLM') != -1 or \
                    inputVariable == 'cosTheta' or inputVariable == 'FSCVariables(cosTPTOFast)' or \
                    inputVariable == 'KaonPionVariables(cosKaonPion)' or \
                    inputVariable == 'BtagToWBosonVariables(recoilMass)':
                legendLocation = 2

            elif inputVariable == 'FSCVariables(SlowFastHaveOpositeCharges)' or \
                    inputVariable == 'KaonPionVariables(HaveOpositeCharges)' or inputVariable == "eid_ECL" or \
                    inputVariable.find('ID') != -1 or inputVariable.find('dEdx') != -1:
                legendLocation = 9

            if inputVariable == 'muid_dEdx':
                if category != 'KinLepton':
                    legendLocation = 8

            ax1.legend([p1, p2], [r'${\rm Signal}$', r'${\rm Bkgr.}$'], prop={
                       'size': 50}, loc=legendLocation, numpoints=1, handlelength=1)
            ax1.grid(True)
            # ax1.set_ylim(0, 1.4)
            ax1.set_xlim(limXmin, limXmax)
            plt.savefig('./InputVariablesPlots/' + category + '/' + category +
                        "_" + str(Belle2.makeROOTCompatible(inputVariable)) + '.pdf')
            fig1.clear()

            signalHistogram.Delete()
            backgroundHistogram.Delete()


if __name__ == '__main__':

    plotInputVariablesOfFlavorTagger()

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
