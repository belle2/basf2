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
import variables as mc_variables
from ROOT import Belle2
import os
import glob


def setBelleOrBelle2AndRevision(belleOrBelle2='Belle2', buildOrRevision='notDefined'):
    """
    Sets belleOrBelle2Flag and the Revision of weight files according to the specified arguments.
    """

    global belleOrBelle2Flag
    global buildOrRevisionFlag

    belleOrBelle2Flag = belleOrBelle2

    if buildOrRevision == 'notDefined':
        releaseFile = open(os.environ['BELLE2_LOCAL_DIR'] + '/.release', 'r')
        buildOrRevision = releaseFile.read().splitlines()[0]
        releaseFile.close()

    buildOrRevisionFlag = "_" + buildOrRevision + "_"


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
mlpFANNCombiner.m_scale_target = True

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
        'QrOf(e+:inRoe, isRightCategory(Electron), isRightTrack(Electron))',
        0],
    'IntermediateElectron': [
        'e+:inRoe',
        'IntermediateElectron',
        'IntermediateElectron',
        'QrOf(e+:inRoe, isRightCategory(IntermediateElectron), isRightTrack(IntermediateElectron))',
        1],
    'Muon': [
        'mu+:inRoe',
        'Muon',
        'Muon',
        'QrOf(mu+:inRoe, isRightCategory(Muon), isRightTrack(Muon))',
        2],
    'IntermediateMuon': [
        'mu+:inRoe',
        'IntermediateMuon',
        'IntermediateMuon',
        'QrOf(mu+:inRoe, isRightCategory(IntermediateMuon), isRightTrack(IntermediateMuon))',
        3],
    'KinLepton': [
        'mu+:inRoe',
        'KinLepton',
        'KinLepton',
        'QrOf(mu+:inRoe, isRightCategory(KinLepton), isRightTrack(KinLepton))',
        4],
    'IntermediateKinLepton': [
        'mu+:inRoe',
        'IntermediateKinLepton',
        'IntermediateKinLepton',
        'QrOf(mu+:inRoe, isRightCategory(IntermediateKinLepton), isRightTrack(IntermediateKinLepton))',
        5],
    'Kaon': [
        'K+:inRoe',
        'Kaon',
        'Kaon',
        'weightedQrOf(K+:inRoe, isRightCategory(Kaon), isRightTrack(Kaon))',
        6],
    'SlowPion': [
        'pi+:inRoe',
        'SlowPion',
        'SlowPion',
        'QrOf(pi+:inRoe, isRightCategory(SlowPion), isRightTrack(SlowPion))',
        7],
    'FastPion': [
        'pi+:inRoe',
        'FastPion',
        'FastPion',
        'QrOf(pi+:inRoe, isRightCategory(FastPion), isRightTrack(FastPion))',
        8],
    'Lambda': [
        'Lambda0:inRoe',
        'Lambda',
        'Lambda',
        'weightedQrOf(Lambda0:inRoe, isRightCategory(Lambda), isRightTrack(Lambda))',
        9],
    'FSC': [
        'pi+:inRoe',
        'SlowPion',
        'FSC',
        'QrOf(pi+:inRoe, isRightCategory(FSC), isRightTrack(SlowPion))',
        10],
    'MaximumPstar': [
        'pi+:inRoe',
        'MaximumPstar',
        'MaximumPstar',
        'QrOf(pi+:inRoe, isRightCategory(MaximumPstar), isRightTrack(MaximumPstar))',
        11],
    'KaonPion': [
        'K+:inRoe',
        'Kaon',
        'KaonPion',
        'QrOf(K+:inRoe, isRightCategory(KaonPion), isRightTrack(Kaon))',
        12],
}

# Lists for each Step.
trackLevelParticleLists = []
eventLevelParticleLists = []
variablesCombinerLevel = []
categoriesCombinationCode = 'CatCode'


def WhichCategories(categories=[
    'Electron',
    'Muon',
    'KinLepton',
    'Kaon',
    'SlowPion',
    'FastPion',
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
            '"KinLepton", "IntermediateKinLepton", "Kaon", "SlowPion", "FastPion", "Lambda", "FSC", "MaximumPstar" or "KaonPion" ')
    categoriesCombination = []
    for category in categories:
        if category in AvailableCategories:
            if category != 'MaximumPstar' and (AvailableCategories[category][0],
                                               AvailableCategories[category][1]) \
                    not in trackLevelParticleLists:
                trackLevelParticleLists.append((AvailableCategories[category][0],
                                                AvailableCategories[category][1]))
            if (AvailableCategories[category][0],
                    AvailableCategories[category][2]) \
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
                    '"Muon", "IntermediateMuon", "KinLepton", "IntermediateKinLepton", "Kaon", "SlowPion", "FastPion", '
                    '"Lambda", "FSC", "MaximumPstar" or "KaonPion" ')
    global categoriesCombinationCode
    for code in sorted(categoriesCombination):
        categoriesCombinationCode = categoriesCombinationCode + '%02d' % code


# Variables for categories on track level - are defined in variables.cc and MetaVariables.cc
variables = dict()
KId = {'Belle': 'Kid_belle', 'Belle2': 'Kid'}
muId = {'Belle': 'muIDBelle', 'Belle2': 'muid'}


def setVariables():
    """
    Sets the Variables used for Track and Event Levels.
    """

    variables['Electron'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        'eid',
        'eid_dEdx',
        'eid_TOP',
        'SemiLeptonicVariables(recoilMass)',
        'SemiLeptonicVariables(pMissCMS)',
        'SemiLeptonicVariables(cosThetaMissCMS)',
        'SemiLeptonicVariables(EW90)',
        'eid_ARICH',
        'eid_ECL',
        'chiProb',
    ]
    variables['IntermediateElectron'] = variables['Electron']
    variables['Muon'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        muId[getBelleOrBelle2()],
        'muid_dEdx',
        'muid_TOP',
        'SemiLeptonicVariables(recoilMass)',
        'SemiLeptonicVariables(pMissCMS)',
        'SemiLeptonicVariables(cosThetaMissCMS)',
        'SemiLeptonicVariables(EW90)',
        'muid_ARICH',
        'chiProb',
    ]
    variables['IntermediateMuon'] = variables['Muon']
    variables['KinLepton'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        muId[getBelleOrBelle2()],
        'muid_dEdx',
        'muid_TOP',
        'SemiLeptonicVariables(recoilMass)',
        'SemiLeptonicVariables(pMissCMS)',
        'SemiLeptonicVariables(cosThetaMissCMS)',
        'SemiLeptonicVariables(EW90)',
        'muid_ARICH',
        'eid',
        'eid_dEdx',
        'eid_TOP',
        'eid_ARICH',
        'eid_ECL',
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
        'distance',
        'chiProb',
    ]
    variables['SlowPion'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'cosTheta',
        'p',
        'pt',
        'piid',
        'piid_dEdx',
        'piid_TOP',
        'piid_ARICH',
        'pi_vs_edEdxid',
        'cosTPTO',
        KId[getBelleOrBelle2()],
        'eid',
        'chiProb',
    ]
    variables['FastPion'] = variables['SlowPion']
    variables['Lambda'] = [
        'lambdaFlavor',
        'NumberOfKShortsInRoe',
        'M',
        'cosAngleBetweenMomentumAndVertexVector',
        'lambdaZError',
        'daughter(1,p)',
        'daughter(1,useCMSFrame(p))',
        'useCMSFrame(p)',
        'p',
        'chiProb',
        'distance',
    ]

    # Only for Event Level
    variables['KaonPion'] = ['extraInfo(isRightTrack(Kaon))',
                             'HighestProbInCat(pi+:inRoe, isRightTrack(SlowPion))',
                             'KaonPionVariables(cosKaonPion)', 'KaonPionVariables(HaveOpositeCharges)', KId[getBelleOrBelle2()]]

    variables['MaximumPstar'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        'cosTPTO',
        'ImpactXY',
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


def FillParticleLists(mode='Expert', path=analysis_main):
    """
    Fills the particle Lists for all categories.
    """

    readyParticleLists = []

    for (particleList, category) in trackLevelParticleLists:

        if particleList in readyParticleLists:
            continue

        # Select particles in ROE for different categories according to mass hypothesis.
        if particleList != ('Lambda0:inRoe' or 'K+:inRoe'):

            # Filling particle list for actual category
            fillParticleList(particleList, 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1', path=path)
            readyParticleLists.append(particleList)

        else:
            if 'pi+:inRoe' not in readyParticleLists:
                fillParticleList('pi+:inRoe', 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1', path=path)
                readyParticleLists.append('pi+:inRoe')

            if 'K_S0:inRoe' not in readyParticleLists:
                reconstructDecay('K_S0:inRoe -> pi+:inRoe pi-:inRoe', '0.40<=M<=0.60', True, path=path)
                fitVertex('K_S0:inRoe', 0.01, fitter='kfitter', path=path)
                readyParticleLists.append('K_S0:inRoe')

            if particleList == 'K+:inRoe':
                fillParticleList(particleList, 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1', path=path)
                # Precut done to prevent from overtraining, might be redundant
                applyCuts(particleList, '0.1<' + KId[getBelleOrBelle2()], path=path)
                readyParticleLists.append(particleList)

            if particleList == 'Lambda0:inRoe':
                fillParticleList('p+:inRoe', 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1', path=path)
                reconstructDecay(particleList + ' -> pi-:inRoe p+:inRoe',
                                 '1.00<=M<=1.23', True, path=path)
                fitVertex(particleList, 0.01, fitter='kfitter', path=path)
                # if mode != 'Expert':
                matchMCTruth(particleList, path=path)
                readyParticleLists.append(particleList)

    return True


def trackLevel(mode='Expert', weightFiles='B2JpsiKs_mu', path=analysis_main):
    """
    Samples data for training or tests all categories at track level except
    KaonPion, MaximumPstar and FSC which are only at the event level.
    """

    B2INFO('TRACK LEVEL')

    ReadyMethods = 0

    identifiersExtrainfosDict = dict()

    for (particleList, category) in trackLevelParticleLists:

        methodPrefixTrackLevel = belleOrBelle2Flag + buildOrRevisionFlag + weightFiles + 'TrackLevel' + category + 'FBDT'
        identifierTrackLevel = filesDirectory + '/' + methodPrefixTrackLevel + "_1.root"
        targetVariable = 'isRightTrack(' + category + ')'
        extraInfoName = targetVariable

        if not os.path.isfile(identifierTrackLevel):
            if mode == 'Expert':
                if downloadFlag:
                    basf2_mva.download(methodPrefixTrackLevel, identifierTrackLevel)
                    if not os.path.isfile(identifierTrackLevel):
                        B2FATAL('Flavor Tagger: Weight file ' + identifierTrackLevel +
                                ' was not downloaded from Database. Please check the buildOrRevision name. Stopped')
                else:
                    B2FATAL(
                        'Flavor Tagger: ' +
                        particleList +
                        ' Tracklevel was not trained. Weight file ' +
                        identifierTrackLevel + ' was not found. Stopped')

            if mode == 'Sampler':

                B2INFO(
                    'flavorTagger: file ' +
                    filesDirectory +
                    '/' +
                    methodPrefixTrackLevel +
                    'sampled' +
                    fileId +
                    '.root will be saved.')

                trackLevelPath = create_path()
                SkipEmptyParticleList = register_module("SkimFilter")
                SkipEmptyParticleList.param('particleLists', particleList)
                SkipEmptyParticleList.if_true(trackLevelPath, AfterConditionPath.CONTINUE)
                path.add_module(SkipEmptyParticleList)

                ntuple = register_module('VariablesToNtuple')
                ntuple.param('fileName', filesDirectory + '/' + methodPrefixTrackLevel + "sampled" + fileId + ".root")
                ntuple.param('treeName', methodPrefixTrackLevel + "_tree")
                ntuple.param('variables', variables[category] + [targetVariable])
                ntuple.param('particleList', particleList)
                trackLevelPath.add_module(ntuple)

        if os.path.isfile(identifierTrackLevel) and mode != 'Teacher':

            B2INFO('flavorTagger: MVAExpert ' + methodPrefixTrackLevel + ' ready.')

            if particleList not in identifiersExtrainfosDict:
                identifiersExtrainfosDict[particleList] = [(extraInfoName, identifierTrackLevel)]
            else:
                identifiersExtrainfosDict[particleList].append((extraInfoName, identifierTrackLevel))

            ReadyMethods += 1

    if ReadyMethods != len(trackLevelParticleLists):
        return False
    else:
        # Each category has its own Path in order to be skipped if the corresponding particle list is empty
        for particleList in identifiersExtrainfosDict:
            trackLevelPath = create_path()
            SkipEmptyParticleList = register_module("SkimFilter")
            SkipEmptyParticleList.param('particleLists', particleList)
            SkipEmptyParticleList.if_true(trackLevelPath, AfterConditionPath.CONTINUE)
            path.add_module(SkipEmptyParticleList)

            trackLevelPath.add_module(
                'MVAMultipleExperts',
                listNames=[particleList],
                extraInfoNames=[row[0] for row in identifiersExtrainfosDict[particleList]],
                signalFraction=signalFraction,
                identifiers=[row[1] for row in identifiersExtrainfosDict[particleList]])
        return True


def trackLevelTeacher(weightFiles='B2JpsiKs_mu'):
    """
    Trains all categories at track level except KaonPion, MaximumPstar and FSC which are only at the event level.
    """

    ReadyMethods = 0

    for (particleList, category) in trackLevelParticleLists:

        methodPrefixTrackLevel = belleOrBelle2Flag + buildOrRevisionFlag + weightFiles + 'TrackLevel' + category + 'FBDT'
        targetVariable = 'isRightTrack(' + category + ')'
        weightFile = filesDirectory + '/' + methodPrefixTrackLevel + "_1.root"

        if not os.path.isfile(weightFile):

            sampledFilesList = glob.glob(filesDirectory + '/' + methodPrefixTrackLevel + 'sampled*.root')
            if len(sampledFilesList) == 0:

                B2INFO('flavorTagger: trackLevelTeacher did not find any ' + filesDirectory + '/' +
                       methodPrefixTrackLevel + "sampled*.root" + ' file. Please run the flavorTagger in "Sampler" mode.')

            else:

                B2INFO('flavorTagger: MVA Teacher training' + methodPrefixTrackLevel + ' .')
                trainingOptionsTrackLevel = basf2_mva.GeneralOptions()
                trainingOptionsTrackLevel.m_datafiles = basf2_mva.vector(*sampledFilesList)
                trainingOptionsTrackLevel.m_treename = methodPrefixTrackLevel + "_tree"
                trainingOptionsTrackLevel.m_identifier = weightFile
                trainingOptionsTrackLevel.m_variables = basf2_mva.vector(*variables[category])
                trainingOptionsTrackLevel.m_target_variable = targetVariable
                trainingOptionsTrackLevel.m_max_events = maxEventsNumber

                basf2_mva.teacher(trainingOptionsTrackLevel, fastBDTCategories)

                if uploadFlag:
                    basf2_mva.upload(weightFile, methodPrefixTrackLevel)

        else:

            ReadyMethods += 1

    if ReadyMethods != len(trackLevelParticleLists):
        return False
    else:
        return True


def eventLevel(mode='Expert', weightFiles='B2JpsiKs_mu', path=analysis_main):
    """
    Samples data for training or tests all categories all categories at event level.
    """

    B2INFO('EVENT LEVEL')

    ReadyMethods = 0

    # Each category has its own Path in order to be skipped if the corresponding particle list is empty
    identifiersExtrainfosDict = dict()

    for (particleList, category, combinerVariable) in eventLevelParticleLists:

        methodPrefixEventLevel = belleOrBelle2Flag + buildOrRevisionFlag + weightFiles + 'EventLevel' + category + 'FBDT'
        identifierEventLevel = filesDirectory + '/' + methodPrefixEventLevel + '_1.root'
        targetVariable = 'isRightCategory(' + category + ')'
        extraInfoName = targetVariable

        if not os.path.isfile(identifierEventLevel):
            if mode == 'Expert':
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

            if mode == 'Sampler':

                B2INFO(
                    'flavorTagger: file ' +
                    filesDirectory +
                    '/' +
                    methodPrefixEventLevel +
                    "sampled" +
                    fileId +
                    '.root will be saved.')

                eventLevelpath = create_path()
                SkipEmptyParticleList = register_module("SkimFilter")
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

        if os.path.isfile(identifierEventLevel) and mode != 'Teacher':

            B2INFO('flavorTagger: MVAExpert ' + methodPrefixEventLevel + ' ready.')

            if particleList not in identifiersExtrainfosDict:
                identifiersExtrainfosDict[particleList] = [(extraInfoName, identifierEventLevel)]
            else:
                identifiersExtrainfosDict[particleList].append((extraInfoName, identifierEventLevel))

            ReadyMethods += 1

    if ReadyMethods != len(eventLevelParticleLists):
        return False
    else:
        # Each category has its own Path in order to be skipped if the corresponding particle list is empty
        for particleList in identifiersExtrainfosDict:
            eventLevelPath = create_path()
            SkipEmptyParticleList = register_module("SkimFilter")
            SkipEmptyParticleList.param('particleLists', particleList)
            SkipEmptyParticleList.if_true(eventLevelPath, AfterConditionPath.CONTINUE)
            path.add_module(SkipEmptyParticleList)

            eventLevelPath.add_module(
                'MVAMultipleExperts',
                listNames=[particleList],
                extraInfoNames=[row[0] for row in identifiersExtrainfosDict[particleList]],
                signalFraction=signalFraction,
                identifiers=[row[1] for row in identifiersExtrainfosDict[particleList]])
        return True


def eventLevelTeacher(weightFiles='B2JpsiKs_mu'):
    """
    Trains all categories at event level.
    """

    B2INFO('EVENT LEVEL TEACHER')
    if not Belle2.FileSystem.findFile(filesDirectory):
        B2FATAL('flavorTagger: THE NEEDED DIRECTORY "./FlavorTagging/TrainedMethods" DOES NOT EXIST!')

    ReadyMethods = 0

    for (particleList, category, combinerVariable) in eventLevelParticleLists:

        methodPrefixEventLevel = belleOrBelle2Flag + buildOrRevisionFlag + weightFiles + 'EventLevel' + category + 'FBDT'
        targetVariable = 'isRightCategory(' + category + ')'
        weightFile = filesDirectory + '/' + methodPrefixEventLevel + "_1.root"

        if not os.path.isfile(weightFile):
            sampledFilesList = glob.glob(filesDirectory + '/' + methodPrefixEventLevel + 'sampled*.root')
            if len(sampledFilesList) == 0:

                B2INFO('flavorTagger: eventLevelTeacher did not find any ' + methodPrefixEventLevel +
                       ".root" + ' file. Please run the flavorTagger in "Sampler" mode.')

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


def trackAndEventLevels(mode='Expert', weightFiles='B2JpsiKs_mu', path=analysis_main):
    """
    Tests all categories at event and track level.
    """

    B2INFO('TRACK AND EVENT LEVELS')

    ReadyMethods = 0

    identifiersExtrainfosDict = dict()
    identifiersExtrainfosKaonPion = []

    # check if methods are ready and download them from Database if specified
    for (particleList, category) in trackLevelParticleLists:

        methodPrefixTrackLevel = belleOrBelle2Flag + buildOrRevisionFlag + weightFiles + 'TrackLevel' + category + 'FBDT'
        identifierTrackLevel = filesDirectory + '/' + methodPrefixTrackLevel + '_1.root'
        targetVariable = 'isRightTrack(' + category + ')'
        extraInfoName = targetVariable

        if not os.path.isfile(identifierTrackLevel):
            if mode == 'Expert':
                if downloadFlag:
                    basf2_mva.download(methodPrefixTrackLevel, identifierTrackLevel)
                    if not os.path.isfile(identifierTrackLevel):
                        B2FATAL('Flavor Tagger: Weight file ' + identifierTrackLevel +
                                ' was not downloaded from Database. Please check the buildOrRevision name. Stopped')
                else:
                    B2FATAL(
                        'Flavor Tagger: ' +
                        particleList +
                        ' Tracklevel was not trained. Weight file ' +
                        identifierTrackLevel + ' was not found. Stopped')

        if os.path.isfile(identifierTrackLevel) and mode != 'Teacher':
            B2INFO('flavorTagger: MVAExpert ' + methodPrefixTrackLevel + ' ready.')

            if particleList not in identifiersExtrainfosDict:
                identifiersExtrainfosDict[particleList] = [(extraInfoName, identifierTrackLevel)]
            else:
                identifiersExtrainfosDict[particleList].append((extraInfoName, identifierTrackLevel))

            ReadyMethods += 1

    for (particleList, category, combinerVariable) in eventLevelParticleLists:

        methodPrefixEventLevel = belleOrBelle2Flag + buildOrRevisionFlag + weightFiles + 'EventLevel' + category + 'FBDT'
        identifierEventLevel = filesDirectory + '/' + methodPrefixEventLevel + '_1.root'
        targetVariable = 'isRightCategory(' + category + ')'
        extraInfoName = targetVariable

        if not os.path.isfile(identifierEventLevel):
            if mode == 'Expert':
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

            if particleList not in identifiersExtrainfosDict and category != 'KaonPion':
                identifiersExtrainfosDict[particleList] = [(extraInfoName, identifierEventLevel)]
            elif category != 'KaonPion':
                identifiersExtrainfosDict[particleList].append((extraInfoName, identifierEventLevel))
            else:
                identifiersExtrainfosKaonPion.append((extraInfoName, identifierEventLevel))

            ReadyMethods += 1

    allMethods = len(trackLevelParticleLists) + len(eventLevelParticleLists)
    if ReadyMethods != allMethods:
        return False
    else:
        # Each category has its own Path in order to be skipped if the corresponding particle list is empty
        for particleList in identifiersExtrainfosDict:
            trackAndEventLevelPath = create_path()
            SkipEmptyParticleList = register_module("SkimFilter")
            SkipEmptyParticleList.param('particleLists', particleList)
            SkipEmptyParticleList.if_true(trackAndEventLevelPath, AfterConditionPath.CONTINUE)
            path.add_module(SkipEmptyParticleList)

            trackAndEventLevelPath.add_module(
                'MVAMultipleExperts',
                listNames=[particleList],
                extraInfoNames=[row[0] for row in identifiersExtrainfosDict[particleList]],
                signalFraction=signalFraction,
                identifiers=[row[1] for row in identifiersExtrainfosDict[particleList]])

        if 'KaonPion' in [row[1] for row in eventLevelParticleLists]:
            trackAndEventLevelKaonPionPath = create_path()
            SkipEmptyParticleList = register_module("SkimFilter")
            SkipEmptyParticleList.param('particleLists', 'K+:inRoe')
            SkipEmptyParticleList.if_true(trackAndEventLevelKaonPionPath, AfterConditionPath.CONTINUE)
            path.add_module(SkipEmptyParticleList)

            trackAndEventLevelKaonPionPath.add_module(
                'MVAExpert',
                listNames=['K+:inRoe'],
                extraInfoName=identifiersExtrainfosKaonPion[0][0],
                signalFraction=signalFraction,
                identifier=identifiersExtrainfosKaonPion[0][1])
        return True


def combinerLevel(mode='Expert', weightFiles='B2JpsiKs_mu', path=analysis_main):
    """
    Samples the input data or tests the combiner according to the selected categories.
    """

    B2INFO('COMBINER LEVEL')

    methodPrefixCombinerLevel = belleOrBelle2Flag + buildOrRevisionFlag + weightFiles + 'Combiner' \
        + categoriesCombinationCode

    if mode == 'Sampler':

        if not (
            os.path.isfile(
                filesDirectory +
                '/' +
                methodPrefixCombinerLevel +
                'FBDT' +
                '_1.root') or os.path.isfile(
                filesDirectory +
                '/' +
                methodPrefixCombinerLevel +
                'FANN' +
                '_1.root')):
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
                path.add_module('MVAMultipleExperts',
                                listNames=[],
                                extraInfoNames=[
                                    'qrCombined' + 'FBDT',
                                    'qrCombined' + 'FANN'],
                                signalFraction=signalFraction,
                                identifiers=[
                                    filesDirectory +
                                    '/' +
                                    methodPrefixCombinerLevel +
                                    'FBDT' +
                                    "_1.root",
                                    filesDirectory +
                                    '/' +
                                    methodPrefixCombinerLevel +
                                    'FANN' +
                                    "_1.root"])
                return True


def combinerLevelTeacher(weightFiles='B2JpsiKs_mu'):
    """
    Trains the combiner according to the selected categories.
    """

    B2INFO('COMBINER LEVEL TEACHER')

    methodPrefixCombinerLevel = belleOrBelle2Flag + buildOrRevisionFlag + weightFiles + 'Combiner' \
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
    particleList,
    mode='Expert',
    weightFiles='B2JpsiKs_mu',
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
        'FastPion',
        'Lambda',
        'FSC',
        'MaximumPstar',
        'KaonPion'],
    belleOrBelle2="Belle2",
    buildOrRevision='notDefined',
    downloadFromDatabaseIfNotfound=True,
    uploadToDatabaseAfterTraining=False,
    samplerFileId='',
    path=analysis_main,
):
    """
      Defines the whole flavor tagging process.
      For each Rest of Event built in the steering file.
      The flavor is predicted by Multivariate Methods trained with Variables and MetaVariables which use
      Tracks, ECL- and KLMClusters from the corresponding RestOfEvent dataobject.
      This function can be used to train or to test the flavorTagger: The available modes are "Teacher" or "Expert".
    """

    if mode != 'Sampler' and mode != 'Teacher' and mode != 'Expert':
        B2FATAL('flavorTagger: Wrong mode given: The available modes are "Sampler", "Teacher" or "Expert"')

    # Directory where the weights of the trained Methods are saved
    # workingDirectory = os.environ['BELLE2_LOCAL_DIR'] + '/analysis/data'

    if not Belle2.FileSystem.findFile(workingDirectory):
        B2FATAL('flavorTagger: THE GIVEN WORKING DIRECTORY "' + workingDirectory + '" DOES NOT EXIST! PLEASE SPECIFY A VALID PATH.')

    if mode == 'Sampler' or (mode == 'Expert' and downloadFromDatabaseIfNotfound):
        if not Belle2.FileSystem.findFile(workingDirectory + '/FlavorTagging'):
            os.mkdir(workingDirectory + '/FlavorTagging')
            os.mkdir(workingDirectory + '/FlavorTagging/TrainedMethods')
        elif not Belle2.FileSystem.findFile(workingDirectory + '/FlavorTagging/TrainedMethods'):
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

    setBelleOrBelle2AndRevision(belleOrBelle2, buildOrRevision)
    setInteractionWithDatabase(downloadFromDatabaseIfNotfound, uploadToDatabaseAfterTraining)
    WhichCategories(categories)
    setVariables()

    roe_path = create_path()
    deadEndPath = create_path()

    # Events containing ROE without B-Meson (but not empty) are discarded for training
    if mode == 'Sampler':
        signalSideParticleFilter(particleList, 'hasRestOfEventTracks > 0 and qrCombined > -2', roe_path, deadEndPath)

    # If trigger returns 1 jump into empty path skipping further modules in roe_path
    if mode == 'Expert':
        signalSideParticleFilter(particleList, 'hasRestOfEventTracks > 0', roe_path, deadEndPath)
        # Initialation of flavorTaggerInfo dataObject needs to be done in the main path
        flavorTaggerInfoBuilder = register_module('FlavorTaggerInfoBuilder')
        path.add_module(flavorTaggerInfoBuilder)

    # sampler or expert
    if mode == 'Sampler' or mode == 'Expert':
        if FillParticleLists(mode, roe_path):
            if not trackAndEventLevels(mode, weightFiles, roe_path):
                if trackLevel(mode, weightFiles, roe_path):
                    eventLevel(mode, weightFiles, roe_path)
            else:
                combinerLevel(mode, weightFiles, roe_path)
                if mode == 'Expert':
                    flavorTaggerInfoFiller = register_module('FlavorTaggerInfoFiller')
                    flavorTaggerInfoFiller.param('trackLevelParticleLists', trackLevelParticleLists)
                    flavorTaggerInfoFiller.param('eventLevelParticleLists', eventLevelParticleLists)
                    flavorTaggerInfoFiller.param('TMVAfbdt', TMVAfbdt)
                    flavorTaggerInfoFiller.param('FANNmlp', FANNmlp)
                    flavorTaggerInfoFiller.param('qrCategories', True)
                    flavorTaggerInfoFiller.param('istrueCategories', True)
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
        if trackLevelTeacher(weightFiles):
            if eventLevelTeacher(weightFiles):
                combinerLevelTeacher(weightFiles)
