#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# *************  Flavor Tagging   ************
# * This script is needed to train           *
# * and to test the flavor tagger.           *
# ********************************************

from basf2 import B2INFO, B2FATAL
import basf2
import basf2_mva
import inspect
import modularAnalysis as ma
from variables import utils
import os
import glob
import b2bii
import collections


def getBelleOrBelle2():
    """
    Gets the global ModeCode.
    """
    if b2bii.isB2BII():
        return 'Belle'
    else:
        return 'Belle2'


def setInteractionWithDatabase(downloadFromDatabaseIfNotFound=False, uploadToDatabaseAfterTraining=False):
    """
    Sets the interaction with the database: download trained weight files or upload weight files after training.
    """

    global downloadFlag
    global uploadFlag

    downloadFlag = downloadFromDatabaseIfNotFound
    uploadFlag = uploadToDatabaseAfterTraining


# Default list of aliases that should be used to save the flavor tagging information using VariablesToNtuple
flavor_tagging = ['FBDT_qrCombined', 'FANN_qrCombined', 'qrMC', 'mcFlavorOfOtherB',
                  'qpElectron', 'hasTrueTargetElectron', 'isRightCategoryElectron',
                  'qpIntermediateElectron', 'hasTrueTargetIntermediateElectron', 'isRightCategoryIntermediateElectron',
                  'qpMuon', 'hasTrueTargetMuon', 'isRightCategoryMuon',
                  'qpIntermediateMuon', 'hasTrueTargetIntermediateMuon', 'isRightCategoryIntermediateMuon',
                  'qpKinLepton', 'hasTrueTargetKinLepton', 'isRightCategoryKinLepton',
                  'qpIntermediateKinLepton', 'hasTrueTargetIntermediateKinLepton', 'isRightCategoryIntermediateKinLepton',
                  'qpKaon', 'hasTrueTargetKaon', 'isRightCategoryKaon',
                  'qpSlowPion', 'hasTrueTargetSlowPion', 'isRightCategorySlowPion',
                  'qpFastHadron', 'hasTrueTargetFastHadron', 'isRightCategoryFastHadron',
                  'qpLambda', 'hasTrueTargetLambda', 'isRightCategoryLambda',
                  'qpFSC', 'hasTrueTargetFSC', 'isRightCategoryFSC',
                  'qpMaximumPstar', 'hasTrueTargetMaximumPstar', 'isRightCategoryMaximumPstar',
                  'qpKaonPion', 'hasTrueTargetKaonPion', 'isRightCategoryKaonPion']


def add_default_FlavorTagger_aliases():
    """
    This function adds the default aliases for flavor tagging variables
    and defines the collection of flavor tagging variables.
    """

    utils._variablemanager.addAlias('FBDT_qrCombined', 'qrOutput(FBDT)')
    utils._variablemanager.addAlias('FANN_qrCombined', 'qrOutput(FANN)')
    utils._variablemanager.addAlias('qrMC', 'isRelatedRestOfEventB0Flavor')

    for iCategory in AvailableCategories:
        aliasForQp = 'qp' + iCategory
        aliasForTrueTarget = 'hasTrueTarget' + iCategory
        aliasForIsRightCategory = 'isRightCategory' + iCategory
        utils._variablemanager.addAlias(aliasForQp, 'qpCategory(' + iCategory + ')')
        utils._variablemanager.addAlias(aliasForTrueTarget, 'hasTrueTargets(' + iCategory + ')')
        utils._variablemanager.addAlias(aliasForIsRightCategory, 'isTrueFTCategory(' + iCategory + ')')

    utils.add_collection(flavor_tagging, 'flavor_tagging')


def set_FlavorTagger_pid_aliases():
    """
    This function adds the pid aliases needed by the flavor tagger.
    """
    utils._variablemanager.addAlias('eid_TOP', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, TOP), 0.5)')
    utils._variablemanager.addAlias('eid_ARICH', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, ARICH), 0.5)')
    utils._variablemanager.addAlias('eid_ECL', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, ECL), 0.5)')

    utils._variablemanager.addAlias('muid_TOP', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, TOP), 0.5)')
    utils._variablemanager.addAlias('muid_ARICH', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, ARICH), 0.5)')
    utils._variablemanager.addAlias('muid_KLM', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, KLM), 0.5)')

    utils._variablemanager.addAlias('piid_TOP', 'ifNANgiveX(pidPairProbabilityExpert(211, 321, TOP), 0.5)')
    utils._variablemanager.addAlias('piid_ARICH', 'ifNANgiveX(pidPairProbabilityExpert(211, 321, ARICH), 0.5)')

    utils._variablemanager.addAlias('Kid_TOP', 'ifNANgiveX(pidPairProbabilityExpert(321, 211, TOP), 0.5)')
    utils._variablemanager.addAlias('Kid_ARICH', 'ifNANgiveX(pidPairProbabilityExpert(321, 211, ARICH), 0.5)')

    if getBelleOrBelle2() == "Belle":
        utils._variablemanager.addAlias('eid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, CDC, SVD), 0.5)')
        utils._variablemanager.addAlias('muid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, CDC, SVD), 0.5)')
        utils._variablemanager.addAlias('piid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(211, 321, CDC, SVD), 0.5)')
        utils._variablemanager.addAlias('pi_vs_edEdxid', 'ifNANgiveX(pidPairProbabilityExpert(211, 11, CDC, SVD), 0.5)')
        utils._variablemanager.addAlias('Kid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(321, 211, CDC, SVD), 0.5)')
    else:
        # Removed SVD PID for Belle II MC and data as it is absent in release 4.
        utils._variablemanager.addAlias('eid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, CDC), 0.5)')
        utils._variablemanager.addAlias('muid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, CDC), 0.5)')
        utils._variablemanager.addAlias('piid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(211, 321, CDC), 0.5)')
        utils._variablemanager.addAlias('pi_vs_edEdxid', 'ifNANgiveX(pidPairProbabilityExpert(211, 11, CDC), 0.5)')
        utils._variablemanager.addAlias('Kid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(321, 211, CDC), 0.5)')


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
maxEventsNumber = 0  # 0 takes all the sampled events. The number in the past was 500000


FTCategoryParameters = collections.namedtuple('FTCategoryParameters',
                                              ['particleList', 'trackName', 'eventName', 'variableName', 'code'])
# Definition of all available categories, 'standard category name':
# ['ParticleList', 'trackLevel category name', 'eventLevel category name',
# 'combinerLevel variable name', 'category code']
AvailableCategories = {
    'Electron':
    FTCategoryParameters('e+:inRoe', 'Electron', 'Electron',
                         'QpOf(e+:inRoe, isRightCategory(Electron), isRightCategory(Electron))',
                         0),
    'IntermediateElectron':
    FTCategoryParameters('e+:inRoe', 'IntermediateElectron', 'IntermediateElectron',
                         'QpOf(e+:inRoe, isRightCategory(IntermediateElectron), isRightCategory(IntermediateElectron))',
                         1),
    'Muon':
    FTCategoryParameters('mu+:inRoe', 'Muon', 'Muon',
                         'QpOf(mu+:inRoe, isRightCategory(Muon), isRightCategory(Muon))',
                         2),
    'IntermediateMuon':
    FTCategoryParameters('mu+:inRoe', 'IntermediateMuon', 'IntermediateMuon',
                         'QpOf(mu+:inRoe, isRightCategory(IntermediateMuon), isRightCategory(IntermediateMuon))',
                         3),
    'KinLepton':
    FTCategoryParameters('mu+:inRoe', 'KinLepton', 'KinLepton',
                         'QpOf(mu+:inRoe, isRightCategory(KinLepton), isRightCategory(KinLepton))',
                         4),
    'IntermediateKinLepton':
    FTCategoryParameters('mu+:inRoe', 'IntermediateKinLepton', 'IntermediateKinLepton',
                         'QpOf(mu+:inRoe, isRightCategory(IntermediateKinLepton), isRightCategory(IntermediateKinLepton))',
                         5),
    'Kaon':
    FTCategoryParameters('K+:inRoe', 'Kaon', 'Kaon',
                         'weightedQpOf(K+:inRoe, isRightCategory(Kaon), isRightCategory(Kaon))',
                         6),
    'SlowPion':
    FTCategoryParameters('pi+:inRoe', 'SlowPion', 'SlowPion',
                         'QpOf(pi+:inRoe, isRightCategory(SlowPion), isRightCategory(SlowPion))',
                         7),
    'FastHadron':
    FTCategoryParameters('pi+:inRoe', 'FastHadron', 'FastHadron',
                         'QpOf(pi+:inRoe, isRightCategory(FastHadron), isRightCategory(FastHadron))',
                         8),
    'Lambda':
    FTCategoryParameters('Lambda0:inRoe', 'Lambda', 'Lambda',
                         'weightedQpOf(Lambda0:inRoe, isRightCategory(Lambda), isRightCategory(Lambda))',
                         9),
    'FSC':
    FTCategoryParameters('pi+:inRoe', 'SlowPion', 'FSC',
                         'QpOf(pi+:inRoe, isRightCategory(FSC), isRightCategory(SlowPion))',
                         10),
    'MaximumPstar':
    FTCategoryParameters('pi+:inRoe', 'MaximumPstar', 'MaximumPstar',
                         'QpOf(pi+:inRoe, isRightCategory(MaximumPstar), isRightCategory(MaximumPstar))',
                         11),
    'KaonPion':
    FTCategoryParameters('K+:inRoe', 'Kaon', 'KaonPion',
                         'QpOf(K+:inRoe, isRightCategory(KaonPion), isRightCategory(Kaon))',
                         12),
}


# Variables for categories on track level - are defined in variables.cc and MetaVariables.cc
variables = dict()
KId = {'Belle': 'ifNANgiveX(atcPIDBelle(3,2), 0.5)', 'Belle2': 'kaonID'}
muId = {'Belle': 'muIDBelle', 'Belle2': 'muonID'}
eId = {'Belle': 'eIDBelle', 'Belle2': 'electronID'}


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
        'eid_TOP',
        'eid_ARICH',
        'eid_ECL',
        'BtagToWBosonVariables(recoilMassSqrd)',
        'BtagToWBosonVariables(pMissCMS)',
        'BtagToWBosonVariables(cosThetaMissCMS)',
        'BtagToWBosonVariables(EW90)',
        'cosTPTO',
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
        'muid_TOP',
        'muid_ARICH',
        'muid_KLM',
        'BtagToWBosonVariables(recoilMassSqrd)',
        'BtagToWBosonVariables(pMissCMS)',
        'BtagToWBosonVariables(cosThetaMissCMS)',
        'BtagToWBosonVariables(EW90)',
        'cosTPTO',
    ]
    variables['IntermediateMuon'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        'cosTheta',
        muId[getBelleOrBelle2()],
        'muid_TOP',
        'muid_ARICH',
        'muid_KLM',
        'BtagToWBosonVariables(recoilMassSqrd)',
        'BtagToWBosonVariables(pMissCMS)',
        'BtagToWBosonVariables(cosThetaMissCMS)',
        'BtagToWBosonVariables(EW90)',
        'cosTPTO',
        'chiProb',
    ]
    variables['KinLepton'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        'cosTheta',
        muId[getBelleOrBelle2()],
        'muid_TOP',
        'muid_ARICH',
        'muid_KLM',
        eId[getBelleOrBelle2()],
        'eid_TOP',
        'eid_ARICH',
        'eid_ECL',
        'BtagToWBosonVariables(recoilMassSqrd)',
        'BtagToWBosonVariables(pMissCMS)',
        'BtagToWBosonVariables(cosThetaMissCMS)',
        'BtagToWBosonVariables(EW90)',
        'cosTPTO',
    ]
    variables['IntermediateKinLepton'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        'cosTheta',
        muId[getBelleOrBelle2()],
        'muid_TOP',
        'muid_ARICH',
        'muid_KLM',
        eId[getBelleOrBelle2()],
        'eid_TOP',
        'eid_ARICH',
        'eid_ECL',
        'BtagToWBosonVariables(recoilMassSqrd)',
        'BtagToWBosonVariables(pMissCMS)',
        'BtagToWBosonVariables(cosThetaMissCMS)',
        'BtagToWBosonVariables(EW90)',
        'cosTPTO',
        'chiProb',
    ]
    variables['Kaon'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        'cosTheta',
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
        'chiProb',
    ]
    variables['SlowPion'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'cosTheta',
        'p',
        'pt',
        'pionID',
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
    ]
    variables['FastHadron'] = [
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
        'cosTPTO',
    ]
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
        'chiProb',
    ]
    variables['MaximumPstar'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        'cosTPTO',
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

    # Special treatment for some input variables:
    if getBelleOrBelle2() == "Belle2":
        variables['Lambda'].append('daughter(1,protonID)')  # protonID always 0 in B2BII check in future
        variables['Lambda'].append('daughter(0,pionID)')  # not very powerful in B2BII
    else:
        # Below we add some input variables in case of Belle B2BII samples.
        # They are added only for Belle samples because they lead to large data/MC discrepancies at Belle II.
        # Add them for Belle II samples, when their distributions will have good data/MC agreement.
        variables['Electron'].append('eid_dEdx')
        variables['Electron'].append('ImpactXY')
        variables['Electron'].append('distance')

        variables['IntermediateElectron'].append('eid_dEdx')
        variables['IntermediateElectron'].append('ImpactXY')
        variables['IntermediateElectron'].append('distance')

        variables['Muon'].append('muid_dEdx')
        variables['Muon'].append('ImpactXY')
        variables['Muon'].append('distance')
        variables['Muon'].append('chiProb')

        variables['IntermediateMuon'].append('muid_dEdx')
        variables['IntermediateMuon'].append('ImpactXY')
        variables['IntermediateMuon'].append('distance')

        variables['KinLepton'].append('muid_dEdx')
        variables['KinLepton'].append('eid_dEdx')
        variables['KinLepton'].append('ImpactXY')
        variables['KinLepton'].append('distance')
        variables['KinLepton'].append('chiProb')

        variables['IntermediateKinLepton'].append('muid_dEdx')
        variables['IntermediateKinLepton'].append('eid_dEdx')
        variables['IntermediateKinLepton'].append('ImpactXY')
        variables['IntermediateKinLepton'].append('distance')

        variables['Kaon'].append('ImpactXY')
        variables['Kaon'].append('distance')

        variables['SlowPion'].append('piid_dEdx')
        variables['SlowPion'].append('ImpactXY')
        variables['SlowPion'].append('distance')
        variables['SlowPion'].append('chiProb')

        variables['FastHadron'].append('BtagToWBosonVariables(pMissCMS)')
        variables['FastHadron'].append('ImpactXY')
        variables['FastHadron'].append('distance')
        variables['FastHadron'].append('chiProb')

        variables['Lambda'].append('distance')

        variables['MaximumPstar'].append('ImpactXY')
        variables['MaximumPstar'].append('distance')


def FillParticleLists(maskName='all', categories=None, path=None):
    """
    Fills the particle Lists for all categories.
    """

    from vertex import kFit
    readyParticleLists = []

    if categories is None:
        categories = []

    trackCut = 'isInRestOfEvent > 0.5 and passesROEMask(' + maskName + ') > 0.5 and ' + 'isNAN(p) !=1 and isInfinity(p) != 1'

    for category in categories:
        particleList = AvailableCategories[category].particleList

        if particleList in readyParticleLists:
            continue

        # Select particles in ROE for different categories according to mass hypothesis.
        if particleList == 'Lambda0:inRoe':
            if 'pi+:inRoe' not in readyParticleLists:
                ma.fillParticleList('pi+:inRoe', trackCut, path=path)
                readyParticleLists.append('pi+:inRoe')

            ma.fillParticleList('p+:inRoe', trackCut, path=path)
            ma.reconstructDecay(particleList + ' -> pi-:inRoe p+:inRoe', '1.00<=M<=1.23', False, path=path)
            kFit(particleList, 0.01, path=path)
            ma.matchMCTruth(particleList, path=path)
            readyParticleLists.append(particleList)

        else:
            # Filling particle list for actual category
            ma.fillParticleList(particleList, trackCut, path=path)
            readyParticleLists.append(particleList)

    # Additional particleLists for K_S0
    if getBelleOrBelle2() == 'Belle':
        ma.cutAndCopyList('K_S0:inRoe', 'K_S0:mdst', 'extraInfo(ksnbStandard) == 1 and isInRestOfEvent == 1', path=path)
    else:
        ma.reconstructDecay('K_S0:inRoe -> pi+:inRoe pi-:inRoe', '0.40<=M<=0.60', False, path=path)
        kFit('K_S0:inRoe', 0.01, path=path)


def eventLevel(mode='Expert', weightFiles='B2JpsiKs_mu', categories=None, path=None):
    """
    Samples data for training or tests all categories all categories at event level.
    """

    from basf2 import create_path
    from basf2 import register_module

    B2INFO('EVENT LEVEL')

    ReadyMethods = 0

    # Each category has its own Path in order to be skipped if the corresponding particle list is empty
    identifiersExtraInfosDict = dict()
    identifiersExtraInfosKaonPion = []

    if categories is None:
        categories = []

    for category in categories:
        particleList = AvailableCategories[category].particleList

        methodPrefixEventLevel = "FlavorTagger_" + getBelleOrBelle2() + "_" + weightFiles + 'EventLevel' + category + 'FBDT'
        identifierEventLevel = methodPrefixEventLevel
        targetVariable = 'isRightCategory(' + category + ')'
        extraInfoName = targetVariable

        if mode == 'Expert':

            if downloadFlag or useOnlyLocalFlag:
                identifierEventLevel = filesDirectory + '/' + methodPrefixEventLevel + '_1.root'

            if downloadFlag:
                if not os.path.isfile(identifierEventLevel):
                    basf2_mva.download(methodPrefixEventLevel, identifierEventLevel)
                    if not os.path.isfile(identifierEventLevel):
                        B2FATAL('Flavor Tagger: Weight file ' + identifierEventLevel +
                                ' was not downloaded from Database. Please check the buildOrRevision name. Stopped')

            if useOnlyLocalFlag:
                if not os.path.isfile(identifierEventLevel):
                    B2FATAL('Flavor Tagger: ' + particleList + ' Eventlevel was not trained. Weight file ' +
                            identifierEventLevel + ' was not found. Stopped')

            B2INFO('flavorTagger: MVAExpert ' + methodPrefixEventLevel + ' ready.')

        elif mode == 'Sampler':

            identifierEventLevel = filesDirectory + '/' + methodPrefixEventLevel + '_1.root'
            if os.path.isfile(identifierEventLevel):
                B2INFO('flavorTagger: MVAExpert ' + methodPrefixEventLevel + ' ready.')

                if 'KaonPion' in categories:
                    methodPrefixEventLevelKaonPion = "FlavorTagger_" + getBelleOrBelle2() + \
                        "_" + weightFiles + 'EventLevelKaonPionFBDT'
                    identifierEventLevelKaonPion = filesDirectory + '/' + methodPrefixEventLevelKaonPion + '_1.root'
                    if not os.path.isfile(identifierEventLevelKaonPion):
                        # Slow Pion and Kaon categories are used if Kaon-Pion is lacking for
                        # sampling. The others are not needed and skipped
                        if category != "SlowPion" and category != "Kaon":
                            continue

        if mode == 'Expert' or (mode == 'Sampler' and os.path.isfile(identifierEventLevel)):

            B2INFO('flavorTagger: Applying MVAExpert ' + methodPrefixEventLevel + '.')

            if category == 'KaonPion':
                identifiersExtraInfosKaonPion.append((extraInfoName, identifierEventLevel))
            elif particleList not in identifiersExtraInfosDict:
                identifiersExtraInfosDict[particleList] = [(extraInfoName, identifierEventLevel)]
            else:
                identifiersExtraInfosDict[particleList].append((extraInfoName, identifierEventLevel))

            ReadyMethods += 1

    # Each category has its own Path in order to be skipped if the corresponding particle list is empty
    for particleList in identifiersExtraInfosDict:
        eventLevelPath = create_path()
        SkipEmptyParticleList = register_module("SkimFilter")
        SkipEmptyParticleList.set_name('SkimFilter_EventLevel_' + particleList)
        SkipEmptyParticleList.param('particleLists', particleList)
        SkipEmptyParticleList.if_true(eventLevelPath, basf2.AfterConditionPath.CONTINUE)
        path.add_module(SkipEmptyParticleList)

        mvaMultipleExperts = register_module('MVAMultipleExperts')
        mvaMultipleExperts.set_name('MVAMultipleExperts_EventLevel_' + particleList)
        mvaMultipleExperts.param('listNames', [particleList])
        mvaMultipleExperts.param('extraInfoNames', [row[0] for row in identifiersExtraInfosDict[particleList]])
        mvaMultipleExperts.param('signalFraction', signalFraction)
        mvaMultipleExperts.param('identifiers', [row[1] for row in identifiersExtraInfosDict[particleList]])
        eventLevelPath.add_module(mvaMultipleExperts)

    if 'KaonPion' in categories and len(identifiersExtraInfosKaonPion) != 0:
        eventLevelKaonPionPath = create_path()
        SkipEmptyParticleList = register_module("SkimFilter")
        SkipEmptyParticleList.set_name('SkimFilter_' + 'K+:inRoe')
        SkipEmptyParticleList.param('particleLists', 'K+:inRoe')
        SkipEmptyParticleList.if_true(eventLevelKaonPionPath, basf2.AfterConditionPath.CONTINUE)
        path.add_module(SkipEmptyParticleList)

        mvaExpertKaonPion = register_module("MVAExpert")
        mvaExpertKaonPion.set_name('MVAExpert_KaonPion_' + 'K+:inRoe')
        mvaExpertKaonPion.param('listNames', ['K+:inRoe'])
        mvaExpertKaonPion.param('extraInfoName', identifiersExtraInfosKaonPion[0][0])
        mvaExpertKaonPion.param('signalFraction', signalFraction)
        mvaExpertKaonPion.param('identifier', identifiersExtraInfosKaonPion[0][1])

        eventLevelKaonPionPath.add_module(mvaExpertKaonPion)

    if mode == 'Sampler':

        for category in categories:
            particleList = AvailableCategories[category].particleList

            methodPrefixEventLevel = "FlavorTagger_" + getBelleOrBelle2() + "_" + weightFiles + 'EventLevel' + category + 'FBDT'
            identifierEventLevel = filesDirectory + '/' + methodPrefixEventLevel + '_1.root'
            targetVariable = 'isRightCategory(' + category + ')'

            if not os.path.isfile(identifierEventLevel):

                if category == 'KaonPion':
                    methodPrefixEventLevelSlowPion = "FlavorTagger_" + getBelleOrBelle2() + \
                                                     "_" + weightFiles + 'EventLevelSlowPionFBDT'
                    identifierEventLevelSlowPion = filesDirectory + '/' + methodPrefixEventLevelSlowPion + '_1.root'
                    if not os.path.isfile(identifierEventLevelSlowPion):
                        B2INFO("Flavor Tagger: event level weight file for the Slow Pion category is absent." +
                               "It is required to sample the training information for the KaonPion category." +
                               "An additional sampling step will be needed after the following training step.")
                        continue

                B2INFO('flavorTagger: file ' + filesDirectory + '/' +
                       methodPrefixEventLevel + "sampled" + fileId + '.root will be saved.')

                ma.applyCuts(particleList, 'isRightCategory(mcAssociated) > 0', path)
                eventLevelpath = create_path()
                SkipEmptyParticleList = register_module("SkimFilter")
                SkipEmptyParticleList.set_name('SkimFilter_EventLevel' + category)
                SkipEmptyParticleList.param('particleLists', particleList)
                SkipEmptyParticleList.if_true(eventLevelpath, basf2.AfterConditionPath.CONTINUE)
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

    if ReadyMethods != len(categories):
        return False
    else:
        return True


def eventLevelTeacher(weightFiles='B2JpsiKs_mu', categories=None):
    """
    Trains all categories at event level.
    """

    B2INFO('EVENT LEVEL TEACHER')

    ReadyMethods = 0

    if categories is None:
        categories = []

    for category in categories:
        particleList = AvailableCategories[category].particleList

        methodPrefixEventLevel = "FlavorTagger_" + getBelleOrBelle2() + "_" + weightFiles + 'EventLevel' + category + 'FBDT'
        targetVariable = 'isRightCategory(' + category + ')'
        weightFile = filesDirectory + '/' + methodPrefixEventLevel + "_1.root"

        if os.path.isfile(weightFile):
            ReadyMethods += 1
            continue

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

    if ReadyMethods != len(categories):
        return False
    else:
        return True


def combinerLevel(mode='Expert', weightFiles='B2JpsiKs_mu', categories=None,
                  variablesCombinerLevel=None, categoriesCombinationCode=None, path=None):
    """
    Samples the input data or tests the combiner according to the selected categories.
    """

    B2INFO('COMBINER LEVEL')

    if categories is None:
        categories = []
    if variablesCombinerLevel is None:
        variablesCombinerLevel = []

    B2INFO("Flavor Tagger: Required Combiner for Categories:")
    for category in categories:
        B2INFO(category)

    B2INFO("Flavor Tagger: which corresponds to a weight file with categories combination code " + categoriesCombinationCode)

    methodPrefixCombinerLevel = "FlavorTagger_" + getBelleOrBelle2() + "_" + weightFiles + 'Combiner' \
        + categoriesCombinationCode

    if mode == 'Sampler':

        if os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FBDT' + '_1.root') or \
           os.path.isfile(filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + '_1.root'):
            B2FATAL('flavorTagger: File' + methodPrefixCombinerLevel + 'FBDT' + "_1.root" + ' or ' + methodPrefixCombinerLevel +
                    'FANN' + '_1.root found. Please run the "Expert" mode or delete the file if a new sampling is desired.')

        B2INFO('flavorTagger: Sampling Data on Combiner Level. File' +
               methodPrefixCombinerLevel + ".root" + ' will be saved')

        ntuple = basf2.register_module('VariablesToNtuple')
        ntuple.param('fileName', filesDirectory + '/' + methodPrefixCombinerLevel + "sampled" + fileId + ".root")
        ntuple.param('treeName', methodPrefixCombinerLevel + 'FBDT' + "_tree")
        ntuple.param('variables', variablesCombinerLevel + ['qrCombined'])
        ntuple.param('particleList', "")
        path.add_module(ntuple)

    if mode == 'Expert':

        # Check if weight files are ready
        if TMVAfbdt:
            identifierFBDT = methodPrefixCombinerLevel + 'FBDT'
            if downloadFlag or useOnlyLocalFlag:
                identifierFBDT = filesDirectory + '/' + methodPrefixCombinerLevel + 'FBDT' + '_1.root'

            if downloadFlag:
                if not os.path.isfile(identifierFBDT):
                    basf2_mva.download(methodPrefixCombinerLevel + 'FBDT', identifierFBDT)
                    if not os.path.isfile(identifierFBDT):
                        B2FATAL('Flavor Tagger: Weight file ' + methodPrefixCombinerLevel + 'FBDT' +
                                '_1.root was not downloaded from Database. Please check the buildOrRevision name. Stopped')

            if useOnlyLocalFlag:
                if not os.path.isfile(identifierFBDT):
                    B2FATAL('flavorTagger: Combinerlevel FastBDT was not trained with this combination of categories.' +
                            ' Weight file ' + methodPrefixCombinerLevel + 'FBDT' + '_1.root not found. Stopped')

            B2INFO('flavorTagger: Ready to be used with weightFile ' + methodPrefixCombinerLevel + 'FBDT' + '_1.root')

        if FANNmlp:
            identifierFANN = methodPrefixCombinerLevel + 'FANN'
            if downloadFlag or useOnlyLocalFlag:
                identifierFANN = filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + '_1.root'

            if downloadFlag:
                if not os.path.isfile(identifierFANN):
                    basf2_mva.download(methodPrefixCombinerLevel + 'FANN', identifierFANN)
                    if not os.path.isfile(identifierFANN):
                        B2FATAL('Flavor Tagger: Weight file ' + methodPrefixCombinerLevel + 'FANN' +
                                '_1.root was not downloaded from Database. Please check the buildOrRevision name. Stopped')
            if useOnlyLocalFlag:
                B2FATAL('flavorTagger: Combinerlevel FANNMLP was not trained with this combination of categories. ' +
                        ' Weight file ' + methodPrefixCombinerLevel + 'FANN' + '_1.root not found. Stopped')

            B2INFO('flavorTagger: Ready to be used with weightFile ' + methodPrefixCombinerLevel + 'FANN' + '_1.root')

        # At this stage, all necessary weight files should be ready.
        # Call MVAExpert or MVAMultipleExperts module.
        if TMVAfbdt and not FANNmlp:
            B2INFO('flavorTagger: Apply FBDTMethod ' + methodPrefixCombinerLevel + 'FBDT')
            path.add_module('MVAExpert', listNames=[], extraInfoName='qrCombined' + 'FBDT', signalFraction=signalFraction,
                            identifier=identifierFBDT)

        if FANNmlp and not TMVAfbdt:
            B2INFO('flavorTagger: Apply FANNMethod on combiner level')
            path.add_module('MVAExpert', listNames=[], extraInfoName='qrCombined' + 'FANN', signalFraction=signalFraction,
                            identifier=identifierFANN)

        if FANNmlp and TMVAfbdt:
            B2INFO('flavorTagger: Apply FANNMethod and FBDTMethod  on combiner level')
            mvaMultipleExperts = basf2.register_module('MVAMultipleExperts')
            mvaMultipleExperts.set_name('MVAMultipleExperts_Combiners')
            mvaMultipleExperts.param('listNames', [])
            mvaMultipleExperts.param('extraInfoNames', ['qrCombined' + 'FBDT', 'qrCombined' + 'FANN'])
            mvaMultipleExperts.param('signalFraction', signalFraction)
            mvaMultipleExperts.param('identifiers', [identifierFBDT, identifierFANN])
            path.add_module(mvaMultipleExperts)


def combinerLevelTeacher(weightFiles='B2JpsiKs_mu', variablesCombinerLevel=None,
                         categoriesCombinationCode=None):
    """
    Trains the combiner according to the selected categories.
    """

    B2INFO('COMBINER LEVEL TEACHER')

    if variablesCombinerLevel is None:
        variablesCombinerLevel = []

    methodPrefixCombinerLevel = "FlavorTagger_" + getBelleOrBelle2() + "_" + weightFiles + 'Combiner' \
        + categoriesCombinationCode

    sampledFilesList = glob.glob(filesDirectory + '/' + methodPrefixCombinerLevel + 'sampled*.root')
    if len(sampledFilesList) == 0:
        B2FATAL('FlavorTagger: combinerLevelTeacher did not find any ' +
                methodPrefixCombinerLevel + 'sampled*.root file. Please run the flavorTagger in "Sampler" mode.')

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


def getEventLevelParticleLists(categories=None):

    if categories is None:
        categories = []

    eventLevelParticleLists = []

    for category in categories:
        ftCategory = AvailableCategories[category]
        event_tuple = (ftCategory.particleList, ftCategory.eventName, ftCategory.variableName)

        if event_tuple not in eventLevelParticleLists:
            eventLevelParticleLists.append(event_tuple)
        else:
            B2FATAL('Flavor Tagger: ' + category + ' has been already given')

    return eventLevelParticleLists


def flavorTagger(
    particleLists=None,
    mode='Expert',
    weightFiles='B2nunubarBGx1',
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
    maskName='all',
    saveCategoriesInfo=True,
    useOnlyLocalWeightFiles=False,
    downloadFromDatabaseIfNotFound=False,
    uploadToDatabaseAfterTraining=False,
    samplerFileId='',
    path=None,
):
    """
      Defines the whole flavor tagging process for each selected Rest of Event (ROE) built in the steering file.
      The flavor is predicted by Multivariate Methods trained with Variables and MetaVariables which use
      Tracks, ECL- and KLMClusters from the corresponding RestOfEvent dataobject.
      This module can be used to sample the training information, to train and/or to test the flavorTagger.

      @param particleLists                     The ROEs for flavor tagging are selected from the given particle lists.
      @param mode                              The available modes are
                                               ``Expert`` (default), ``Sampler``, and ``Teacher``. In the ``Expert`` mode
                                               Flavor Tagging is applied to the analysis,. In the ``Sampler`` mode you save
                                               save the variables for training. In the ``Teacher`` mode the FlavorTagger is
                                               trained, for this step you do not reconstruct any particle or do any analysis,
                                               you just run the flavorTagger alone.
      @param weightFiles                       Weight files name. Default=
                                               ``B2nunubarBGx1`` (official weight files). If the user self
                                               wants to train the FlavorTagger, the weightfiles name should correspond to the
                                               analysed CP channel in order to avoid confusions. The default name
                                               ``B2nunubarBGx1`` corresponds to
                                               :math:`B^0_{\\rm sig}\\to \\nu \\overline{\\nu}`.
                                               and ``B2JpsiKs_muBGx1`` to
                                               :math:`B^0_{\\rm sig}\\to J/\\psi (\\to \\mu^+ \\mu^-) K_s (\\to \\pi^+ \\pi^-)`.
                                               BGx1 stays for events simulated with background.
      @param workingDirectory                  Path to the directory containing the FlavorTagging/ folder.
      @param combinerMethods                   MVAs for the combiner: ``TMVA-FBDT`` or ``FANN-MLP``. Both used by default.
      @param categories                        Categories used for flavor tagging. By default all are used.
      @param maskName                          Gets ROE particles from a specified ROE mask.
      @param saveCategoriesInfo                Sets to save information of individual categories.
      @param useOnlyLocalWeightFiles           [Expert] Uses only locally saved weight files.
      @param downloadFromDatabaseIfNotFound    [Expert] Weight files are downloaded from
                                               the conditions database if not available in workingDirectory.
      @param uploadToDatabaseAfterTraining     [Expert] For librarians only: uploads weight files to localdb after training.
      @param samplerFileId                     Identifier to paralellize
                                               sampling. Only used in ``Sampler`` mode.  If you are training by yourself and
                                               want to parallelize the sampling, you can run several sampling scripts in
                                               parallel. By changing this parameter you will not overwrite an older sample.
      @param path                              Modules are added to this path

    """

    if mode != 'Sampler' and mode != 'Teacher' and mode != 'Expert':
        B2FATAL('flavorTagger: Wrong mode given: The available modes are "Sampler", "Teacher" or "Expert"')

    if len(categories) > 13 or len(categories) < 2:
        B2FATAL('Flavor Tagger: Invalid amount of categories. At least two are needed. No more than 13 are available')
        B2FATAL(
            'Flavor Tagger: Possible categories are  "Electron", "IntermediateElectron", "Muon", "IntermediateMuon", '
            '"KinLepton", "IntermediateKinLepton", "Kaon", "SlowPion", "FastHadron",'
            '"Lambda", "FSC", "MaximumPstar" or "KaonPion" ')

    for category in categories:
        if category not in AvailableCategories:
            B2FATAL('Flavor Tagger: ' + category + ' is not a valid category name given')
            B2FATAL('Flavor Tagger: Available categories are  "Electron", "IntermediateElectron", '
                    '"Muon", "IntermediateMuon", "KinLepton", "IntermediateKinLepton", "Kaon", "SlowPion", "FastHadron", '
                    '"Lambda", "FSC", "MaximumPstar" or "KaonPion" ')

    # Directory where the weights of the trained Methods are saved
    # workingDirectory = os.environ['BELLE2_LOCAL_DIR'] + '/analysis/data'

    basf2.find_file(workingDirectory)

    global filesDirectory
    filesDirectory = workingDirectory + '/FlavorTagging/TrainedMethods'

    if mode == 'Sampler' or (mode == 'Expert' and downloadFromDatabaseIfNotFound):
        if not basf2.find_file(workingDirectory + '/FlavorTagging', silent=True):
            os.mkdir(workingDirectory + '/FlavorTagging')
            os.mkdir(workingDirectory + '/FlavorTagging/TrainedMethods')
        elif not basf2.find_file(workingDirectory + '/FlavorTagging/TrainedMethods', silent=True):
            os.mkdir(workingDirectory + '/FlavorTagging/TrainedMethods')
        filesDirectory = workingDirectory + '/FlavorTagging/TrainedMethods'

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

    global useOnlyLocalFlag
    useOnlyLocalFlag = useOnlyLocalWeightFiles

    B2INFO('*** FLAVOR TAGGING ***')
    B2INFO(' ')
    B2INFO('    Working directory is: ' + filesDirectory)
    B2INFO(' ')

    setInteractionWithDatabase(downloadFromDatabaseIfNotFound, uploadToDatabaseAfterTraining)
    set_FlavorTagger_pid_aliases()
    setVariables()

    trackLevelParticleLists = []
    eventLevelParticleLists = []
    variablesCombinerLevel = []
    categoriesCombination = []
    categoriesCombinationCode = 'CatCode'

    for category in categories:
        ftCategory = AvailableCategories[category]

        track_tuple = (ftCategory.particleList, ftCategory.trackName)
        event_tuple = (ftCategory.particleList, ftCategory.eventName, ftCategory.variableName)

        if track_tuple not in trackLevelParticleLists and category != 'MaximumPstar':
            trackLevelParticleLists.append(track_tuple)

        if event_tuple not in eventLevelParticleLists:
            eventLevelParticleLists.append(event_tuple)
            variablesCombinerLevel.append(ftCategory.variableName)
            categoriesCombination.append(ftCategory.code)
        else:
            B2FATAL('Flavor Tagger: ' + category + ' has been already given')

    for code in sorted(categoriesCombination):
        categoriesCombinationCode = categoriesCombinationCode + '%02d' % code

    roe_path = basf2.create_path()
    deadEndPath = basf2.create_path()

    if mode == 'Sampler':
        # Events containing ROE without B-Meson (but not empty) are discarded for training
        ma.signalSideParticleListsFilter(
            particleLists,
            'nROE_Charged(' + maskName + ', 0) > 0 and abs(qrCombined) == 1',
            roe_path,
            deadEndPath)

        FillParticleLists(maskName, categories, roe_path)

        if eventLevel(mode, weightFiles, categories, roe_path):
            combinerLevel(mode, weightFiles, categories, variablesCombinerLevel, categoriesCombinationCode, roe_path)

        # Removes EventExtraInfos and ParticleExtraInfos of the EventParticleLists
        particleListsToRemoveExtraInfo = []
        for category in categories:
            particleList = AvailableCategories[category].particleList
            if particleList not in particleListsToRemoveExtraInfo:
                particleListsToRemoveExtraInfo.append(particleList)

        ma.removeExtraInfo(particleListsToRemoveExtraInfo, False, roe_path)

        path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

    elif mode == 'Expert':
        # If trigger returns 1 jump into empty path skipping further modules in roe_path
        ma.signalSideParticleListsFilter(particleLists, 'nROE_Charged(' + maskName + ', 0) > 0', roe_path, deadEndPath)

        # Initialization of flavorTaggerInfo dataObject needs to be done in the main path
        flavorTaggerInfoBuilder = basf2.register_module('FlavorTaggerInfoBuilder')
        path.add_module(flavorTaggerInfoBuilder)

        FillParticleLists(maskName, categories, roe_path)

        if eventLevel(mode, weightFiles, categories, roe_path):
            combinerLevel(mode, weightFiles, categories, variablesCombinerLevel, categoriesCombinationCode, roe_path)

            flavorTaggerInfoFiller = basf2.register_module('FlavorTaggerInfoFiller')
            flavorTaggerInfoFiller.param('trackLevelParticleLists', trackLevelParticleLists)
            flavorTaggerInfoFiller.param('eventLevelParticleLists', eventLevelParticleLists)
            flavorTaggerInfoFiller.param('TMVAfbdt', TMVAfbdt)
            flavorTaggerInfoFiller.param('FANNmlp', FANNmlp)
            flavorTaggerInfoFiller.param('qpCategories', saveCategoriesInfo)
            flavorTaggerInfoFiller.param('istrueCategories', saveCategoriesInfo)
            flavorTaggerInfoFiller.param('targetProb', False)
            flavorTaggerInfoFiller.param('trackPointers', False)
            roe_path.add_module(flavorTaggerInfoFiller)  # Add FlavorTag Info filler to roe_path
            add_default_FlavorTagger_aliases()

        # Removes EventExtraInfos and ParticleExtraInfos of the EventParticleLists
        particleListsToRemoveExtraInfo = []
        for category in categories:
            particleList = AvailableCategories[category].particleList
            if particleList not in particleListsToRemoveExtraInfo:
                particleListsToRemoveExtraInfo.append(particleList)

        ma.removeExtraInfo(particleListsToRemoveExtraInfo, True, roe_path)

        path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

    elif mode == 'Teacher':
        if eventLevelTeacher(weightFiles, categories):
            combinerLevelTeacher(weightFiles, variableCombinerLevel, categoriesCombinationCode)


if __name__ == '__main__':

    desc_list = []

    function = globals()["flavorTagger"]
    signature = inspect.formatargspec(*inspect.getfullargspec(function))
    desc_list.append((function.__name__, signature + '\n' + function.__doc__))

    from terminal_utils import Pager
    from basf2.utils import pretty_print_description_list
    with Pager('Flavor Tagger function accepts the following arguments:'):
        pretty_print_description_list(desc_list)
