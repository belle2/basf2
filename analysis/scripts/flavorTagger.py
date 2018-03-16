#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# *************  Flavor Tagging   ************
# * Authors: Fernando Abudinen, Moritz Gelb  *
# *.....     and Thomas Keck                 *
# * This script is needed to train           *
# * and to test the flavor tagger.           *
# ********************************************

from basf2 import *
from modularAnalysis import *
import basf2_mva
from variables import variables as flavorTaggerVariables
from ROOT import Belle2
import os
import glob


def setBelleOrBelle2(belleOrBelle2='Belle2'):
    """
    Sets belleOrBelle2Flag and the Revision of weight files according to the specified arguments.
    """

    global belleOrBelle2Flag

    belleOrBelle2Flag = belleOrBelle2


def getBelleOrBelle2():
    """
    Gets the global ModeCode.
    """
    return belleOrBelle2Flag


def setInteractionWithDatabase(downloadFromDatabaseIfNotfound=True, uploadToDatabaseAfterTraining=False):
    """
    Sets the interaction with the database: download trained weight files or upload weight files after training.
    """

    global downloadFlag
    global uploadFlag

    downloadFlag = downloadFromDatabaseIfNotfound
    uploadFlag = uploadToDatabaseAfterTraining


# Options for Track and Event Levels
fastBDTCategories = basf2_mva.FastBDTOptions()
fastBDTCategories.m_nTrees = 500
fastBDTCategories.m_nCuts = 8
fastBDTCategories.m_nLevels = 3
fastBDTCategories.m_shrinkage = 0.10
fastBDTCategories.m_randRatio = 0.5

# Options for Combiner Level
fastBDTCombiner = basf2_mva.FastBDTOptions()
fastBDTCombiner.m_nTrees = 500
fastBDTCombiner.m_nCuts = 8
fastBDTCombiner.m_nLevels = 3
fastBDTCombiner.m_shrinkage = 0.10
fastBDTCombiner.m_randRatio = 0.5

mlpFANNCombiner = basf2_mva.FANNOptions()
mlpFANNCombiner.m_max_epochs = 10000
mlpFANNCombiner.m_hidden_layers_architecture = "3*N"
mlpFANNCombiner.m_hidden_activiation_function = "FANN_SIGMOID_SYMMETRIC"
mlpFANNCombiner.m_output_activiation_function = "FANN_SIGMOID_SYMMETRIC"
mlpFANNCombiner.m_error_function = "FANN_ERRORFUNC_LINEAR"
mlpFANNCombiner.m_training_method = "FANN_TRAIN_RPROP"
mlpFANNCombiner.m_validation_fraction = 0.5
mlpFANNCombiner.m_random_seeds = 10
mlpFANNCombiner.m_test_rate = 500
mlpFANNCombiner.m_number_of_threads = 8
mlpFANNCombiner.m_scale_features = True
mlpFANNCombiner.m_scale_target = False
# mlpFANNCombiner.m_scale_target = True

# SignalFraction: FBDT feature
# For smooth output set to -1, this will break the calibration.
# For correct calibration set to -2, leads to peaky combiner output.
signalFraction = -2

# Maximal number of events to train each method
maxEventsNumber = 500000

# Definition of all available categories, 'standard category name':
# ['ParticleList', 'trackLevel category name', 'eventLevel category name',
# 'combinerLevel variable name', 'category code']
AvailableCategories = {
    'Electron': [
        'e+:inRoe',
        'Electron',
        'Electron',
        'QpOf(e+:inRoe, isRightCategory(Electron), isRightCategory(Electron))',
        0],
    'IntermediateElectron': [
        'e+:inRoe',
        'IntermediateElectron',
        'IntermediateElectron',
        'QpOf(e+:inRoe, isRightCategory(IntermediateElectron), isRightCategory(IntermediateElectron))',
        1],
    'Muon': [
        'mu+:inRoe',
        'Muon',
        'Muon',
        'QpOf(mu+:inRoe, isRightCategory(Muon), isRightCategory(Muon))',
        2],
    'IntermediateMuon': [
        'mu+:inRoe',
        'IntermediateMuon',
        'IntermediateMuon',
        'QpOf(mu+:inRoe, isRightCategory(IntermediateMuon), isRightCategory(IntermediateMuon))',
        3],
    'KinLepton': [
        'mu+:inRoe',
        'KinLepton',
        'KinLepton',
        'QpOf(mu+:inRoe, isRightCategory(KinLepton), isRightCategory(KinLepton))',
        4],
    'IntermediateKinLepton': [
        'mu+:inRoe',
        'IntermediateKinLepton',
        'IntermediateKinLepton',
        'QpOf(mu+:inRoe, isRightCategory(IntermediateKinLepton), isRightCategory(IntermediateKinLepton))',
        5],
    'Kaon': [
        'K+:inRoe',
        'Kaon',
        'Kaon',
        'weightedQpOf(K+:inRoe, isRightCategory(Kaon), isRightCategory(Kaon))',
        6],
    'SlowPion': [
        'pi+:inRoe',
        'SlowPion',
        'SlowPion',
        'QpOf(pi+:inRoe, isRightCategory(SlowPion), isRightCategory(SlowPion))',
        7],
    'FastHadron': [
        'pi+:inRoe',
        'FastHadron',
        'FastHadron',
        'QpOf(pi+:inRoe, isRightCategory(FastHadron), isRightCategory(FastHadron))',
        8],
    'Lambda': [
        'Lambda0:inRoe',
        'Lambda',
        'Lambda',
        'weightedQpOf(Lambda0:inRoe, isRightCategory(Lambda), isRightCategory(Lambda))',
        9],
    'FSC': [
        'pi+:inRoe',
        'SlowPion',
        'FSC',
        'QpOf(pi+:inRoe, isRightCategory(FSC), isRightCategory(SlowPion))',
        10],
    'MaximumPstar': [
        'pi+:inRoe',
        'MaximumPstar',
        'MaximumPstar',
        'QpOf(pi+:inRoe, isRightCategory(MaximumPstar), isRightCategory(MaximumPstar))',
        11],
    'KaonPion': [
        'K+:inRoe',
        'Kaon',
        'KaonPion',
        'QpOf(K+:inRoe, isRightCategory(KaonPion), isRightCategory(Kaon))',
        12],
}

# Lists for each Step.
trackLevelParticleLists = []
eventLevelParticleLists = []
variablesCombinerLevel = []


def WhichCategories(categories=[
    'Electron',
    'Muon',
    'KinLepton',
    'Kaon',
    'SlowPion',
    'FastHadron',
    'Lambda',
    'FSC',
    'MaximumPstar',
    'KaonPion',
]):
    """
    Selection of the Categories that are going to be used.
    """

    if len(categories) > 13 or len(categories) < 2:
        B2FATAL('Flavor Tagger: Invalid amount of categories. At least two are needed. No more than 13 are available'
                )
        B2FATAL(
            'Flavor Tagger: Possible categories are  "Electron", "IntermediateElectron", "Muon", "IntermediateMuon", '
            '"KinLepton", "IntermediateKinLepton", "Kaon", "SlowPion", "FastHadron",'
            '"Lambda", "FSC", "MaximumPstar" or "KaonPion" ')
    categoriesCombination = []
    for category in categories:
        if category in AvailableCategories:
            if category != 'MaximumPstar' and (AvailableCategories[category][0],
                                               AvailableCategories[category][1]) \
                    not in trackLevelParticleLists:
                trackLevelParticleLists.append((AvailableCategories[category][0],
                                                AvailableCategories[category][1]))
            if (AvailableCategories[category][0],
                    AvailableCategories[category][2], AvailableCategories[category][3]) \
                    not in eventLevelParticleLists:
                eventLevelParticleLists.append((AvailableCategories[category][0],
                                                AvailableCategories[category][2], AvailableCategories[category][3]))
                variablesCombinerLevel.append(AvailableCategories[category][3])
                categoriesCombination.append(AvailableCategories[category][4])
            else:
                B2FATAL('Flavor Tagger: ' + category + ' has been already given')
        else:
            B2FATAL('Flavor Tagger: ' + category + ' is not a valid category name given')
            B2FATAL('Flavor Tagger: Available categories are  "Electron", "IntermediateElectron", '
                    '"Muon", "IntermediateMuon", "KinLepton", "IntermediateKinLepton", "Kaon", "SlowPion", "FastHadron", '
                    '"Lambda", "FSC", "MaximumPstar" or "KaonPion" ')
    global categoriesCombinationCode
    categoriesCombinationCode = 'CatCode'
    for code in sorted(categoriesCombination):
        categoriesCombinationCode = categoriesCombinationCode + '%02d' % code


# Variables for categories on track level - are defined in variables.cc and MetaVariables.cc
variables = dict()
KId = {'Belle': 'kIDBelle', 'Belle2': 'kaonID'}
muId = {'Belle': 'muIDBelle', 'Belle2': 'muonID'}
eId = {'Belle': 'eIDBelle', 'Belle2': 'electronID'}

flavorTaggerVariables.addAlias('eid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, CDC, SVD), 0.5)')
flavorTaggerVariables.addAlias('eid_TOP', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, TOP), 0.5)')
flavorTaggerVariables.addAlias('eid_ARICH', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, ARICH), 0.5)')
flavorTaggerVariables.addAlias('eid_ECL', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, ECL), 0.5)')

flavorTaggerVariables.addAlias('muid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, CDC, SVD), 0.5)')
flavorTaggerVariables.addAlias('muid_TOP', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, TOP), 0.5)')
flavorTaggerVariables.addAlias('muid_ARICH', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, ARICH), 0.5)')
flavorTaggerVariables.addAlias('muid_KLM', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, KLM), 0.5)')

flavorTaggerVariables.addAlias('piid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(211, 321, CDC, SVD), 0.5)')
flavorTaggerVariables.addAlias('piid_TOP', 'ifNANgiveX(pidPairProbabilityExpert(211, 321, TOP), 0.5)')
flavorTaggerVariables.addAlias('piid_ARICH', 'ifNANgiveX(pidPairProbabilityExpert(211, 321, ARICH), 0.5)')
flavorTaggerVariables.addAlias('pi_vs_edEdxid', 'ifNANgiveX(pidPairProbabilityExpert(211, 11, CDC, SVD), 0.5)')

flavorTaggerVariables.addAlias('Kid_TOP', 'ifNANgiveX(pidPairProbabilityExpert(321, 211, TOP), 0.5)')
flavorTaggerVariables.addAlias('Kid_ARICH', 'ifNANgiveX(pidPairProbabilityExpert(321, 211, ARICH), 0.5)')
flavorTaggerVariables.addAlias('Kid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(321, 211, CDC, SVD), 0.5)')


def setVariables():
    """
    Sets the Variables used for Track and Event Levels.
    """

    variables['Electron'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        'cosTheta',
        eId[getBelleOrBelle2()],
        'eid_dEdx',
        'eid_TOP',
        'eid_ARICH',
        'eid_ECL',
        'BtagToWBosonVariables(recoilMassSqrd)',
        'BtagToWBosonVariables(pMissCMS)',
        'BtagToWBosonVariables(cosThetaMissCMS)',
        'BtagToWBosonVariables(EW90)',
        'cosTPTO',
        'ImpactXY',
        'distance',
        'chiProb',
    ]
    variables['IntermediateElectron'] = variables['Electron']
    variables['Muon'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        'cosTheta',
        muId[getBelleOrBelle2()],
        'muid_dEdx',
        'muid_TOP',
        'muid_ARICH',
        'muid_KLM',
        'BtagToWBosonVariables(recoilMassSqrd)',
        'BtagToWBosonVariables(pMissCMS)',
        'BtagToWBosonVariables(cosThetaMissCMS)',
        'BtagToWBosonVariables(EW90)',
        'cosTPTO',
        'ImpactXY',
        'distance',
        'chiProb',
    ]
    variables['IntermediateMuon'] = variables['Muon']
    variables['KinLepton'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        'cosTheta',
        muId[getBelleOrBelle2()],
        'muid_dEdx',
        'muid_TOP',
        'muid_ARICH',
        'muid_KLM',
        eId[getBelleOrBelle2()],
        'eid_dEdx',
        'eid_TOP',
        'eid_ARICH',
        'eid_ECL',
        'BtagToWBosonVariables(recoilMassSqrd)',
        'BtagToWBosonVariables(pMissCMS)',
        'BtagToWBosonVariables(cosThetaMissCMS)',
        'BtagToWBosonVariables(EW90)',
        'cosTPTO',
        'ImpactXY',
        'distance',
        'chiProb',
    ]
    variables['IntermediateKinLepton'] = variables['KinLepton']
    variables['Kaon'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'cosTheta',
        'pt',
        KId[getBelleOrBelle2()],
        'Kid_dEdx',
        'Kid_TOP',
        'Kid_ARICH',
        'NumberOfKShortsInRoe',
        'ptTracksRoe',
        'BtagToWBosonVariables(recoilMassSqrd)',
        'BtagToWBosonVariables(pMissCMS)',
        'BtagToWBosonVariables(cosThetaMissCMS)',
        'BtagToWBosonVariables(EW90)',
        'cosTPTO',
        'ImpactXY',
        'distance',
        'chiProb',
    ]
    variables['SlowPion'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'cosTheta',
        'p',
        'pt',
        'pionID',
        'piid_dEdx',
        'piid_TOP',
        'piid_ARICH',
        'pi_vs_edEdxid',
        KId[getBelleOrBelle2()],
        'Kid_dEdx',
        'Kid_TOP',
        'Kid_ARICH',
        'NumberOfKShortsInRoe',
        'ptTracksRoe',
        eId[getBelleOrBelle2()],
        'BtagToWBosonVariables(recoilMassSqrd)',
        'BtagToWBosonVariables(EW90)',
        'BtagToWBosonVariables(cosThetaMissCMS)',
        'BtagToWBosonVariables(pMissCMS)',
        'cosTPTO',
        'ImpactXY',
        'distance',
        'chiProb'
    ]
    variables['FastHadron'] = variables['SlowPion']
    variables['Lambda'] = [
        'lambdaFlavor',
        'NumberOfKShortsInRoe',
        'M',
        'cosAngleBetweenMomentumAndVertexVector',
        'lambdaZError',
        'daughter(0,p)',
        'daughter(0,useCMSFrame(p))',
        'daughter(1,p)',
        'daughter(1,useCMSFrame(p))',
        'useCMSFrame(p)',
        'p',
        'distance',
        'chiProb',
    ]
    if getBelleOrBelle2() != "Belle":
        variables['Lambda'].append('daughter(1,protonID)')  # protonID always 0 in B2BII check in future
        variables['Lambda'].append('daughter(0,pionID)')  # not very powerful in B2BII

    variables['MaximumPstar'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        'cosTPTO',
        'ImpactXY',
        'distance',
    ]

    variables['FSC'] = [
        'useCMSFrame(p)',
        'cosTPTO',
        KId[getBelleOrBelle2()],
        'FSCVariables(pFastCMS)',
        'FSCVariables(cosSlowFast)',
        'FSCVariables(cosTPTOFast)',
        'FSCVariables(SlowFastHaveOpositeCharges)',
    ]

    # For sampling and teaching in a second step
    variables['KaonPion'] = ['extraInfo(isRightCategory(Kaon))',
                             'HighestProbInCat(pi+:inRoe, isRightCategory(SlowPion))',
                             'KaonPionVariables(cosKaonPion)', 'KaonPionVariables(HaveOpositeCharges)', KId[getBelleOrBelle2()]]


def FillParticleLists(mode='Expert', path=analysis_main):
    """
    Fills the particle Lists for all categories.
    """

    readyParticleLists = []

    for (particleList, category) in trackLevelParticleLists:

        if particleList in readyParticleLists:
            continue

        # Select particles in ROE for different categories according to mass hypothesis.
        if particleList != 'Lambda0:inRoe' and particleList != 'K+:inRoe' and particleList != 'pi+:inRoe':

            # Filling particle list for actual category
            fillParticleList(particleList, 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1', path=path)
            readyParticleLists.append(particleList)

        else:
            if 'pi+:inRoe' not in readyParticleLists:
                fillParticleList(
                    'pi+:inRoe', 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1', path=path)
                readyParticleLists.append('pi+:inRoe')

            if 'K_S0:inRoe' not in readyParticleLists:
                if getBelleOrBelle2() == 'Belle':
                    cutAndCopyList('K_S0:inRoe', 'K_S0:mdst', 'extraInfo(ksnbStandard) == 1 and isInRestOfEvent == 1', path=path)
                else:
                    reconstructDecay('K_S0:inRoe -> pi+:inRoe pi-:inRoe', '0.40<=M<=0.60', False, path=path)
                    fitVertex('K_S0:inRoe', 0.01, fitter='kfitter', path=path)
                readyParticleLists.append('K_S0:inRoe')

            if particleList == 'K+:inRoe':
                fillParticleList(
                    particleList, 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1', path=path)
                # Precut done to prevent from overtraining, found not necessary now
                # applyCuts(particleList, '0.1<' + KId[getBelleOrBelle2()], path=path)
                readyParticleLists.append(particleList)

            if particleList == 'Lambda0:inRoe':
                fillParticleList(
                    'p+:inRoe', 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1', path=path)
                reconstructDecay(particleList + ' -> pi-:inRoe p+:inRoe', '1.00<=M<=1.23', False, path=path)
                fitVertex(particleList, 0.01, fitter='kfitter', path=path)
                # if mode != 'Expert':
                matchMCTruth(particleList, path=path)
                readyParticleLists.append(particleList)

    return True


def eventLevel(mode='Expert', weightFiles='B2JpsiKs_mu', path=analysis_main):
    """
    Samples data for training or tests all categories all categories at event level.
    """

    B2INFO('EVENT LEVEL')

    ReadyMethods = 0

    # Each category has its own Path in order to be skipped if the corresponding particle list is empty
    identifiersExtraInfosDict = dict()
    identifiersExtraInfosKaonPion = []

    for (particleList, category, combinerVariable) in eventLevelParticleLists:

        methodPrefixEventLevel = belleOrBelle2Flag + "_" + weightFiles + 'EventLevel' + category + 'FBDT'
        identifierEventLevel = filesDirectory + '/' + methodPrefixEventLevel + '_1.root'
        targetVariable = 'isRightCategory(' + category + ')'
        extraInfoName = targetVariable

        if not os.path.isfile(identifierEventLevel) and mode == 'Expert':
            if downloadFlag:
                basf2_mva.download(methodPrefixEventLevel, identifierEventLevel)
                if not os.path.isfile(identifierEventLevel):
                    B2FATAL('Flavor Tagger: Weight file ' + identifierEventLevel +
                            ' was not downloaded from Database. Please check the buildOrRevision name. Stopped')
            else:
                B2FATAL(
                    'Flavor Tagger: ' +
                    particleList +
                    ' Eventlevel was not trained. Weight file ' +
                    identifierEventLevel + ' was not found. Stopped')

        if os.path.isfile(identifierEventLevel) and mode != 'Teacher':

            B2INFO('flavorTagger: MVAExpert ' + methodPrefixEventLevel + ' ready.')

            if mode == 'Sampler':
                if 'KaonPion' in [row[1] for row in eventLevelParticleLists]:
                    methodPrefixEventLevelKaonPion = belleOrBelle2Flag + "_" + weightFiles + 'EventLevelKaonPionFBDT'
                    identifierEventLevelKaonPion = filesDirectory + '/' + methodPrefixEventLevelKaonPion + '_1.root'
                    if not os.path.isfile(identifierEventLevelKaonPion):
                        # Slow Pion and Kaon categories are used if Kaon-Pion is lacking for
                        # sampling. The others are not needed and skipped
                        if category != "SlowPion" and category != "Kaon":
                            continue

            if particleList not in identifiersExtraInfosDict and category != 'KaonPion':
                identifiersExtraInfosDict[particleList] = [(extraInfoName, identifierEventLevel)]
            elif category != 'KaonPion':
                identifiersExtraInfosDict[particleList].append((extraInfoName, identifierEventLevel))
            else:
                identifiersExtraInfosKaonPion.append((extraInfoName, identifierEventLevel))

            ReadyMethods += 1

    # Each category has its own Path in order to be skipped if the corresponding particle list is empty
    for particleList in identifiersExtraInfosDict:
        eventLevelPath = create_path()
        SkipEmptyParticleList = register_module("SkimFilter")
        SkipEmptyParticleList.set_name('SkimFilter_EventLevel_' + particleList)
        SkipEmptyParticleList.param('particleLists', particleList)
        SkipEmptyParticleList.if_true(eventLevelPath, AfterConditionPath.CONTINUE)
        path.add_module(SkipEmptyParticleList)

        mvaMultipleExperts = register_module('MVAMultipleExperts')
        mvaMultipleExperts.set_name('MVAMultipleExperts_EventLevel_' + particleList)
        mvaMultipleExperts.param('listNames', [particleList])
        mvaMultipleExperts.param('extraInfoNames', [row[0] for row in identifiersExtraInfosDict[particleList]])
        mvaMultipleExperts.param('signalFraction', signalFraction)
        mvaMultipleExperts.param('identifiers', [row[1] for row in identifiersExtraInfosDict[particleList]])
        eventLevelPath.add_module(mvaMultipleExperts)

    if 'KaonPion' in [row[1] for row in eventLevelParticleLists] and len(identifiersExtraInfosKaonPion) != 0:
        eventLevelKaonPionPath = create_path()
        SkipEmptyParticleList = register_module("SkimFilter")
        SkipEmptyParticleList.set_name('SkimFilter_' + 'K+:inRoe')
        SkipEmptyParticleList.param('particleLists', 'K+:inRoe')
        SkipEmptyParticleList.if_true(eventLevelKaonPionPath, AfterConditionPath.CONTINUE)
        path.add_module(SkipEmptyParticleList)

        mvaExpertKaonPion = register_module("MVAExpert")
        mvaExpertKaonPion.set_name('MVAExpert_KaonPion_' + 'K+:inRoe')
        mvaExpertKaonPion.param('listNames', ['K+:inRoe'])
        mvaExpertKaonPion.param('extraInfoName', identifiersExtraInfosKaonPion[0][0])
        mvaExpertKaonPion.param('signalFraction', signalFraction)
        mvaExpertKaonPion.param('identifier', identifiersExtraInfosKaonPion[0][1])

        eventLevelKaonPionPath.add_module(mvaExpertKaonPion)

    for (particleList, category, combinerVariable) in eventLevelParticleLists:

        methodPrefixEventLevel = belleOrBelle2Flag + "_" + weightFiles + 'EventLevel' + category + 'FBDT'
        identifierEventLevel = filesDirectory + '/' + methodPrefixEventLevel + '_1.root'
        targetVariable = 'isRightCategory(' + category + ')'

        if not os.path.isfile(identifierEventLevel) and mode == 'Sampler':

            if category == 'KaonPion':
                methodPrefixEventLevelSlowPion = belleOrBelle2Flag + "_" + weightFiles + 'EventLevelSlowPionFBDT'
                identifierEventLevelSlowPion = filesDirectory + '/' + methodPrefixEventLevelSlowPion + '_1.root'
                if not os.path.isfile(identifierEventLevelSlowPion):
                    B2INFO("Flavor Tagger: event level weight file for the Slow Pion category is absent." +
                           "It is required to sample the training information for the KaonPion category." +
                           "An additional sampling step will be needed after the following training step.")
                    continue

            B2INFO(
                'flavorTagger: file ' + filesDirectory + '/' +
                methodPrefixEventLevel + "sampled" + fileId + '.root will be saved.')

            applyCuts(particleList, 'isRightCategory(mcAssociated) > 0', path)
            eventLevelpath = create_path()
            SkipEmptyParticleList = register_module("SkimFilter")
            SkipEmptyParticleList.set_name('SkimFilter_EventLevel' + category)
            SkipEmptyParticleList.param('particleLists', particleList)
            SkipEmptyParticleList.if_true(eventLevelpath, AfterConditionPath.CONTINUE)
            path.add_module(SkipEmptyParticleList)

            ntuple = register_module('VariablesToNtuple')
            ntuple.param('fileName', filesDirectory + '/' + methodPrefixEventLevel + "sampled" + fileId + ".root")
            ntuple.param('treeName', methodPrefixEventLevel + "_tree")
            variablesToBeSaved = variables[category] + [targetVariable, 'ancestorHasWhichFlavor',
                                                        'isSignal', 'mcPDG', 'mcErrors', 'genMotherPDG',
                                                        'nMCMatches', 'B0mcErrors']
            if category != 'KaonPion' and category != 'FSC':
                variablesToBeSaved = variablesToBeSaved + \
                    ['extraInfo(isRightTrack(' + category + '))',
                     'hasHighestProbInCat(' + particleList + ', isRightTrack(' + category + '))']
            ntuple.param('variables', variablesToBeSaved)
            ntuple.param('particleList', particleList)
            eventLevelpath.add_module(ntuple)

    if ReadyMethods != len(eventLevelParticleLists):
        return False
    else:
        return True


def eventLevelTeacher(weightFiles='B2JpsiKs_mu'):
    """
    Trains all categories at event level.
    """

    B2INFO('EVENT LEVEL TEACHER')

    ReadyMethods = 0

    for (particleList, category, combinerVariable) in eventLevelParticleLists:

        methodPrefixEventLevel = belleOrBelle2Flag + "_" + weightFiles + 'EventLevel' + category + 'FBDT'
        targetVariable = 'isRightCategory(' + category + ')'
        weightFile = filesDirectory + '/' + methodPrefixEventLevel + "_1.root"

        if not os.path.isfile(weightFile):

            sampledFilesList = glob.glob(filesDirectory + '/' + methodPrefixEventLevel + 'sampled*.root')
            if len(sampledFilesList) == 0:

                B2INFO('flavorTagger: eventLevelTeacher did not find any ' + methodPrefixEventLevel +
                       ".root" + ' file. Please run the flavorTagger in "Sampler" mode afterwards.')

            else:

                B2INFO('flavorTagger: MVA Teacher training' + methodPrefixEventLevel + ' .')
                trainingOptionsEventLevel = basf2_mva.GeneralOptions()
                trainingOptionsEventLevel.m_datafiles = basf2_mva.vector(*sampledFilesList)
                trainingOptionsEventLevel.m_treename = methodPrefixEventLevel + "_tree"
                trainingOptionsEventLevel.m_identifier = weightFile
                trainingOptionsEventLevel.m_variables = basf2_mva.vector(*variables[category])
                trainingOptionsEventLevel.m_target_variable = targetVariable
                trainingOptionsEventLevel.m_max_events = maxEventsNumber

                basf2_mva.teacher(trainingOptionsEventLevel, fastBDTCategories)

                if uploadFlag:
                    basf2_mva.upload(weightFile, methodPrefixEventLevel)

        else:

            ReadyMethods += 1

    if ReadyMethods != len(eventLevelParticleLists):
        return False
    else:
        return True


def combinerLevel(mode='Expert', weightFiles='B2JpsiKs_mu', path=analysis_main):
    """
    Samples the input data or tests the combiner according to the selected categories.
    """

    B2INFO('COMBINER LEVEL')

    B2INFO("Flavor Tagger: Required Combiner for Categories:")
    for (particleList, category, combinerVariable) in eventLevelParticleLists:
        B2INFO(category)

    B2INFO("Flavor Tagger: which corresponds to a weight file with categories combination code " + categoriesCombinationCode)

    methodPrefixCombinerLevel = belleOrBelle2Flag + "_" + weightFiles + 'Combiner' \
        + categoriesCombinationCode

    if mode == 'Sampler':

        if not (
            os.path.isfile(
                filesDirectory + '/' + methodPrefixCombinerLevel + 'FBDT' + '_1.root') or os.path.isfile(
                filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + '_1.root')):
            B2INFO('flavorTagger: Sampling Data on Combiner Level. File' +
                   methodPrefixCombinerLevel + ".root" + ' will be saved')

            ntuple = register_module('VariablesToNtuple')
            ntuple.param('fileName', filesDirectory + '/' + methodPrefixCombinerLevel + "sampled" + fileId + ".root")
            ntuple.param('treeName', methodPrefixCombinerLevel + 'FBDT' + "_tree")
            ntuple.param('variables', variablesCombinerLevel + ['qrCombined'])
            ntuple.param('particleList', "")
            path.add_module(ntuple)

            return False

        else:
            B2FATAL('flavorTagger: File' + methodPrefixCombinerLevel + 'FBDT' + "_1.root" + ' or ' + methodPrefixCombinerLevel +
                    'FANN' + '_1.root found. Please run the "Expert" mode or delete the file if a new sampling is desired.')

    if mode == 'Expert':

        if TMVAfbdt and not FANNmlp:

            if not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FBDT' + '_1.root') and downloadFlag:
                basf2_mva.download(methodPrefixCombinerLevel + 'FBDT', filesDirectory +
                                   '/' + methodPrefixCombinerLevel + 'FBDT' + '_1.root')
                if not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FBDT' + '_1.root'):
                    B2FATAL('Flavor Tagger: Weight file ' + methodPrefixCombinerLevel + 'FBDT' +
                            '_1.root was not downloaded from Database. Please check the buildOrRevision name. Stopped')

            if not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FBDT' + '_1.root'):

                B2FATAL('flavorTagger: Combinerlevel FastBDT was not trained with this combination of categories. Weight file ' +
                        methodPrefixCombinerLevel + 'FBDT' + '_1.root not found. Stopped')
            else:
                B2INFO('flavorTagger: Ready to be used with weightFile ' + methodPrefixCombinerLevel + 'FBDT' + '_1.root')
                B2INFO('flavorTagger: Apply FBDTMethod ' + methodPrefixCombinerLevel + 'FBDT')
                path.add_module('MVAExpert', listNames=[], extraInfoName='qrCombined' + 'FBDT', signalFraction=signalFraction,
                                identifier=filesDirectory + '/' + methodPrefixCombinerLevel + 'FBDT' + "_1.root")
                return True

        if FANNmlp and not TMVAfbdt:

            if not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + '_1.root') and downloadFlag:
                basf2_mva.download(methodPrefixCombinerLevel + 'FANN', filesDirectory +
                                   '/' + methodPrefixCombinerLevel + 'FANN' + '_1.root')
                if not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + '_1.root'):
                    B2FATAL('Flavor Tagger: Weight file ' + methodPrefixCombinerLevel + 'FANN' +
                            '_1.root was not downloaded from Database. Please check the buildOrRevision name. Stopped')

            if not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + '_1.root'):

                B2FATAL('flavorTagger: Combinerlevel FANN was not trained with this combination of categories. Weight file ' +
                        methodPrefixCombinerLevel + 'FANN' + '_1.root not found. Stopped')

            else:
                B2INFO('flavorTagger: Ready to be used with weightFile ' + methodPrefixCombinerLevel + 'FANN' + '_1.root')

                B2INFO('flavorTagger: Apply FANNMethod on combiner level')
                path.add_module('MVAExpert', listNames=[], extraInfoName='qrCombined' + 'FANN', signalFraction=signalFraction,
                                identifier=filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + "_1.root")
                return True

        if FANNmlp and TMVAfbdt:

            if not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FBDT' + '_1.root') and downloadFlag:
                basf2_mva.download(methodPrefixCombinerLevel + 'FBDT', filesDirectory +
                                   '/' + methodPrefixCombinerLevel + 'FBDT' + '_1.root')
                if not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FBDT' + '_1.root'):
                    B2FATAL('Flavor Tagger: Weight file ' + methodPrefixCombinerLevel + 'FBDT' +
                            '_1.root was not downloaded from Database. Please check the buildOrRevision name. Stopped')

            if not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + '_1.root') and downloadFlag:
                basf2_mva.download(methodPrefixCombinerLevel + 'FANN', filesDirectory +
                                   '/' + methodPrefixCombinerLevel + 'FANN' + '_1.root')
                if not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + '_1.root'):
                    B2FATAL('Flavor Tagger: Weight file ' + methodPrefixCombinerLevel + 'FANN' +
                            '_1.root was not downloaded from Database. Please check the buildOrRevision name. Stopped')

            if not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FBDT' + '_1.root'):

                B2FATAL('flavorTagger: Combinerlevel FastBDT was not trained with this combination of categories. Weight file ' +
                        methodPrefixCombinerLevel + 'FBDT' + '_1.root not found. Stopped')

            elif not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + '_1.root'):

                B2FATAL('flavorTagger: Combinerlevel FANN was not trained with this combination of categories. Weight file ' +
                        methodPrefixCombinerLevel + 'FANN' + '_1.root not found. Stopped')

            else:
                B2INFO('flavorTagger: Ready to be used with weightFiles ' + methodPrefixCombinerLevel + 'FBDT' + '_1.root and ' +
                       methodPrefixCombinerLevel + 'FANN' + '_1.root')

                B2INFO('flavorTagger: Apply FANNMethod on combiner level')

                mvaMultipleExperts = register_module('MVAMultipleExperts')
                mvaMultipleExperts.set_name('MVAMultipleExperts_Combiners')
                mvaMultipleExperts.param('listNames', [])
                mvaMultipleExperts.param('extraInfoNames', ['qrCombined' + 'FBDT', 'qrCombined' + 'FANN'])
                mvaMultipleExperts.param('signalFraction', signalFraction)
                mvaMultipleExperts.param('identifiers', [filesDirectory + '/' + methodPrefixCombinerLevel + 'FBDT' + "_1.root",
                                                         filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + "_1.root"])
                path.add_module(mvaMultipleExperts)

                return True


def combinerLevelTeacher(weightFiles='B2JpsiKs_mu'):
    """
    Trains the combiner according to the selected categories.
    """

    B2INFO('COMBINER LEVEL TEACHER')

    methodPrefixCombinerLevel = belleOrBelle2Flag + "_" + weightFiles + 'Combiner' \
        + categoriesCombinationCode

    sampledFilesList = glob.glob(filesDirectory + '/' + methodPrefixCombinerLevel + 'sampled*.root')
    if len(sampledFilesList) == 0:
        B2FATAL('FlavorTagger: combinerLevelTeacher did not find any ' +
                methodPrefixCombinerLevel + 'sampled*.root file. Please run the flavorTagger in "Sampler" mode.')

    ReadyTMVAfbdt = False
    ReadyFANNmlp = False

    if TMVAfbdt:

        if not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FBDT' + '_1.root'):

            B2INFO('flavorTagger: MVA Teacher training a FastBDT on Combiner Level')

            trainingOptionsCombinerLevel = basf2_mva.GeneralOptions()
            trainingOptionsCombinerLevel.m_datafiles = basf2_mva.vector(*sampledFilesList)
            trainingOptionsCombinerLevel.m_treename = methodPrefixCombinerLevel + 'FBDT' + "_tree"
            trainingOptionsCombinerLevel.m_identifier = filesDirectory + '/' + methodPrefixCombinerLevel + 'FBDT' + "_1.root"
            trainingOptionsCombinerLevel.m_variables = basf2_mva.vector(*variablesCombinerLevel)
            trainingOptionsCombinerLevel.m_target_variable = 'qrCombined'
            trainingOptionsCombinerLevel.m_max_events = maxEventsNumber

            basf2_mva.teacher(trainingOptionsCombinerLevel, fastBDTCombiner)

            if uploadFlag:
                basf2_mva.upload(filesDirectory + '/' + methodPrefixCombinerLevel +
                                 'FBDT' + "_1.root", methodPrefixCombinerLevel + 'FBDT')

        elif FANNmlp and not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + '_1.root'):

            B2INFO('flavorTagger: Combinerlevel FBDT was already trained with this combination of categories. Weight file ' +
                   methodPrefixCombinerLevel + 'FBDT' + '_1.root has been found.')

        else:
            B2FATAL('flavorTagger: Combinerlevel was already trained with this combination of categories. Weight files ' +
                    methodPrefixCombinerLevel + 'FBDT' + '_1.root and ' +
                    methodPrefixCombinerLevel + 'FANN' + '_1.root has been found. Please use the "Expert" mode')

    if FANNmlp:

        if not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + '_1.root'):

            B2INFO('flavorTagger: MVA Teacher training a FANN MLP on Combiner Level')

            trainingOptionsCombinerLevel = basf2_mva.GeneralOptions()
            trainingOptionsCombinerLevel.m_datafiles = basf2_mva.vector(*sampledFilesList)
            trainingOptionsCombinerLevel.m_treename = methodPrefixCombinerLevel + 'FBDT' + "_tree"
            trainingOptionsCombinerLevel.m_identifier = filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + "_1.root"
            trainingOptionsCombinerLevel.m_variables = basf2_mva.vector(*variablesCombinerLevel)
            trainingOptionsCombinerLevel.m_target_variable = 'qrCombined'
            trainingOptionsCombinerLevel.m_max_events = maxEventsNumber

            basf2_mva.teacher(trainingOptionsCombinerLevel, mlpFANNCombiner)

            if uploadFlag:
                basf2_mva.upload(filesDirectory + '/' + methodPrefixCombinerLevel +
                                 'FANN' + "_1.root", methodPrefixCombinerLevel + 'FANN')

        elif TMVAfbdt and not os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FBDT' + '_1.root'):

            B2INFO('flavorTagger: Combinerlevel FBDT was already trained with this combination of categories. Weight file ' +
                   methodPrefixCombinerLevel + 'FANN' + '_1.config has been found.')

        else:
            B2FATAL('flavorTagger: Combinerlevel was already trained with this combination of categories. Weight files ' +
                    methodPrefixCombinerLevel + 'FBDT' + '_1.root and ' +
                    methodPrefixCombinerLevel + 'FANN' + '_1.root has been found. Please use the "Expert" mode')


def flavorTagger(
    particleLists=[],
    mode='Expert',
    weightFiles='B2JpsiKs_muBGx1',
    workingDirectory='.',
    combinerMethods=['TMVA-FBDT', 'FANN-MLP'],
    categories=[
        'Electron',
        'IntermediateElectron',
        'Muon',
        'IntermediateMuon',
        'KinLepton',
        'IntermediateKinLepton',
        'Kaon',
        'SlowPion',
        'FastHadron',
        'Lambda',
        'FSC',
        'MaximumPstar',
        'KaonPion'],
    belleOrBelle2="Belle2",
    saveCategoriesInfo=True,
    downloadFromDatabaseIfNotfound=True,
    uploadToDatabaseAfterTraining=False,
    samplerFileId='',
    path=analysis_main,
):
    """
      Defines the whole flavor tagging process for each selected Rest of Event (ROE) built in the steering file.
      The flavor is predicted by Multivariate Methods trained with Variables and MetaVariables which use
      Tracks, ECL- and KLMClusters from the corresponding RestOfEvent dataobject.
      This module can be used to sample the training information, to train and/or to test the flavorTagger.

      @param particleLists                     The ROEs for flavor tagging are selected from the given particle lists.
      @param mode                              The available modes are "Sampler", "Teacher" or "Expert".
      @param weightFiles                       Weight files name. Default= "B2JpsiKs_muBGx1". Use 'B2JpsiKs_muBGx0' with BGx0 MC.
      @param workingDirectory                  Path to the directory containing the FlavorTagging/ folder.
      @param combinerMethods                   MVAs for the combiner: 'TMVA-FBDT' or 'FANN-MLP'. Both used by default.
      @param categories                        Categories used for flavor tagging. By default all are used.
      @param belleOrBelle2                     Uses files trained for "Belle" or "Belle2" MC.
      @param saveCategoriesInfo                Sets to save information of individual categories.
      @param downloadFromDatabaseIfNotfound    Weight files are downloaded from the conditions database if not in workingDirectory.
      @param uploadToDatabaseAfterTraining     For librarians: uploads weight files to localdb after training.
      @param samplerFileId                     Identifier to paralellize sampling. Only used in "Sampler" mode.
      @param path                              Modules are added to this path

    """

    if mode != 'Sampler' and mode != 'Teacher' and mode != 'Expert':
        B2FATAL('flavorTagger: Wrong mode given: The available modes are "Sampler", "Teacher" or "Expert"')

    # Directory where the weights of the trained Methods are saved
    # workingDirectory = os.environ['BELLE2_LOCAL_DIR'] + '/analysis/data'

    if not Belle2.FileSystem.findFile(workingDirectory, True):
        B2FATAL('flavorTagger: THE GIVEN WORKING DIRECTORY "' + workingDirectory + '" DOES NOT EXIST! PLEASE SPECIFY A VALID PATH.')

    if mode == 'Sampler' or (mode == 'Expert' and downloadFromDatabaseIfNotfound):
        if not Belle2.FileSystem.findFile(workingDirectory + '/FlavorTagging', True):
            os.mkdir(workingDirectory + '/FlavorTagging')
            os.mkdir(workingDirectory + '/FlavorTagging/TrainedMethods')
        elif not Belle2.FileSystem.findFile(workingDirectory + '/FlavorTagging/TrainedMethods', True):
            os.mkdir(workingDirectory + '/FlavorTagging/TrainedMethods')

    global filesDirectory

    filesDirectory = workingDirectory + '/FlavorTagging/TrainedMethods'

    if not (belleOrBelle2 == 'Belle2' or belleOrBelle2 == 'Belle'):
        B2FATAL('flavorTagger: Wrong argument for belleOrBelle2 given: The available modes are "Belle2" or "Belle"')

    if len(combinerMethods) < 1 or len(combinerMethods) > 2:
        B2FATAL('flavorTagger: Invalid list of combinerMethods. The available methods are "TMVA-FBDT" and "FANN-MLP"')

    global FANNmlp
    global TMVAfbdt

    FANNmlp = False
    TMVAfbdt = False

    for method in combinerMethods:
        if method == 'TMVA-FBDT':
            TMVAfbdt = True
        elif method == 'FANN-MLP':
            FANNmlp = True
        else:
            B2FATAL('flavorTagger: Invalid list of combinerMethods. The available methods are "TMVA-FBDT" and "FANN-MLP"')

    global fileId

    fileId = samplerFileId

    B2INFO('*** FLAVOR TAGGING ***')
    B2INFO(' ')
    B2INFO('    Working directory is: ' + filesDirectory)
    B2INFO(' ')

    setBelleOrBelle2(belleOrBelle2)
    setInteractionWithDatabase(downloadFromDatabaseIfNotfound, uploadToDatabaseAfterTraining)
    WhichCategories(categories)
    setVariables()

    roe_path = create_path()
    deadEndPath = create_path()

    # Events containing ROE without B-Meson (but not empty) are discarded for training
    if mode == 'Sampler':
        signalSideParticleListsFilter(particleLists, 'hasRestOfEventTracks > 0 and abs(qrCombined) == 1', roe_path, deadEndPath)

    # If trigger returns 1 jump into empty path skipping further modules in roe_path
    if mode == 'Expert':
        signalSideParticleListsFilter(particleLists, 'hasRestOfEventTracks > 0', roe_path, deadEndPath)
        # Initialation of flavorTaggerInfo dataObject needs to be done in the main path
        flavorTaggerInfoBuilder = register_module('FlavorTaggerInfoBuilder')
        path.add_module(flavorTaggerInfoBuilder)

    # sampler or expert
    if mode == 'Sampler' or mode == 'Expert':
        if FillParticleLists(mode, roe_path):
            if eventLevel(mode, weightFiles, roe_path):
                combinerLevel(mode, weightFiles, roe_path)
                if mode == 'Expert':
                    flavorTaggerInfoFiller = register_module('FlavorTaggerInfoFiller')
                    flavorTaggerInfoFiller.param('trackLevelParticleLists', trackLevelParticleLists)
                    flavorTaggerInfoFiller.param('eventLevelParticleLists', eventLevelParticleLists)
                    flavorTaggerInfoFiller.param('TMVAfbdt', TMVAfbdt)
                    flavorTaggerInfoFiller.param('FANNmlp', FANNmlp)
                    flavorTaggerInfoFiller.param('qpCategories', saveCategoriesInfo)
                    flavorTaggerInfoFiller.param('istrueCategories', saveCategoriesInfo)
                    flavorTaggerInfoFiller.param('targetProb', False)
                    flavorTaggerInfoFiller.param('trackPointers', False)
                    roe_path.add_module(flavorTaggerInfoFiller)  # Add FlavorTag Info filler to roe_path

    # Removes EventExtraInfos and ParticleExtraInfos of the EventParticleLists
    particleListsToRemoveExtraInfo = []
    for particleList in eventLevelParticleLists:
        if particleList[0] not in particleListsToRemoveExtraInfo:
            particleListsToRemoveExtraInfo.append(particleList[0])

    if mode == 'Expert':
        removeExtraInfo(particleListsToRemoveExtraInfo, True, roe_path)
    elif mode == 'Sampler':
        removeExtraInfo(particleListsToRemoveExtraInfo, False, roe_path)

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

    if mode == 'Teacher':
        if eventLevelTeacher(weightFiles):
            combinerLevelTeacher(weightFiles)


if __name__ == '__main__':

    desc_list = []

    function = globals()["flavorTagger"]
    signature = inspect.formatargspec(*inspect.getfullargspec(function))
    signature = signature.replace(repr(analysis_main), 'analysis_main')
    desc_list.append((function.__name__, signature + '\n' + function.__doc__))

    from pager import Pager
    with Pager('Flavor Tagger function accepts the following arguments:'):
        pretty_print_description_list(desc_list)
