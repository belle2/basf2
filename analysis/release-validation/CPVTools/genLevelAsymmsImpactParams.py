#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This script checks if there are asymmetries in the
# reconstructed impact parameters and in the generated vertex
# positions of electrons, muons, kaons, pions and protons.
# As input one needs the path to one or several mdst files.
# As output the script saves the plots comparing
# positively and negatively charged particles.
#
# Contributor: F. Abudinen (December 2018)
#
######################################################


import basf2 as b2
import os
import glob
import sys
import math
from matplotlib.ticker import FormatStrFormatter
import matplotlib.pyplot as plt
import ROOT
from ROOT import Belle2

import numpy as np
import matplotlib as mpl
mpl.use('Agg')
mpl.rcParams.update({'font.size': 22})
mpl.rcParams['text.usetex'] = True
mpl.rcParams['text.latex.preamble'] = [r"\usepackage{amsmath}"]

if len(sys.argv) != 3:
    sys.exit("Must provide 2 arguments: [Belle or Belle2] [samplesWildCards]"
             )

belleOrBelle2 = sys.argv[1]
samplesWildCards = sys.argv[2]


workingDirectory = '.'

if not b2.find_file('GenLevelVariablesPlots', silent=True):
    os.mkdir(workingDirectory + '/GenLevelVariablesPlots')

sourceFiles = glob.glob(str(samplesWildCards))

tree = ROOT.TChain("tree")
for Inputfile in sourceFiles:
    tree.AddFile(Inputfile)


ROOT.TH1.SetDefaultSumw2()

binsPt = 50

HistoBelle2_PtMC = ROOT.TH1F('PtMCBelle2', 'MC Transverse Momentum', binsPt, 0, 5)
HistoBelle2_PtFit = ROOT.TH1F('PtFitBelle2', 'TrackFitResult Transverse Momentum', binsPt, 0, 5)

HistoBelle2_pseudoPD0MC = ROOT.TH1F('pseudoPD0MCBelle2', 'MC pseudo Momentum for D0', binsPt, 0, 5)
HistoBelle2_pseudoPD0Fit = ROOT.TH1F('pseudoPD0FitBelle2', 'Fit pseudo Momentum for D0', binsPt, 0, 5)

HistoBelle2_pseudoPZ0MC = ROOT.TH1F('pseudoPZ0MCBelle2', 'MC pseudo Momentum for Z0', binsPt, 0, 5)
HistoBelle2_pseudoPZ0Fit = ROOT.TH1F('pseudoPZ0FitBelle2', 'Fit pseudo Momentum for Z0', binsPt, 0, 5)

HistoBelle2_d0Error = ROOT.TH1F('d0ErrorBelle2', 'TrackFitResult Error on d0', binsPt, 0, 0.03)
HistoBelle2_z0Error = ROOT.TH1F('z0ErrorBelle2', 'TrackFitResult Error on z0', binsPt, 0, 0.03)

D0 = 'abs(TrackFitResults.m_tau[0][0])'
Z0 = 'abs(TrackFitResults.m_tau[0][3])'
X0 = 'sqrt(TrackFitResults.m_tau[0][0]**2 + TrackFitResults.m_tau[0][3]**2)'

mcX = "MCParticles.m_productionVertex_x"
mcY = "MCParticles.m_productionVertex_y"
mcZ = "MCParticles.m_productionVertex_z"

mcD0 = "sqrt(MCParticles.m_productionVertex_x*MCParticles.m_productionVertex_x + " + \
    " MCParticles.m_productionVertex_y*MCParticles.m_productionVertex_y)"
mcZ0 = "abs(MCParticles.m_productionVertex_z)"
mcX0 = "sqrt(MCParticles.m_productionVertex_x*MCParticles.m_productionVertex_x + " + \
    "MCParticles.m_productionVertex_y*MCParticles.m_productionVertex_y + " + \
    "MCParticles.m_productionVertex_z*MCParticles.m_productionVertex_z)"

ptMC = 'sqrt(MCParticles.m_momentum_x**2 + MCParticles.m_momentum_y**2)'
ptFit = 'abs(0.003*1.5/TrackFitResults.m_tau[0][2])'

pMC = 'sqrt(MCParticles.m_momentum_x**2 + MCParticles.m_momentum_y**2 + + MCParticles.m_momentum_z**2)'
pFit = '(abs(0.003*1.5/TrackFitResults.m_tau[0][2])*sqrt(1+TrackFitResults.m_tau[0][4]**2))'
chiProb = 'TrackFitResults.m_pValue'

pseudoPD0MC = 'sqrt(' + ptMC + '**3/(' + pMC + '*(1+(MCParticles.m_mass[0]/' + pMC + ')**2)))'
pseudoPD0Fit = 'sqrt(' + ptFit + '**3/(' + pFit + '*(1+(MCParticles.m_mass[0]/' + pFit + ')**2)))'

pseudoPZ0MC = 'sqrt(' + ptMC + '**5/(' + pMC + '**3*(1+(MCParticles.m_mass[0]/' + pMC + ')**2)))'
pseudoPZ0Fit = 'sqrt(' + ptFit + '**5/(' + pFit + '**3*(1+(MCParticles.m_mass[0]/' + pFit + ')**2)))'


dBw = 50
pBins = 50
fBins = 100

particleConditions = {
    'Electron': ['e', "11 ", " MCParticles.m_pdg > 0 ", " MCParticles.m_pdg < 0 "],
    'Muon': [r'\mu', "13 ", " MCParticles.m_pdg > 0 ", " MCParticles.m_pdg < 0 "],
    'Kaon': ['K', "321 ", " MCParticles.m_pdg < 0 ", " MCParticles.m_pdg > 0 "],
    'Pion': [r'\pi', "211 ", " MCParticles.m_pdg < 0 ", " MCParticles.m_pdg > 0 "],
    'Proton': ['p', "2212 ", " MCParticles.m_pdg < 0 ", " MCParticles.m_pdg > 0 "]}


Particles = ["Electron", "Muon", "Kaon", "Pion", "Proton"]

unitImp = "mm"


def makePlotsForEachParticleKind(cutOnUpsilonFourS=""):
    """
    Makes plots showing the distribution of the impact parameters and vertices
    for positively and negatively charged particles.
    """

    unitImp = "mm"

    variablesPlotParamsDict = {'ImpactXY': [D0, dBw, 0, 0.5, r'$d_0\ [{\rm ' + unitImp + '}]$', r"{\rm " + unitImp + r"}\, "],
                               'z0': [Z0, dBw, 0, 1.0, r'$z_0\ [{\rm ' + unitImp + '}]$', r"{\rm " + unitImp + r"}\, "],
                               'distance': [X0, dBw, 0, 1.5, r'$\xi_0\ [{\rm ' + unitImp + '}]$', r"{\rm " + unitImp + r"}\, "],
                               'mcGenX': [mcX, dBw, -0.05, 0.15, r'$x_{\rm MC}\ [{\rm ' + unitImp + '}]$',
                                          r"{\rm " + unitImp + r"}\, "],
                               'mcGenY': [mcY, dBw, -0.075, 0.075, r'$y_{\rm MC}\ [{\rm ' + unitImp + '}]$',
                                          r"{\rm " + unitImp + r"}\, "],
                               'mcGenZ': [mcZ, dBw, -0.6, 1, r'$z_{\rm MC}\ [{\rm ' + unitImp + '}]$',
                                          r"{\rm " + unitImp + r"}\, "],
                               'mcGenImpactXY': [mcD0, dBw, 0, 0.1,
                                                 r'$\sqrt{x_{\rm MC}^2 + y_{\rm MC}^2}\ [{\rm ' + unitImp + '}]$',
                                                 r"{\rm " + unitImp + r"}\, "],
                               'mcGenz0': [mcZ0, dBw, 0, 1, r'$|z_{\rm MC}|\ [{\rm ' + unitImp + '}]$',
                                           r"{\rm " + unitImp + r"}\, "],
                               'mcGendistance': [mcX0, dBw, 0, 1,
                                                 r'$\sqrt{x_{\rm MC}^2 + y_{\rm MC}^2 + z_{\rm MC}^2}\ [{\rm ' + unitImp + '}]$',
                                                 r"{\rm " + unitImp + r"}\, "],
                               }

    if belleOrBelle2 == "Belle":
        unitImp = "cm"

        variablesPlotParamsDict = {
            'ImpactXY': [
                D0,
                dBw,
                0,
                0.1,
                r'$d_0\ [{\rm ' + unitImp + '}]$',
                r"{\rm " + unitImp + r"}\, "],
            'z0': [
                Z0,
                dBw,
                0,
                1.0,
                r'$z_0\ [{\rm ' + unitImp + '}]$',
                r"{\rm " + unitImp + r"}\, "],
            'distance': [
                X0,
                dBw,
                0,
                1.5,
                r'$\xi_0\ [{\rm ' + unitImp + '}]$',
                r"{\rm " + unitImp + r"}\, "],
            'mcGenX': [
                mcX,
                dBw,
                0,
                0.1,
                r'$x_{\rm MC}\ [{\rm ' + unitImp + '}]$',
                r"{\rm " + unitImp + r"}\, "],
            'mcGenY': [
                mcY,
                dBw,
                -0.005,
                0.02,
                r'$y_{\rm MC}\ [{\rm ' + unitImp + '}]$',
                r"{\rm " + unitImp + r"}\, "],
            'mcGenZ': [
                mcZ,
                dBw,
                -0.6,
                1,
                r'$z_{\rm MC}\ [{\rm ' + unitImp + '}]$',
                r"{\rm " + unitImp + r"}\, "],
            'mcGenImpactXY': [
                mcD0,
                dBw,
                0,
                0.1,
                r'$\sqrt{x_{\rm MC}^2 + y_{\rm MC}^2}\ [{\rm ' + unitImp + '}]$',
                r"{\rm " + unitImp + r"}\, "],
            'mcGenz0': [
                mcZ0,
                dBw,
                0,
                1,
                r'$|z_{\rm MC}|\ [{\rm ' + unitImp + '}]$',
                r"{\rm " + unitImp + r"}\, "],
            'mcGendistance': [
                mcX0,
                dBw,
                0,
                1,
                r'$\sqrt{x_{\rm MC}^2 + y_{\rm MC}^2 + z_{\rm MC}^2}\ [{\rm ' + unitImp + '}]$',
                r"{\rm " + unitImp + r"}\, "],
        }

    withOrWithoutCut = ""
    if cutOnUpsilonFourS != "":
        withOrWithoutCut = "WithUpsilonFourSCut"
        unitImp = "mm"
        variablesPlotParamsDict = {'ImpactXY': [D0, dBw, 0, 1.0, r'$d_0\ [{\rm ' + unitImp + '}]$', r"{\rm " + unitImp + r"}\, "],
                                   'z0': [Z0, dBw, 0, 1.0, r'$z_0\ [{\rm ' + unitImp + '}]$', r"{\rm " + unitImp + r"}\, "],
                                   'distance': [X0, dBw, 0, 1.5, r'$\xi_0\ [{\rm ' + unitImp + '}]$', r"{\rm " + unitImp + r"}\, "],
                                   'mcGenX': [mcX, dBw, 0, 1, r'$x_{\rm MC}\ [{\rm ' + unitImp + '}]$',
                                              r"{\rm " + unitImp + r"}\, "],
                                   'mcGenY': [mcY, dBw, -1, 1, r'$y_{\rm MC}\ [{\rm ' + unitImp + '}]$',
                                              r"{\rm " + unitImp + r"}\, "],
                                   'mcGenZ': [mcZ, dBw, -0.5, 1, r'$z_{\rm MC}\ [{\rm ' + unitImp + '}]$',
                                              r"{\rm " + unitImp + r"}\, "],
                                   'mcGenImpactXY': [mcD0, dBw, 0, 1.0,
                                                     r'$\sqrt{x_{\rm MC}^2 + y_{\rm MC}^2}\ [{\rm ' + unitImp + '}]$',
                                                     r"{\rm " + unitImp + r"}\, "],
                                   'mcGenz0': [mcZ0, dBw, 0, 0.4,
                                               r'$|z_{\rm MC}|\ [{\rm ' + unitImp + '}]$', r"{\rm " + unitImp + r"}\, "],
                                   'mcGendistance': [mcX0, dBw, 0, 1.5,
                                                     r'$\sqrt{x_{\rm MC}^2 + y_{\rm MC}^2 + z_{\rm MC}^2}\ [{\rm ' +
                                                     unitImp + '}]$',
                                                     r"{\rm " + unitImp + r"}\, "],
                                   }

    for Particle in Particles:
        print(Particle)
        for inputVariable in variablesPlotParamsDict:

            print(inputVariable)

            nBins = variablesPlotParamsDict[inputVariable][1]
            limXmin = variablesPlotParamsDict[inputVariable][2]
            limXmax = variablesPlotParamsDict[inputVariable][3]

            negativeHistogram = ROOT.TH1F("negative" + Particle + str(Belle2.makeROOTCompatible(inputVariable)), "",
                                          nBins,
                                          limXmin,
                                          limXmax)
            positiveHistogram = ROOT.TH1F("positive" + Particle + str(Belle2.makeROOTCompatible(inputVariable)), "",
                                          nBins,
                                          limXmin,
                                          limXmax)
            condition = "MCParticles.m_status%2==1 && abs(MCParticles.m_pdg) == " + particleConditions[Particle][1] + " && "
            condition = condition + cutOnUpsilonFourS
            condition = condition + " abs(MCParticles.m_pdg[MCParticles.m_mother - 1])==511 && "

            factorMultiplication = str()

            if belleOrBelle2 != "Belle" or (belleOrBelle2 == "Belle" and cutOnUpsilonFourS != ""):
                factorMultiplication = "*10 "

            tree.Draw(variablesPlotParamsDict[inputVariable][0] +
                      factorMultiplication +
                      ">> negative" +
                      Particle +
                      str(Belle2.makeROOTCompatible(inputVariable)), condition +
                      particleConditions[Particle][2])

            tree.Draw(variablesPlotParamsDict[inputVariable][0] +
                      factorMultiplication +
                      ">> positive" +
                      Particle +
                      str(Belle2.makeROOTCompatible(inputVariable)), condition +
                      particleConditions[Particle][3])

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

                    uncertainty = 2 * math.sqrt((negativeArray[i][1] * negativeHistogram.GetBinError(i + 1))**2 +
                                                (positiveArray[i][1] * positiveHistogram.GetBinError(i + 1))**2) / \
                        (negativeArray[i][1] + positiveArray[i][1])**2

                    asymmetryArrayUncertainties[i] = np.array([negativeHistogram.GetBinCenter(i + 1), uncertainty])

            fig1 = plt.figure(1, figsize=(11, 11))

            ax1 = plt.axes([0.19, 0.37, 0.76, 0.60])

            ax1.hist(
                negativeArray[:, 0], weights=negativeArray[:, 1], bins=negativeHistogram.GetNbinsX(),
                histtype='step', edgecolor='b', linewidth=4.5, linestyle='dashed',

                label=r'$' + particleConditions[Particle][0] + '^{-} $')

            ax1.hist(positiveArray[:, 0], weights=positiveArray[:, 1], bins=positiveHistogram.GetNbinsX(),
                     histtype='step', edgecolor='r', linewidth=4, alpha=0.9,
                     label=r'$' + particleConditions[Particle][0] + '^{+} $')  # hatch='.',

            p1, =  ax1.plot([], label=r'$' + particleConditions[Particle][0] + '^{-} $', linewidth=5.5, linestyle='dashed', c='b')
            p2, =  ax1.plot([], label=r'$' + particleConditions[Particle][0] +
                            '^{+} $', linewidth=5, linestyle='solid', alpha=0.9, c='r')

            binWidth = negativeHistogram.GetBinWidth(2)

            binWidth = '{:8.3f}'.format(binWidth)

            legendLocation = 1
            ax1.set_ylabel(r'${\rm Fraction\hspace{0.25em} of\hspace{0.25em} Events}\, /\, (\, ' + binWidth + r'\, ' +
                           variablesPlotParamsDict[inputVariable][5] + r')$', fontsize=35)
            if inputVariable == 'p' or inputVariable == 'pt':
                legendLocation = 7

            ax1.yaxis.set_major_formatter(FormatStrFormatter(r'$%.2f$'))

            ax1.set_xlim(limXmin, limXmax)

            locs, labels = plt.xticks()

            empty_string_labels = [''] * len(labels)
            plt.locator_params(axis='x', nbins=len(labels))
            ax1.set_xticklabels(empty_string_labels)
            ax1.tick_params(axis='y', labelsize=37)

            ax1.legend([p1, p2], [r'$' +
                                  particleConditions[Particle][0] +
                                  '^{-} $', r'$' +
                                  particleConditions[Particle][0] +
                                  '^{+} $'], prop={'size': 50}, loc=legendLocation, numpoints=1, handlelength=1)
            ax1.grid(True)

            ax2 = plt.axes([0.19, 0.15, 0.76, 0.2])

            ax2.errorbar(asymmetryArray[:, 0], asymmetryArray[:, 1], xerr=float(negativeHistogram.GetBinWidth(2) / 2),
                         yerr=asymmetryArrayUncertainties[:, 1], elinewidth=2, mew=2, ecolor='k',
                         fmt='o', mfc='k', mec='k', markersize=6, label=r'${\rm Data}$')

            ax2.set_ylabel(r'$\frac{f^{+}\; -\;\, f^{-}}{f^{+}\; +\;\, f^{-}}$', fontsize=50, labelpad=20)
            ax2.yaxis.labelpad = 8

            plt.yticks([-0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75],
                       [r'', r'$-0.5$', r'', r'$0.0$', r'', r'$0.5$', r''], rotation=0, size=25)
            ax2.set_ylim(-0.75, 0.75)
            ax2.xaxis.grid(True)
            plt.axhline(y=0.5, linewidth=2, color='tab:gray', linestyle='-')
            plt.axhline(y=0.25, linewidth=2, color='tab:gray', linestyle='-')
            plt.axhline(y=0, linewidth=2, color='tab:gray', linestyle='-')
            plt.axhline(y=-0.25, linewidth=2, color='tab:gray', linestyle='-')
            plt.axhline(y=-0.5, linewidth=2, color='tab:gray', linestyle='-')

            xLabel = variablesPlotParamsDict[inputVariable][4]

            ax2.xaxis.labelpad = 15
            plt.locator_params(axis='x', nbins=len(labels))
            ax2.tick_params(axis='x', labelsize=40)

            ax2.set_xlim(limXmin, limXmax)

            if inputVariable == 'mcGenY' and belleOrBelle2 == "Belle2":
                plt.xticks([-0.06, -0.04, -0.02, 0, 0.02, 0.04, 0.06],
                           ['', r'$-0.04$', '', r'$0.00$', '', r'$0.04$', ''], rotation=0, size=40)

            ax2.set_xlabel(xLabel, fontsize=50)
            plt.savefig(workingDirectory + '/GenLevelVariablesPlots' + '/' + belleOrBelle2 + withOrWithoutCut +
                        "_" + Particle + "_" + str(Belle2.makeROOTCompatible(inputVariable)) + '.pdf')
            fig1.clear()

            negativeHistogram.Delete()
            positiveHistogram.Delete()


DecZ = "MCParticles.m_decayVertex_z"

if belleOrBelle2 == "Belle2":

    variablesPlotB0tag = {'DecZ': [DecZ, dBw, -0.6, 1.5, r"{\rm " + unitImp + r"}\, "],
                          }

else:
    unitImp = "cm"
    variablesPlotB0tag = {'DecZ': [DecZ, dBw, -1.5, 1.5, r"{\rm " + unitImp + r"}\, "],
                          }

particleCondsB0tag = {
    'B0tag': [
        'B0',
        "511 ",
        " MCParticles.m_pdg < 0 ",
        " MCParticles.m_pdg > 0 ",
        " !(MCParticles.m_pdg[MCParticles.m_firstDaughter-1] == 310 && MCParticles.m_pdg[MCParticles.m_firstDaughter] == 443) && ",
        r'$(z_{\rm tag}^{\rm dec})^{\rm gen}\ [{\rm ' + unitImp + '}]$'],
    'B0sig': [
        'B0',
        "511 ",
        " MCParticles.m_pdg < 0 ",
        " MCParticles.m_pdg > 0 ",
        " (MCParticles.m_pdg[MCParticles.m_firstDaughter-1] == 310 && MCParticles.m_pdg[MCParticles.m_firstDaughter] == 443) && ",
        r'$(z_{\rm sig}^{\rm dec})^{\rm gen}\ [{\rm ' + unitImp + '}]$']}


def makeZtagDecayPlot(cutOnUpsilonFourS="", asymplot=True):
    """
    Plots the distribution of the decay z-vertex of the tag-side B0 meson.
    """

    withOrWithoutCut = ""

    if cutOnUpsilonFourS != "":
        withOrWithoutCut = "WithUpsilonFourSCut"
        unitImp = "mm"
        variablesPlotB0tag['DecZ'] = [DecZ, dBw, -0.6, 1.5, r"{\rm " + unitImp + r"}\, "]
        particleCondsB0tag['B0tag'][5] = r'$(z_{\rm tag}^{\rm dec})^{\rm gen}\ [{\rm ' + unitImp + r'}]$'

    for Particle in particleCondsB0tag:
        inputVariable = "DecZ"

        print("Plotting " + Particle + " z-decay Vertex")

        nBins = variablesPlotB0tag[inputVariable][1]
        limXmin = variablesPlotB0tag[inputVariable][2]
        limXmax = variablesPlotB0tag[inputVariable][3]

        negativeHistogram = ROOT.TH1F("negative" + Particle + str(Belle2.makeROOTCompatible(inputVariable)), "",
                                      nBins,
                                      limXmin,
                                      limXmax)
        positiveHistogram = ROOT.TH1F("positive" + Particle + str(Belle2.makeROOTCompatible(inputVariable)), "",
                                      nBins,
                                      limXmin,
                                      limXmax)
        condition = "abs(MCParticles.m_pdg) == " + particleCondsB0tag[Particle][1] + " && "
        condition = condition + cutOnUpsilonFourS
        condition = condition + particleCondsB0tag[Particle][4]

        factorMultiplication = str()

        if belleOrBelle2 != "Belle" or (belleOrBelle2 == "Belle" and cutOnUpsilonFourS != ""):
            factorMultiplication = "*10 "

        tree.Draw(variablesPlotB0tag[inputVariable][0] + factorMultiplication + ">> negative" + Particle +
                  str(Belle2.makeROOTCompatible(inputVariable)), condition + particleCondsB0tag[Particle][2])

        tree.Draw(variablesPlotB0tag[inputVariable][0] + factorMultiplication + ">> positive" + Particle +
                  str(Belle2.makeROOTCompatible(inputVariable)), condition + particleCondsB0tag[Particle][3])

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

        ax1 = plt.axes([0.19, 0.37, 0.76, 0.60])

        ax1.hist(
            negativeArray[:, 0], weights=negativeArray[:, 1], bins=negativeHistogram.GetNbinsX(),
            histtype='step', edgecolor='b', linewidth=4.5, linestyle='dashed',

            label=r'$' + particleCondsB0tag[Particle][0] + '^{-} $')

        ax1.hist(positiveArray[:, 0], weights=positiveArray[:, 1], bins=positiveHistogram.GetNbinsX(),
                 histtype='step', edgecolor='r', linewidth=4, alpha=0.9,
                 label=r'$' + particleCondsB0tag[Particle][0] + '^{+} $')  # hatch='.',

        p1, =  ax1.plot([], label=r'$\bar{B}^0$', linewidth=4.5, linestyle='dashed', c='b')
        p2, =  ax1.plot([], label=r'$B^0$', linewidth=4, linestyle='solid', alpha=0.9, c='r')

        binWidth = negativeHistogram.GetBinWidth(2)

        binWidth = '{:8.3f}'.format(binWidth)

        legendLocation = 1
        ax1.set_ylabel(r'${\rm Fraction\hspace{0.25em} of\hspace{0.25em} Events}\, /\, (\, ' + binWidth + r'\, ' +
                       variablesPlotB0tag[inputVariable][4] + r')$', fontsize=35)
        if inputVariable == 'p' or inputVariable == 'pt':
            legendLocation = 7

        ax1.yaxis.set_major_formatter(FormatStrFormatter(r'$%.2f$'))

        ax1.set_xlim(limXmin, limXmax)

        locs, labels = plt.xticks()
        ax1.tick_params(axis='y', labelsize=37)

        ax1.legend([p1, p2], [r'$\bar{B}^0$', r'$B^0$'], prop={'size': 50}, loc=legendLocation, numpoints=1, handlelength=1)
        ax1.grid(True)
        xLabel = particleCondsB0tag[Particle][5]

        if not asymplot:
            ax1.xaxis.labelpad = 15
            plt.locator_params(axis='x', nbins=len(labels))
            ax1.tick_params(axis='x', labelsize=40)
            ax1.set_xlim(limXmin, limXmax)
            ax1.set_xlabel(xLabel, fontsize=50)

        else:

            empty_string_labels = [''] * len(labels)
            plt.locator_params(axis='x', nbins=len(labels))
            ax1.set_xticklabels(empty_string_labels)

            ax2 = plt.axes([0.19, 0.15, 0.76, 0.2])

            ax2.errorbar(asymmetryArray[:, 0], asymmetryArray[:, 1], xerr=float(negativeHistogram.GetBinWidth(2) / 2),
                         yerr=asymmetryArrayUncertainties[:, 1], elinewidth=2, mew=2, ecolor='k',
                         fmt='o', mfc='k', mec='k', markersize=6, label=r'${\rm Data}$')

            ax2.set_ylabel(
                r'$\frac{f^{B^0}\; -\;\, f^{\overline{B}^0}}{f^{B^0}\; +\;\, f^{\overline{B}^0}}$',
                fontsize=46.5,
                labelpad=20)
            ax2.yaxis.labelpad = 0

            plt.yticks([-0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75],
                       [r'', r'$-0.5$', r'', r'$0.0$', r'', r'$0.5$', r''], rotation=0, size=25)
            ax2.set_ylim(-0.75, 0.75)
            ax2.xaxis.grid(True)
            plt.axhline(y=0.5, linewidth=2, color='tab:gray', linestyle='-')
            plt.axhline(y=0.25, linewidth=2, color='tab:gray', linestyle='-')
            plt.axhline(y=0, linewidth=2, color='tab:gray', linestyle='-')
            plt.axhline(y=-0.25, linewidth=2, color='tab:gray', linestyle='-')
            plt.axhline(y=-0.5, linewidth=2, color='tab:gray', linestyle='-')

            ax2.xaxis.labelpad = 15
            plt.locator_params(axis='x', nbins=len(labels))
            ax2.tick_params(axis='x', labelsize=40)
            ax2.set_xlim(limXmin, limXmax)
            ax2.set_xlabel(xLabel, fontsize=50)

        plt.savefig(workingDirectory + '/GenLevelVariablesPlots' + '/' +
                    belleOrBelle2 + withOrWithoutCut + "_" + Particle + "_" +
                    str(Belle2.makeROOTCompatible(inputVariable)) + '.pdf')
        fig1.clear()

        negativeHistogram.Delete()
        positiveHistogram.Delete()


makePlotsForEachParticleKind("")

if belleOrBelle2 == "Belle":
    makeZtagDecayPlot("")
    makeZtagDecayPlot(" abs(MCParticles.m_decayVertex_z[MCParticles.m_mother-1]) < 0.04 && ")
    makePlotsForEachParticleKind(" abs(MCParticles.m_decayVertex_z[MCParticles.m_mother[MCParticles.m_mother - 1]-1]) < 0.04 && ")
else:
    makeZtagDecayPlot("")
