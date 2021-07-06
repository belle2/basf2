#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2_mva_util

import ftPlotting as plotting
import argparse
import tempfile

import numpy as np
from B2Tools import b2latex, format

import os
import shutil
import collections
import ROOT
from ROOT import Belle2
from flavorTagger import KId, muId, eId


def getCommandLineOptions():
    """ Parses the command line options of the fei and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('-id', '--identifiers', dest='identifiers', type=str, required=True, action='append', nargs='+',
                        help='DB Identifier or weightfile')
    parser.add_argument('-train', '--train_datafiles', dest='train_datafiles', type=str, required=False, action='append', nargs='+',
                        help='Data file containing ROOT TTree used during training')
    parser.add_argument('-data', '--datafiles', dest='datafiles', type=str, required=True, action='append', nargs='+',
                        help='Data file containing ROOT TTree with independent test data')
    parser.add_argument('-tree', '--treename', dest='treename', type=str, default='tree', help='Treename in data file')
    parser.add_argument('-out', '--outputfile', dest='outputfile', type=str, default='output.pdf',
                        help='Name of the outputted pdf file')
    parser.add_argument('-w', '--working_directory', dest='working_directory', type=str, default='',
                        help="""Working directory where the created images and root files are stored,
                              default is to create a temporary directory.""")
    parser.add_argument('-b2Orb', '--BelleOrBelle2', dest='BelleOrBelle2', type=str, default='Belle2',
                        help="""Tell me if this is Belle or Belle2 MC please.""")
    args = parser.parse_args()
    return args


def unique(input):
    """
    Returns a list containing only unique elements, keeps the original order of the list
    @param input list containing the elements
    """
    output = []
    for x in input:
        if x not in output:
            output.append(x)
    return output


def create_abbreviations(names, length=5):

    variablesPlotParamsDict = {'useCMSFrame(p)': [r'$p^*$', r"{\rm GeV}/c\, "],
                               'useCMSFrame(pt)': [r'$p_{\rm t}^*$', r"{\rm GeV}/c\, "],
                               'p': [r'$p$', r"{\rm GeV}/c\, "],
                               'pt': [r'$p_{\rm t}$', r"{\rm GeV}/c\, "],
                               'pLambda': [r'$p_{\Lambda}$', r"{\rm GeV}/c\, "],
                               'useCMSFrame(p)Lambda': [r'$p^*_{\Lambda}$', r"{\rm GeV}/c\, "],
                               'useCMSFrame(p)FSC': [r'$p^*_{\rm Slow}$', r"{\rm GeV}/c\, "],
                               'cosTheta': [r'$\cos{\theta}$', ""],
                               eId[args.BelleOrBelle2]: [r'$\mathcal{L}_{e}$', ""],
                               'eid_dEdx': [r'$\mathcal{L}_{e}^{{\rm d}E/{\rm d}x}$', ""],
                               'eid_TOP': [r'$\mathcal{L}_{e}^{\rm TOP}$', ""],
                               'eid_ARICH': [r'$\mathcal{L}_{e}^{\rm ARICH}$', ""],
                               'eid_ECL': [r'$\mathcal{L}_{e}^{\rm ECL}$', ""],
                               'BtagToWBosonVariables(recoilMassSqrd)': [r'$M_{\rm rec}^2$', r"{\rm GeV}^2/c^4"],
                               'BtagToWBosonVariables(pMissCMS)': [r'$p^*_{\rm miss}$', r"{\rm GeV}/c\, "],
                               'BtagToWBosonVariables(cosThetaMissCMS)': [r'$\cos{\theta^*_{\rm miss}}$', ""],
                               'BtagToWBosonVariables(EW90)': [r'$E_{90}^{W}$', r"{\rm GeV}\, "],
                               'BtagToWBosonVariables(recoilMass)': [r'$M_{\rm rec}$', r"{\rm GeV}/c^2\, "],
                               'cosTPTO': [r'$\vert\cos{\theta^*_{\rm T}}\vert$', ""],
                               'cosTPTOFSC': [r'$\vert\cos{\theta^*_{\rm T,Slow}}\vert$', ""],
                               'ImpactXY': [r'$d_0$', r"{\rm mm}\, "],
                               'distance': [r'$\xi_0$', r"{\rm mm}\, "],
                               'chiProb': [r'$p$-${\rm value}$', ""],
                               muId[args.BelleOrBelle2]: [r'$\mathcal{L}_{\mu}$', ""],
                               'muid_dEdx': [r'$\mathcal{L}_{\mu}^{{\rm d}E/{\rm d}x}$', ""],
                               'muid_TOP': [r'$\mathcal{L}_{\mu}^{\rm TOP}$', ""],
                               'muid_ARICH': [r'$\mathcal{L}_{\mu}^{\rm ARICH}$', ""],
                               'muid_KLM': [r'$\mathcal{L}_{\mu}^{\rm KLM}$', ""],
                               KId[args.BelleOrBelle2]: [r'$\mathcal{L}_{K}$', ""],
                               'Kid_dEdx': [r'$\mathcal{L}_{K}^{{\rm d}E/{\rm d}x}$', ""],
                               'Kid_TOP': [r'$\mathcal{L}_{K}^{\rm TOP}$', ""],
                               'Kid_ARICH': [r'$\mathcal{L}_{K}^{\rm ARICH}$', ""],
                               'NumberOfKShortsInRoe': [r'$n_{K^0_S}$', ""],
                               'ptTracksRoe': [r'$\Sigma\, p_{\rm t}^2$', r"{\rm GeV^2}/c^2"],
                               'extraInfo(isRightCategory(Kaon))': [r"$y_{\rm Kaon}$", ""],
                               'HighestProbInCat(pi+:inRoe, isRightCategory(SlowPion))': [r"$y_{\rm SlowPion}$", ""],
                               'KaonPionVariables(cosKaonPion)': [r'$\cos{\theta^*_{K\pi}}$', ""],
                               'KaonPionVariables(HaveOpositeCharges)': [r'$\frac{1 - q_{K} \cdot q_\pi}{2}$', ""],
                               'pionID': [r'$\mathcal{L}_{\pi}$', ""],
                               'piid_dEdx': [r'$\mathcal{L}_{\pi}^{{\rm d}E/{\rm d}x}$', ""],
                               'piid_TOP': [r'$\mathcal{L}_{\pi}^{\rm TOP}$', ""],
                               'piid_ARICH': [r'$\mathcal{L}_{\pi}^{\rm ARICH}$', ""],
                               'pi_vs_edEdxid': [r'$\mathcal{L}_{\pi/e}^{{\rm d}E/{\rm d}x}$', ""],
                               'FSCVariables(pFastCMS)': [r'$p^*_{\rm Fast}$', r"{\rm GeV}/c\, "],
                               'FSCVariables(cosSlowFast)': [r'$\cos{\theta^*_{\rm SlowFast}}$', ''],
                               'FSCVariables(cosTPTOFast)': [r'$\vert\cos{\theta^*_{\rm T, Fast}}\vert$', ''],
                               'FSCVariables(SlowFastHaveOpositeCharges)': [r'$\frac{1 - q_{\rm Slow} \cdot q_{\rm Fast}}{2}$', ""],
                               'lambdaFlavor': [r'$q_{\Lambda}$', ""],
                               'M': [r'$M_{\Lambda}$', r"{\rm MeV}/c^2\, "],
                               'cosAngleBetweenMomentumAndVertexVector': [
        r'$\cos{\theta_{\boldsymbol{x}_{\Lambda},\boldsymbol{p}_{\Lambda}}}$', ""],
        'lambdaZError': [r'$\sigma_{\Lambda}^{zz}$', r"{\rm mm}\, "],
        'daughter(0,p)': [r'$p_{\pi}$', r"{\rm GeV}/c\, "],
        'daughter(0,useCMSFrame(p))': [r'$p^*_{\pi}$', r"{\rm GeV}/c\, "],
        'daughter(1,p)': [r'$p_{p}$', r"{\rm GeV}/c"],
        'daughter(1,useCMSFrame(p))': [r'$p^*_{p}$', r"{\rm GeV}/c\, "],
        'daughter(1,protonID)': [r'$\mathcal{L}_{p}$', ""],
        'daughter(0,pionID)': [r'$\mathcal{L}_{\pi}$', ""],
        'QpOf(mu+:inRoe, isRightCategory(IntermediateMuon), isRightCategory(IntermediateMuon))': [
        r'${\rm Int.\ Muon}$'],
        'QpOf(mu+:inRoe, isRightCategory(Muon), isRightCategory(Muon))': [r'${\rm Muon}$'],
        'QpOf(pi+:inRoe, isRightCategory(FSC), isRightCategory(SlowPion))': [r'${\rm FSC}$'],
        'QpOf(e+:inRoe, isRightCategory(Electron), isRightCategory(Electron))': [r'${\rm Electron}$'],
        'QpOf(e+:inRoe, isRightCategory(IntermediateElectron), isRightCategory(IntermediateElectron))': [
        r'${\rm Int.\ El.}$'],
        'weightedQpOf(Lambda0:inRoe, isRightCategory(Lambda), isRightCategory(Lambda))': [r'${\rm Lambda}$'],
        'QpOf(K+:inRoe, isRightCategory(KaonPion), isRightCategory(Kaon))': [r'${\rm Kaon}$' + '-' + r'${\rm Pion}$'],
        'QpOf(pi+:inRoe, isRightCategory(FastHadron), isRightCategory(FastHadron))': [r'${\rm Fast\ Hadron}$'],
        'QpOf(mu+:inRoe, isRightCategory(IntermediateKinLepton), isRightCategory(IntermediateKinLepton))': [
        r'${\rm Int.\ Kin.\ Lep.}$'],
        'QpOf(pi+:inRoe, isRightCategory(MaximumPstar), isRightCategory(MaximumPstar))': [r'${\rm Max.}\,p^*$'],
        'QpOf(pi+:inRoe, isRightCategory(SlowPion), isRightCategory(SlowPion))': [r'${\rm Slow\ Pion}$'],
        'QpOf(mu+:inRoe, isRightCategory(KinLepton), isRightCategory(KinLepton))': [r'${\rm Kin.\ Lep.}$'],
        'weightedQpOf(K+:inRoe, isRightCategory(Kaon), isRightCategory(Kaon))': [r'${\rm Kaon}$']}

    if sum(args.identifiers, [])[0].find('LevelLambdaFBDT') != -1:
        variablesPlotParamsDict['distance'] = [r'$\vert \boldsymbol{x}_{\Lambda}\vert$', r"{\rm mm}\, "]

    count = dict()

    for name in names:

        if name in variablesPlotParamsDict:
            abbreviation = variablesPlotParamsDict[name][0]
        else:
            abbreviation = name[:length]

        if abbreviation not in count:
            count[abbreviation] = 0
        count[abbreviation] += 1
    abbreviations = collections.OrderedDict()

    count2 = dict()
    for name in names:

        if name in variablesPlotParamsDict:
            abbreviation = variablesPlotParamsDict[name][0]
        else:
            abbreviation = name[:length]

        abbreviations[name] = abbreviation
        if count[abbreviation] > 1:
            if abbreviation not in count2:
                count2[abbreviation] = 0
            count2[abbreviation] += 1
            abbreviations[name] += str(count2[abbreviation])
    return abbreviations


if __name__ == '__main__':

    ROOT.gROOT.SetBatch(True)

    old_cwd = os.getcwd()
    args = getCommandLineOptions()

    identifiers = sum(args.identifiers, [])
    identifier_abbreviations = create_abbreviations(identifiers)

    datafiles = sum(args.datafiles, [])

    print("Load methods")
    methods = [basf2_mva_util.Method(identifier) for identifier in identifiers]

    print("Apply experts on independent data")
    test_probability = {}
    test_target = {}
    for method in methods:
        p, t = method.apply_expert(datafiles, args.treename)
        test_probability[identifier_abbreviations[method.identifier]] = p
        test_target[identifier_abbreviations[method.identifier]] = t

    print("Apply experts on training data")
    train_probability = {}
    train_target = {}
    if args.train_datafiles is not None:
        train_datafiles = sum(args.train_datafiles, [])
        for method in methods:
            p, t = method.apply_expert(train_datafiles, args.treename)
            train_probability[identifier_abbreviations[method.identifier]] = p
            train_target[identifier_abbreviations[method.identifier]] = t

    variables = unique(v for method in methods for v in method.variables)
    root_variables = unique(v for method in methods for v in method.root_variables)

    print("Here Variables")
    print(variables)

    bkgrOutput = 0
    displayHeatMap = False
    classOutputLabel = r'${\rm Classifier\ Output}$'
    isNN = False

    if identifiers[0].find('Combiner') != -1 or identifiers[0].find('KaonFBDT') != -1 or \
       identifiers[0].find('Electron') != -1 or identifiers[0].find('Muon') != -1 or \
       identifiers[0].find('Lepton') != -1 or \
       identifiers[0].find('SlowPion') != -1 or identifiers[0].find('FastHadron') != -1 or \
       identifiers[0].find('KaonPion') != -1 or identifiers[0].find('FSC') != -1 or \
       identifiers[0].find('MaximumPstar') != -1 or identifiers[0].find('Lambda') != -1:

        if identifiers[0].find('Combiner') != -1:
            displayHeatMap = True
            bkgrOutput = -1

            variables = [
                'weightedQpOf(Lambda0:inRoe, isRightCategory(Lambda), isRightCategory(Lambda))',
                'QpOf(pi+:inRoe, isRightCategory(FastHadron), isRightCategory(FastHadron))',
                'QpOf(pi+:inRoe, isRightCategory(MaximumPstar), isRightCategory(MaximumPstar))',
                'QpOf(pi+:inRoe, isRightCategory(FSC), isRightCategory(SlowPion))',
                'QpOf(pi+:inRoe, isRightCategory(SlowPion), isRightCategory(SlowPion))',
                'QpOf(K+:inRoe, isRightCategory(KaonPion), isRightCategory(Kaon))',
                'weightedQpOf(K+:inRoe, isRightCategory(Kaon), isRightCategory(Kaon))',
                'QpOf(mu+:inRoe, isRightCategory(IntermediateKinLepton), isRightCategory(IntermediateKinLepton))',
                'QpOf(mu+:inRoe, isRightCategory(KinLepton), isRightCategory(KinLepton))',
                'QpOf(mu+:inRoe, isRightCategory(IntermediateMuon), isRightCategory(IntermediateMuon))',
                'QpOf(mu+:inRoe, isRightCategory(Muon), isRightCategory(Muon))',
                'QpOf(e+:inRoe, isRightCategory(IntermediateElectron), isRightCategory(IntermediateElectron))',
                'QpOf(e+:inRoe, isRightCategory(Electron), isRightCategory(Electron))'
            ]
            variables = list(reversed(variables))

        if identifiers[0].find('Electron') != -1:

            variables = [
                'useCMSFrame(p)',
                'useCMSFrame(pt)',
                'p',
                'pt',
                'cosTheta',
                'ImpactXY',
                'distance',
                'BtagToWBosonVariables(recoilMassSqrd)',
                'BtagToWBosonVariables(EW90)',
                'BtagToWBosonVariables(pMissCMS)',
                'BtagToWBosonVariables(cosThetaMissCMS)',
                'cosTPTO',
                'eid_dEdx',
                'eid_TOP',
                'eid_ARICH',
                'eid_ECL',
                eId[args.BelleOrBelle2],
                'chiProb']

        if identifiers[0].find('Muon') != -1:

            variables = [
                'useCMSFrame(p)',
                'useCMSFrame(pt)',
                'p',
                'pt',
                'cosTheta',
                'ImpactXY',
                'distance',
                'BtagToWBosonVariables(recoilMassSqrd)',
                'BtagToWBosonVariables(EW90)',
                'BtagToWBosonVariables(pMissCMS)',
                'BtagToWBosonVariables(cosThetaMissCMS)',
                'cosTPTO',
                'muid_dEdx',
                'muid_TOP',
                'muid_ARICH',
                'muid_KLM',
                muId[args.BelleOrBelle2],
                'chiProb']

        if identifiers[0].find('Lepton') != -1:

            variables = [
                'useCMSFrame(p)',
                'useCMSFrame(pt)',
                'p',
                'pt',
                'cosTheta',
                'ImpactXY',
                'distance',
                'BtagToWBosonVariables(recoilMassSqrd)',
                'BtagToWBosonVariables(EW90)',
                'BtagToWBosonVariables(pMissCMS)',
                'BtagToWBosonVariables(cosThetaMissCMS)',
                'cosTPTO',
                'eid_dEdx',
                'eid_TOP',
                'eid_ARICH',
                'eid_ECL',
                eId[args.BelleOrBelle2],
                'muid_dEdx',
                'muid_TOP',
                'muid_ARICH',
                'muid_KLM',
                muId[args.BelleOrBelle2],
                'chiProb']

        if identifiers[0].find('KaonFBDT') != -1:
            displayHeatMap = True
            variables = ['useCMSFrame(p)', 'useCMSFrame(pt)', 'pt', 'ptTracksRoe', 'cosTheta', 'ImpactXY', 'distance',
                         'BtagToWBosonVariables(recoilMassSqrd)', 'BtagToWBosonVariables(EW90)',
                         'BtagToWBosonVariables(pMissCMS)', 'BtagToWBosonVariables(cosThetaMissCMS)',
                         'cosTPTO', 'Kid_dEdx', 'Kid_TOP', 'Kid_ARICH', KId[args.BelleOrBelle2],
                         'NumberOfKShortsInRoe', 'chiProb']

        if identifiers[0].find('SlowPion') != -1 or identifiers[0].find('FastHadron') != -1:

            variables = [
                'useCMSFrame(p)',
                'useCMSFrame(pt)',
                'p',
                'pt',
                'cosTheta',
                'ImpactXY',
                'distance',
                'BtagToWBosonVariables(recoilMassSqrd)',
                'BtagToWBosonVariables(EW90)',
                'BtagToWBosonVariables(pMissCMS)',
                'BtagToWBosonVariables(cosThetaMissCMS)',
                'cosTPTO',
                'Kid_dEdx', 'Kid_TOP', 'Kid_ARICH', KId[args.BelleOrBelle2],
                'piid_dEdx',
                'piid_TOP',
                'piid_ARICH',
                'pionID',
                'pi_vs_edEdxid',
                eId[args.BelleOrBelle2],
                'ptTracksRoe',
                'NumberOfKShortsInRoe',
                'chiProb']

        if identifiers[0].find('KaonPion') != -1:

            variables = [KId[args.BelleOrBelle2], 'extraInfo(isRightCategory(Kaon))',
                         'HighestProbInCat(pi+:inRoe, isRightCategory(SlowPion))',
                         'KaonPionVariables(cosKaonPion)',
                         'KaonPionVariables(HaveOpositeCharges)'
                         ]

        if identifiers[0].find('FSC') != -1:

            variables = [KId[args.BelleOrBelle2], 'useCMSFrame(p)',
                         'FSCVariables(pFastCMS)',
                         'FSCVariables(SlowFastHaveOpositeCharges)',
                         'cosTPTO',
                         'FSCVariables(cosTPTOFast)',
                         'FSCVariables(cosSlowFast)']

        if identifiers[0].find('MaximumPstar') != -1:

            variables = ['useCMSFrame(p)',
                         'useCMSFrame(pt)',
                         'p',
                         'pt',
                         'ImpactXY',
                         'distance', 'cosTPTO']

        if identifiers[0].find('Lambda') != -1:
            displayHeatMap = True
            if args.BelleOrBelle2 == 'Belle':
                variables = [
                    'useCMSFrame(p)',
                    'p',
                    'daughter(0,useCMSFrame(p))',
                    'daughter(0,p)',
                    'daughter(1,useCMSFrame(p))',
                    'daughter(1,p)',
                    'lambdaFlavor',
                    'M',
                    'NumberOfKShortsInRoe',
                    'cosAngleBetweenMomentumAndVertexVector',
                    'distance',
                    'lambdaZError',
                    'chiProb']
            else:
                variables = [
                    'daughter(0,pionID)',
                    'daughter(1,protonID)',
                    'useCMSFrame(p)',
                    'p',
                    'daughter(0,useCMSFrame(p))',
                    'daughter(0,p)',
                    'daughter(1,useCMSFrame(p))',
                    'daughter(1,p)',
                    'lambdaFlavor',
                    'M',
                    'NumberOfKShortsInRoe',
                    'cosAngleBetweenMomentumAndVertexVector',
                    'distance',
                    'lambdaZError',
                    'chiProb']

        variables = list(reversed(variables))
        for iVarPosition in range(len(variables)):
            root_variables[iVarPosition] = Belle2.makeROOTCompatible(variables[iVarPosition])

        if identifiers[0].find('FSC') != -1:

            variables = [KId[args.BelleOrBelle2], 'useCMSFrame(p)FSC',
                         'FSCVariables(pFastCMS)',
                         'FSCVariables(SlowFastHaveOpositeCharges)',
                         'cosTPTOFSC',
                         'FSCVariables(cosTPTOFast)',
                         'FSCVariables(cosSlowFast)']

        if identifiers[0].find('Lambda') != -1:
            displayHeatMap = True
            if args.BelleOrBelle2 == 'Belle':
                variables = [
                    'useCMSFrame(p)Lambda',
                    'pLambda',
                    'daughter(0,useCMSFrame(p))',
                    'daughter(0,p)',
                    'daughter(1,useCMSFrame(p))',
                    'daughter(1,p)',
                    'lambdaFlavor',
                    'M',
                    'NumberOfKShortsInRoe',
                    'cosAngleBetweenMomentumAndVertexVector',
                    'distance',
                    'lambdaZError',
                    'chiProb']
            else:
                variables = [
                    'daughter(0,pionID)',
                    'daughter(1,protonID)',
                    'useCMSFrame(p)Lambda',
                    'pLambda',
                    'daughter(0,useCMSFrame(p))',
                    'daughter(0,p)',
                    'daughter(1,useCMSFrame(p))',
                    'daughter(1,p)',
                    'lambdaFlavor',
                    'M',
                    'NumberOfKShortsInRoe',
                    'cosAngleBetweenMomentumAndVertexVector',
                    'distance',
                    'lambdaZError',
                    'chiProb']

    if identifiers[0].find('Combiner') != -1:
        if identifiers[0].find('FANN') != -1:
            classOutputLabel = r'$(q\cdot r)_{\rm MLP}$'
            isNN = True
        if identifiers[0].find('FBDT') != -1:
            classOutputLabel = r'$(q\cdot r)_{\rm FBDT}$'
    elif identifiers[0].find('LevelMaximumPstar') != -1:
        classOutputLabel = r'$y_{{\rm Maximum}\, p^*}$'
    elif identifiers[0].find('LevelFSCFBDT') != -1:
        classOutputLabel = r'$y_{\rm FSC}$'
    elif identifiers[0].find('LevelMuonFBDT') != -1:
        classOutputLabel = r'$y_{\rm Muon}$'
    elif identifiers[0].find('LevelElectronFBDT') != -1:
        classOutputLabel = r'$y_{\rm Electron}$'
    elif identifiers[0].find('LevelKaonFBDT') != -1:
        classOutputLabel = r'$y_{\rm Kaon}$'
    elif identifiers[0].find('LevelLambdaFBDT') != -1:
        classOutputLabel = r'$y_{\rm Lambda}$'
    elif identifiers[0].find('LevelIntermediateKinLeptonFBDT') != -1:
        classOutputLabel = r'$y_{\rm Int.\, Kin.\, Lepton}$'
    elif identifiers[0].find('LevelKinLeptonFBDT') != -1:
        classOutputLabel = r'$y_{\rm Kin.\, Lepton}$'
    elif identifiers[0].find('LevelIntermediateMuon') != -1:
        classOutputLabel = r'$y_{\rm Int.\, Muon}$'
    elif identifiers[0].find('LevelIntermediateElectron') != -1:
        classOutputLabel = r'$y_{\rm Int.\, Electron}$'
    elif identifiers[0].find('LevelKaonPionFBDT') != -1:
        classOutputLabel = r'$y_{\rm Kaon-Pion}$'
    elif identifiers[0].find('LevelFastHadron') != -1:
        classOutputLabel = r'$y_{\rm Fast\, Hadron}$'
    elif identifiers[0].find('LevelSlowPion') != -1:
        classOutputLabel = r'$y_{\rm Slow\, Pion}$'

    variable_abbreviations = create_abbreviations(variables)

    spectators = unique(v for method in methods for v in method.spectators)
    spectator_abbreviations = create_abbreviations(spectators)
    root_spectators = unique(v for method in methods for v in method.root_spectators)

    print("Load variables array")
    rootchain = ROOT.TChain(args.treename)
    for datafile in datafiles:
        rootchain.Add(datafile)

    variables_data = basf2_mva_util.tree2dict(rootchain, root_variables, list(variable_abbreviations.values()))
    spectators_data = basf2_mva_util.tree2dict(rootchain, root_spectators, list(spectator_abbreviations.values()))

    print("Create latex file")
    # Change working directory after experts run, because they might want to access
    # a locadb in the current working directory
    with tempfile.TemporaryDirectory() as tempdir:
        if args.working_directory == '':
            os.chdir(tempdir)
        else:
            os.chdir(args.working_directory)

        o = b2latex.LatexFile()
        o += b2latex.TitlePage(title='Automatic MVA Evaluation',
                               authors=[r'Thomas Keck\\ Moritz Gelb\\ Nils Braun'],
                               abstract='Evaluation plots',
                               add_table_of_contents=True).finish()

        o += b2latex.Section("Classifiers")
        o += b2latex.String(r"""
            This section contains the GeneralOptions and SpecificOptions of all classifiers represented by an XML tree.
            The same information can be retreived using the basf2\_mva\_info tool.
        """)

        table = b2latex.LongTable(r"ll", "Abbreviations of identifiers", "{name} & {abbr}", r"Identifier & Abbreviation")
        for identifier in identifiers:
            table.add(name=format.string(identifier), abbr=format.string(identifier_abbreviations[identifier]))
        o += table.finish()

#        for method in methods:
#            o += b2latex.SubSection(format.string(method.identifier))
#            o += b2latex.Listing(language='XML').add(method.description).finish()

        o += b2latex.Section("Variables")
        o += b2latex.String("""
            This section contains an overview of the importance and correlation of the variables used by the classifiers.
            And distribution plots of the variables on the independent dataset. The distributions are normed for signal and
            background separately, and only the region +- 3 sigma around the mean is shown.
        """)

        table = b2latex.LongTable(r"ll", "Abbreviations of variables", "{name} & {abbr}", r"Variable & Abbreviation")
        for v in variables:
            # table.add(name=format.string(v), abbr=format.string(variable_abbreviations[v]))
            table.add(name=format.string(v), abbr=variable_abbreviations[v])
        o += table.finish()

        o += b2latex.SubSection("Importance")
        graphics = b2latex.Graphics()
        p = plotting.Importance()
        p.add({identifier_abbreviations[i.identifier]: np.array([i.importances.get(v, 0.0) for v in variables]) for i in methods},
              identifier_abbreviations.values(), variable_abbreviations.values(), displayHeatMap)
        p.finish()
        p.save('importance.pdf')
        graphics.add('importance.pdf', width=1.0)
        o += graphics.finish()

        o += b2latex.SubSection("Correlation")
        first_identifier_abbr = list(identifier_abbreviations.values())[0]
        graphics = b2latex.Graphics()
        p = plotting.CorrelationMatrix()
        p.add(variables_data, variable_abbreviations.values(),
              test_target[first_identifier_abbr] == 1,
              test_target[first_identifier_abbr] == bkgrOutput, bkgrOutput)
        p.finish()
        p.save('correlation_plot.pdf')
        graphics.add('correlation_plot.pdf', width=1.0)
        o += graphics.finish()

        if False:
            graphics = b2latex.Graphics()
            p = plotting.TSNE()
            p.add(variables_data, variable_abbreviations.values(),
                  test_target[first_identifier_abbr] == 1,
                  test_target[first_identifier_abbr] == bkgrOutput)
            p.finish()
            p.save('tsne_plot.pdf')
            graphics.add('tsne_plot.pdf', width=1.0)
            o += graphics.finish()

#        for v in variables:
#            variable_abbr = variable_abbreviations[v]
#            o += b2latex.SubSection(format.string(v))
#            graphics = b2latex.Graphics()
#            p = plotting.VerboseDistribution(normed=True, range_in_std=3)
#            p.add(variables_data, variable_abbr, test_target[first_identifier_abbr] == 1, label=r"${\rm Signal}$")
#            p.add(variables_data, variable_abbr, test_target[first_identifier_abbr] == bkgrOutput, label=r"${\rm Background}$")
#            p.finish()
#            p.save('variable_{}.pdf'.format(hash(v)))
#            graphics.add('variable_{}.pdf'.format(hash(v)), width=1.0)
#            o += graphics.finish()

        o += b2latex.Section("Classifier Plot")
        o += b2latex.String("This section contains the receiver operating characteristics (ROC), purity projection, ..."
                            "of the classifiers on training and independent data."
                            "The legend of each plot contains the shortened identifier and the area under the ROC curve"
                            "in parenthesis.")

        o += b2latex.Section("ROC Plot")
        graphics = b2latex.Graphics()
        p = plotting.RejectionOverEfficiency()
        for identifier in identifier_abbreviations.values():
            auc = p.add(test_probability, identifier, test_target[identifier] == 1, test_target[identifier] == bkgrOutput)
            o += b2latex.String("This is the Area under the ROC " + " ({:.2f})".format(auc) + ".")
            f = open("AUCROCTest.txt", "w")
            f.write("{:.6f}".format(auc))
            f.close()
        p.finish()
        # p.axis.set_title("ROC Rejection Plot on independent data")
        p.save('roc_plot_test.pdf')
        graphics.add('roc_plot_test.pdf', width=1.0)
        o += graphics.finish()

        if train_probability:
            for i, identifier in enumerate(identifiers):
                graphics = b2latex.Graphics()
                p = plotting.RejectionOverEfficiency()
                identifier_abbr = identifier_abbreviations[identifier]
                aucTrain = p.add(train_probability, identifier_abbr, train_target[identifier_abbr] == 1,
                                 train_target[identifier_abbr] == bkgrOutput, label=r'{\rm Train}')
                o += b2latex.String("This is the Area under the train ROC  " + " ({:.2f})".format(aucTrain) + ". ")
                f = open("AUCROCTrain.txt", "w")
                f.write("{:.6f}".format(auc))
                f.close()
                aucTest = p.add(test_probability, identifier_abbr, test_target[identifier_abbr] == 1,
                                test_target[identifier_abbr] == bkgrOutput, label=r'{\rm Test}')
                o += b2latex.String("This is the Area under the test ROC  " + " ({:.2f})".format(aucTest) + ".")
                p.finish()
                # p.axis.set_title(identifier)
                p.save('roc_test.pdf')
                graphics.add('roc_test.pdf', width=1.0)
                o += graphics.finish()

        o += b2latex.Section("Classification Results")

        for identifier in identifiers:
            identifier_abbr = identifier_abbreviations[identifier]
            o += b2latex.SubSection(format.string(identifier_abbr))
            graphics = b2latex.Graphics()
            p = plotting.Multiplot(plotting.PurityAndEfficiencyOverCut, 2)
            p.add(0, test_probability, identifier_abbr, test_target[identifier_abbr] == 1,
                  test_target[identifier_abbr] == bkgrOutput, normed=True)
            p.sub_plots[0].axis.set_title("Classification result in test data ")

            p.add(1, test_probability, identifier_abbr, test_target[identifier_abbr] == 1,
                  test_target[identifier_abbr] == bkgrOutput, normed=False)
            p.sub_plots[1].axis.set_title("Classification result in test data ")
            p.finish()

            p.save('classification_result.pdf')
            graphics.add('classification_result.pdf', width=1)
            o += graphics.finish()

        o += b2latex.Section("Diagonal Plot")
        graphics = b2latex.Graphics()
        p = plotting.Diagonal()
        for identifier in identifiers:
            o += b2latex.SubSection(format.string(identifier_abbr))
            identifier_abbr = identifier_abbreviations[identifier]
            p.add(test_probability, identifier_abbr, test_target[identifier_abbr] == 1, test_target[identifier_abbr] == bkgrOutput)
        p.finish()
        p.axis.set_title("Diagonal plot on independent data")
        p.save('diagonal_plot_test.pdf')
        graphics.add('diagonal_plot_test.pdf', width=1.0)
        o += graphics.finish()

        if train_probability:
            o += b2latex.SubSection("Overtraining Plot")
            for identifier in identifiers:
                identifier_abbr = identifier_abbreviations[identifier]
                probability = {identifier_abbr: np.r_[train_probability[identifier_abbr], test_probability[identifier_abbr]]}
                target = np.r_[train_target[identifier_abbr], test_target[identifier_abbr]]
                train_mask = np.r_[np.ones(len(train_target[identifier_abbr])), np.zeros(len(test_target[identifier_abbr]))]
                graphics = b2latex.Graphics()
                p = plotting.Overtraining()
                p.add(probability, identifier_abbr,
                      train_mask == 1, train_mask == 0,
                      target == 1, target == bkgrOutput, None, bkgrOutput, isNN)
                p.finish(xLabel=classOutputLabel)
                # p.axis.set_title("Overtraining check for " + str(identifier))
                p.save('overtraining_plot.pdf')
                graphics.add('overtraining_plot.pdf', width=1.0)
                o += graphics.finish()
        print("Finished Overtraining plot")

        o += b2latex.Section("Spectators")
        o += b2latex.String("This section contains the distribution and dependence on the"
                            "classifier outputs of all spectator variables.")

        table = b2latex.LongTable(r"ll", "Abbreviations of spectators", "{name} & {abbr}", r"Spectator & Abbreviation")
        for s in spectators:
            table.add(name=format.string(s), abbr=format.string(spectator_abbreviations[s]))
        o += table.finish()

        for spectator in spectators:
            spectator_abbr = spectator_abbreviations[spectator]
            o += b2latex.SubSection(format.string(spectator))
            graphics = b2latex.Graphics()
            p = plotting.VerboseDistribution()
            p.add(spectators_data, spectator_abbr, test_target[first_identifier_abbr] == 1, label="Signal")
            p.add(spectators_data, spectator_abbr, test_target[first_identifier_abbr] == bkgrOutput, label="Background")
            p.finish()
            p.save('spectator_{}.pdf'.format(hash(spectator)))
            graphics.add('spectator_{}.pdf'.format(hash(spectator)), width=1.0)
            o += graphics.finish()

            for identifier in identifiers:
                o += b2latex.SubSubSection(format.string(spectator) + " with classifier " + format.string(identifier))
                identifier_abbr = identifier_abbreviations[identifier]
                data = {identifier_abbr: test_probability[identifier_abbr], spectator_abbr: spectators_data[spectator_abbr]}
                graphics = b2latex.Graphics()
                p = plotting.Correlation()
                p.add(data, spectator_abbr, identifier_abbr, list(range(10, 100, 10)),
                      test_target[identifier_abbr] == 1,
                      test_target[identifier_abbr] == bkgrOutput)
                p.finish()
                p.save('correlation_plot_{}_{}.pdf'.format(hash(spectator), hash(identifier)))
                graphics.add('correlation_plot_{}_{}.pdf'.format(hash(spectator), hash(identifier)), width=1.0)
                o += graphics.finish()

        o.save('latex.tex', compile=True)
        os.chdir(old_cwd)
        if args.working_directory == '':
            shutil.copy(tempdir + '/latex.pdf', args.outputfile)
        else:
            shutil.copy(args.working_directory + '/latex.pdf', args.outputfile)
