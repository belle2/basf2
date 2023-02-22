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

from basf2 import B2INFO, B2FATAL, B2WARNING
import basf2
import basf2_mva
import inspect
import modularAnalysis as ma
import variables
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
flavor_tagging = ['FBDT_qrCombined', 'FANN_qrCombined', 'qrMC', 'mcFlavorOfOtherB', 'qrGNN',
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

    variables.variables.addAlias('FBDT_qrCombined', 'qrOutput(FBDT)')
    variables.variables.addAlias('FANN_qrCombined', 'qrOutput(FANN)')
    variables.variables.addAlias('qrMC', 'isRelatedRestOfEventB0Flavor')

    variables.variables.addAlias('qrGNN', 'extraInfo(qrGNN)')

    for iCategory in AvailableCategories:
        aliasForQp = 'qp' + iCategory
        aliasForTrueTarget = 'hasTrueTarget' + iCategory
        aliasForIsRightCategory = 'isRightCategory' + iCategory
        variables.variables.addAlias(aliasForQp, 'qpCategory(' + iCategory + ')')
        variables.variables.addAlias(aliasForTrueTarget, 'hasTrueTargets(' + iCategory + ')')
        variables.variables.addAlias(aliasForIsRightCategory, 'isTrueFTCategory(' + iCategory + ')')

    utils.add_collection(flavor_tagging, 'flavor_tagging')


def set_FlavorTagger_pid_aliases():
    """
    This function adds the pid aliases needed by the flavor tagger.
    """
    variables.variables.addAlias('eid_TOP', 'pidPairProbabilityExpert(11, 211, TOP)')
    variables.variables.addAlias('eid_ARICH', 'pidPairProbabilityExpert(11, 211, ARICH)')
    variables.variables.addAlias('eid_ECL', 'pidPairProbabilityExpert(11, 211, ECL)')

    variables.variables.addAlias('muid_TOP', 'pidPairProbabilityExpert(13, 211, TOP)')
    variables.variables.addAlias('muid_ARICH', 'pidPairProbabilityExpert(13, 211, ARICH)')
    variables.variables.addAlias('muid_KLM', 'pidPairProbabilityExpert(13, 211, KLM)')

    variables.variables.addAlias('piid_TOP', 'pidPairProbabilityExpert(211, 321, TOP)')
    variables.variables.addAlias('piid_ARICH', 'pidPairProbabilityExpert(211, 321, ARICH)')

    variables.variables.addAlias('Kid_TOP', 'pidPairProbabilityExpert(321, 211, TOP)')
    variables.variables.addAlias('Kid_ARICH', 'pidPairProbabilityExpert(321, 211, ARICH)')

    if getBelleOrBelle2() == "Belle":
        variables.variables.addAlias('eid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, CDC, SVD), 0.5)')
        variables.variables.addAlias('muid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, CDC, SVD), 0.5)')
        variables.variables.addAlias('piid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(211, 321, CDC, SVD), 0.5)')
        variables.variables.addAlias('pi_vs_edEdxid', 'ifNANgiveX(pidPairProbabilityExpert(211, 11, CDC, SVD), 0.5)')
        variables.variables.addAlias('Kid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(321, 211, CDC, SVD), 0.5)')
    else:
        variables.variables.addAlias('eid_dEdx', 'pidPairProbabilityExpert(11, 211, CDC)')
        variables.variables.addAlias('muid_dEdx', 'pidPairProbabilityExpert(13, 211, CDC)')
        variables.variables.addAlias('piid_dEdx', 'pidPairProbabilityExpert(211, 321, CDC)')
        variables.variables.addAlias('pi_vs_edEdxid', 'pidPairProbabilityExpert(211, 11, CDC)')
        variables.variables.addAlias('Kid_dEdx', 'pidPairProbabilityExpert(321, 211, CDC)')


def set_FlavorTagger_pid_aliases_legacy():
    """
    This function adds the pid aliases needed by the flavor tagger trained for MC13.
    """
    variables.variables.addAlias('eid_TOP', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, TOP), 0.5)')
    variables.variables.addAlias('eid_ARICH', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, ARICH), 0.5)')
    variables.variables.addAlias('eid_ECL', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, ECL), 0.5)')

    variables.variables.addAlias('muid_TOP', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, TOP), 0.5)')
    variables.variables.addAlias('muid_ARICH', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, ARICH), 0.5)')
    variables.variables.addAlias('muid_KLM', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, KLM), 0.5)')

    variables.variables.addAlias('piid_TOP', 'ifNANgiveX(pidPairProbabilityExpert(211, 321, TOP), 0.5)')
    variables.variables.addAlias('piid_ARICH', 'ifNANgiveX(pidPairProbabilityExpert(211, 321, ARICH), 0.5)')

    variables.variables.addAlias('Kid_TOP', 'ifNANgiveX(pidPairProbabilityExpert(321, 211, TOP), 0.5)')
    variables.variables.addAlias('Kid_ARICH', 'ifNANgiveX(pidPairProbabilityExpert(321, 211, ARICH), 0.5)')

    if getBelleOrBelle2() == "Belle":
        variables.variables.addAlias('eid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, CDC, SVD), 0.5)')
        variables.variables.addAlias('muid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, CDC, SVD), 0.5)')
        variables.variables.addAlias('piid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(211, 321, CDC, SVD), 0.5)')
        variables.variables.addAlias('pi_vs_edEdxid', 'ifNANgiveX(pidPairProbabilityExpert(211, 11, CDC, SVD), 0.5)')
        variables.variables.addAlias('Kid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(321, 211, CDC, SVD), 0.5)')
    else:
        # Removed SVD PID for Belle II MC and data as it is absent in release 4.
        variables.variables.addAlias('eid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(11, 211, CDC), 0.5)')
        variables.variables.addAlias('muid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(13, 211, CDC), 0.5)')
        variables.variables.addAlias('piid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(211, 321, CDC), 0.5)')
        variables.variables.addAlias('pi_vs_edEdxid', 'ifNANgiveX(pidPairProbabilityExpert(211, 11, CDC), 0.5)')
        variables.variables.addAlias('Kid_dEdx', 'ifNANgiveX(pidPairProbabilityExpert(321, 211, CDC), 0.5)')


def set_GNNFlavorTagger_aliases(categories):
    """
    This function adds aliases for the GNN-based flavor tagger.
    """

    # will be used for target variable 0:B0bar, 1:B0
    variables.variables.addAlias('qrCombined_bit', '(qrCombined+1)/2')
    alias_list = ['qrCombined_bit']

    var_dict = {
        # position
        'dx': 'dx',
        'dy': 'dy',
        'dz': 'dz',
        # mask
        'E': 'E',
        # charge,
        'charge': 'charge',
        # feature
        'px_c': 'px*charge',
        'py_c': 'py*charge',
        'pz_c': 'pz*charge',
        'electronID_c': 'electronID*charge',
        'muonID_c': 'muonID*charge',
        'pionID_c': 'pionID*charge',
        'kaonID_c': 'kaonID*charge',
        'protonID_c': 'protonID*charge',
        'deuteronID_c': 'deuteronID*charge',
        'electronID_noSVD_noTOP_c': 'electronID_noSVD_noTOP*charge',
    }

    # 16 charged particles are used at most
    for rank in range(1, 17):

        for cat in categories:
            listName = AvailableCategories[cat].particleList
            varName = f'QpTrack({listName}, isRightCategory({cat}), isRightCategory({cat}))'

            varWithRank = f'ifNANgiveX(getVariableByRank(pi+:inRoe, p, {varName}, {rank}), 0)'
            aliasWithRank = f'{cat}_rank{rank}'

            variables.variables.addAlias(aliasWithRank, varWithRank)
            alias_list.append(aliasWithRank)

        for alias, var in var_dict.items():
            varWithRank = f'ifNANgiveX(getVariableByRank(pi+:inRoe, p, {var}, {rank}), 0)'
            aliasWithRank = f'{alias}_rank{rank}'

            variables.variables.addAlias(aliasWithRank, varWithRank)
            alias_list.append(aliasWithRank)

    return alias_list


def setInputVariablesWithMask(maskName='all'):
    """
    Set aliases for input variables with ROE mask.
    """
    variables.variables.addAlias('pMissTag_withMask', 'pMissTag('+maskName+')')
    variables.variables.addAlias('cosTPTO_withMask', 'cosTPTO('+maskName+')')
    variables.variables.addAlias('ptTracksRoe_withMask', 'ptTracksRoe('+maskName+')')
    variables.variables.addAlias('pt2TracksRoe_withMask', 'pt2TracksRoe('+maskName+')')
    variables.variables.addAlias('ptTracksRoe_withMask', 'ptTracksRoe('+maskName+')')


def getFastBDTCategories():
    '''
    Helper function for getting the FastBDT categories.
    It's necessary for removing top-level ROOT imports.
    '''
    fastBDTCategories = basf2_mva.FastBDTOptions()
    fastBDTCategories.m_nTrees = 500
    fastBDTCategories.m_nCuts = 8
    fastBDTCategories.m_nLevels = 3
    fastBDTCategories.m_shrinkage = 0.10
    fastBDTCategories.m_randRatio = 0.5
    return fastBDTCategories


def getFastBDTCombiner():
    '''
    Helper function for getting the FastBDT combiner.
    It's necessary for removing top-level ROOT imports.
    '''
    fastBDTCombiner = basf2_mva.FastBDTOptions()
    fastBDTCombiner.m_nTrees = 500
    fastBDTCombiner.m_nCuts = 8
    fastBDTCombiner.m_nLevels = 3
    fastBDTCombiner.m_shrinkage = 0.10
    fastBDTCombiner.m_randRatio = 0.5
    return fastBDTCombiner


def getMlpFANNCombiner():
    '''
    Helper function for getting the MLP FANN combiner.
    It's necessary for removing top-level ROOT imports.
    '''
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
    return mlpFANNCombiner


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


def getTrainingVariables(category=None):
    """
    Helper function to get training variables.

    NOTE: This function is not called the Expert mode. It is not necessary to be consistent with variables list of weight files.
    """

    KId = {'Belle': 'ifNANgiveX(atcPIDBelle(3,2), 0.5)', 'Belle2': 'kaonID'}
    muId = {'Belle': 'muIDBelle', 'Belle2': 'muonID'}
    eId = {'Belle': 'eIDBelle', 'Belle2': 'electronID'}

    variables = []
    if category == 'Electron' or category == 'IntermediateElectron':
        variables = ['useCMSFrame(p)',
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
        if getBelleOrBelle2() == "Belle":
            variables.append('eid_dEdx')
            variables.append('ImpactXY')
            variables.append('distance')

    elif category == 'Muon' or category == 'IntermediateMuon':
        variables = ['useCMSFrame(p)',
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
        if getBelleOrBelle2() == "Belle":
            variables.append('muid_dEdx')
            variables.append('ImpactXY')
            variables.append('distance')
            variables.append('chiProb')

    elif category == 'KinLepton' or category == 'IntermediateKinLepton':
        variables = ['useCMSFrame(p)',
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
        if getBelleOrBelle2() == "Belle":
            variables.append('eid_dEdx')
            variables.append('muid_dEdx')
            variables.append('ImpactXY')
            variables.append('distance')
            variables.append('chiProb')

    elif category == 'Kaon':
        variables = ['useCMSFrame(p)',
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
        if getBelleOrBelle2() == "Belle":
            variables.append('ImpactXY')
            variables.append('distance')

    elif category == 'SlowPion':
        variables = ['useCMSFrame(p)',
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
        if getBelleOrBelle2() == "Belle":
            variables.append('piid_dEdx')
            variables.append('ImpactXY')
            variables.append('distance')
            variables.append('chiProb')

    elif category == 'FastHadron':
        variables = ['useCMSFrame(p)',
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
        if getBelleOrBelle2() == "Belle":
            variables.append('BtagToWBosonVariables(pMissCMS)')
            variables.append('ImpactXY')
            variables.append('distance')
            variables.append('chiProb')

    elif category == 'Lambda':
        variables = ['lambdaFlavor',
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
        if getBelleOrBelle2() == "Belle2":
            variables.append('daughter(1,protonID)')  # protonID always 0 in B2BII check in future
            variables.append('daughter(0,pionID)')  # not very powerful in B2BII
        else:
            variables.append('distance')

    elif category == 'MaximumPstar':
        variables = ['useCMSFrame(p)',
                     'useCMSFrame(pt)',
                     'p',
                     'pt',
                     'cosTPTO',
                     ]
        if getBelleOrBelle2() == "Belle2":
            variables.append('ImpactXY')
            variables.append('distance')

    elif category == 'FSC':
        variables = ['useCMSFrame(p)',
                     'cosTPTO',
                     KId[getBelleOrBelle2()],
                     'FSCVariables(pFastCMS)',
                     'FSCVariables(cosSlowFast)',
                     'FSCVariables(cosTPTOFast)',
                     'FSCVariables(SlowFastHaveOpositeCharges)',
                     ]
    elif category == 'KaonPion':
        variables = ['extraInfo(isRightCategory(Kaon))',
                     'HighestProbInCat(pi+:inRoe, isRightCategory(SlowPion))',
                     'KaonPionVariables(cosKaonPion)',
                     'KaonPionVariables(HaveOpositeCharges)',
                     KId[getBelleOrBelle2()]
                     ]

    return variables


def FillParticleLists(maskName='all', categories=None, path=None):
    """
    Fills the particle Lists for all categories.
    """

    from vertex import kFit
    readyParticleLists = []

    if categories is None:
        categories = []

    trackCut = 'isInRestOfEvent > 0.5 and passesROEMask(' + maskName + ') > 0.5 and p >= 0'

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
        if 'pi+:inRoe' not in readyParticleLists:
            ma.fillParticleList('pi+:inRoe', trackCut, path=path)
        ma.reconstructDecay('K_S0:inRoe -> pi+:inRoe pi-:inRoe', '0.40<=M<=0.60', False, path=path)
        kFit('K_S0:inRoe', 0.01, path=path)

    # Apply BDT-based LID
    if getBelleOrBelle2() == 'Belle2':
        default_list_for_lid_BDT = ['e+:inRoe', 'mu+:inRoe']
        list_for_lid_BDT = []

        for particleList in default_list_for_lid_BDT:
            if particleList in readyParticleLists:
                list_for_lid_BDT.append(particleList)

        if list_for_lid_BDT:  # empty check
            ma.applyChargedPidMVA(particleLists=list_for_lid_BDT, path=path,
                                  trainingMode=0,  # binary
                                  binaryHypoPDGCodes=(11, 211))  # e vs pi
            ma.applyChargedPidMVA(particleLists=list_for_lid_BDT, path=path,
                                  trainingMode=0,  # binary
                                  binaryHypoPDGCodes=(13, 211))  # mu vs pi
            ma.applyChargedPidMVA(particleLists=list_for_lid_BDT, path=path,
                                  trainingMode=1)  # Multiclass


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
                variablesToBeSaved = getTrainingVariables(category) + [targetVariable, 'ancestorHasWhichFlavor',
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
            trainingOptionsEventLevel.m_variables = basf2_mva.vector(*getTrainingVariables(category))
            trainingOptionsEventLevel.m_target_variable = targetVariable
            trainingOptionsEventLevel.m_max_events = maxEventsNumber

            basf2_mva.teacher(trainingOptionsEventLevel, getFastBDTCategories())

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
                        B2FATAL('Flavor Tagger: Weight file ' + identifierFBDT +
                                ' was not downloaded from Database. Please check the buildOrRevision name. Stopped')

            if useOnlyLocalFlag:
                if not os.path.isfile(identifierFBDT):
                    B2FATAL('flavorTagger: Combinerlevel FastBDT was not trained with this combination of categories.' +
                            ' Weight file ' + identifierFBDT + ' not found. Stopped')

            B2INFO('flavorTagger: Ready to be used with weightFile ' + methodPrefixCombinerLevel + 'FBDT' + '_1.root')

        if FANNmlp:
            identifierFANN = methodPrefixCombinerLevel + 'FANN'
            if downloadFlag or useOnlyLocalFlag:
                identifierFANN = filesDirectory + '/' + methodPrefixCombinerLevel + 'FANN' + '_1.root'

            if downloadFlag:
                if not os.path.isfile(identifierFANN):
                    basf2_mva.download(methodPrefixCombinerLevel + 'FANN', identifierFANN)
                    if not os.path.isfile(identifierFANN):
                        B2FATAL('Flavor Tagger: Weight file ' + identifierFANN +
                                ' was not downloaded from Database. Please check the buildOrRevision name. Stopped')
            if useOnlyLocalFlag:
                if not os.path.isfile(identifierFANN):
                    B2FATAL('flavorTagger: Combinerlevel FANNMLP was not trained with this combination of categories. ' +
                            ' Weight file ' + identifierFANN + ' not found. Stopped')

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

            basf2_mva.teacher(trainingOptionsCombinerLevel, getFastBDTCombiner())

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

            basf2_mva.teacher(trainingOptionsCombinerLevel, getMlpFANNCombiner())

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
    combinerMethods=['TMVA-FBDT'],
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
    maskName='FTDefaultMask',
    saveCategoriesInfo=True,
    useOnlyLocalWeightFiles=False,
    downloadFromDatabaseIfNotFound=False,
    uploadToDatabaseAfterTraining=False,
    samplerFileId='',
    prefix='MC15ri_light-2207-bengal_0',
    useGNN=False,
    identifierGNN='',
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
      @param combinerMethods                   MVAs for the combiner: ``TMVA-FBDT` (default).
                                               ``FANN-MLP`` is available only with ``prefix=''`` (MC13 weight files).
      @param categories                        Categories used for flavor tagging. By default all are used.
      @param maskName                          Gets ROE particles from a specified ROE mask.
                                               ``FTDefaultMask`` (default): tentative mask definition that will be created
                                               automatically. The definition is as follows:

                                               - Track (pion): thetaInCDCAcceptance and dr<1 and abs(dz)<3
                                               - ECL-cluster (gamma): thetaInCDCAcceptance and clusterNHits>1.5 and \
                                               [[clusterReg==1 and E>0.08] or [clusterReg==2 and E>0.03] or \
                                               [clusterReg==3 and E>0.06]] \
                                               (Same as gamma:pi0eff30_May2020 and gamma:pi0eff40_May2020)

                                               ``all``: all ROE particles are used.
                                               Or one can give any mask name defined before calling this function.
      @param saveCategoriesInfo                Sets to save information of individual categories.
      @param useOnlyLocalWeightFiles           [Expert] Uses only locally saved weight files.
      @param downloadFromDatabaseIfNotFound    [Expert] Weight files are downloaded from
                                               the conditions database if not available in workingDirectory.
      @param uploadToDatabaseAfterTraining     [Expert] For librarians only: uploads weight files to localdb after training.
      @param samplerFileId                     Identifier to paralellize
                                               sampling. Only used in ``Sampler`` mode.  If you are training by yourself and
                                               want to parallelize the sampling, you can run several sampling scripts in
                                               parallel. By changing this parameter you will not overwrite an older sample.
      @param prefix                            Prefix of weight files.
                                               ``MC15ri_light-2207-bengal_0`` (default): Weight files trained for MC15ri samples.
                                               ``''``: Weight files trained for MC13 samples.
      @param useGNN                            Use GNN-based Flavor Tagger in addition with FastBDT-based one.
                                               Please specify the weight file with the option ``identifierGNN``.
                                               [Expert] In the sampler mode, training files for GNN-based Flavor Tagger is produced.
      @param identifierGNN                     The name of weight file of the GNN-based Flavor Tagger.
      @param path                              Modules are added to this path

    """

    if (not isinstance(particleLists, list)):
        particleLists = [particleLists]  # in case user inputs a particle list as string

    if mode != 'Sampler' and mode != 'Teacher' and mode != 'Expert':
        B2FATAL('flavorTagger: Wrong mode given: The available modes are "Sampler", "Teacher" or "Expert"')

    if len(categories) != len(set(categories)):
        dup = [cat for cat in set(categories) if categories.count(cat) > 1]
        B2WARNING('Flavor Tagger: There are duplicate elements in the given categories list. '
                  << 'The following duplicate elements are removed; ' << ', '.join(dup))
        categories = list(set(categories))

    if len(categories) < 2:
        B2FATAL('Flavor Tagger: Invalid amount of categories. At least two are needed.')
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

    if mode == 'Expert' and useGNN and identifierGNN == '':
        B2FATAL('The weight file of GNN-based Flavor Tagger is not set as default yet. '
                'Please specify the name of the weight file with ``identifierGNN``')

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

    if prefix == '':
        set_FlavorTagger_pid_aliases_legacy()
    else:
        set_FlavorTagger_pid_aliases()

    alias_list_for_GNN = []
    if useGNN:
        alias_list_for_GNN = set_GNNFlavorTagger_aliases(categories)

    setInputVariablesWithMask()
    if prefix != '':
        weightFiles = prefix + '_' + weightFiles

    # Create configuration lists and code-name for given category's list
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

    # Create default ROE-mask
    if maskName == 'FTDefaultMask':
        FTDefaultMask = (
            'FTDefaultMask',
            'thetaInCDCAcceptance and dr<1 and abs(dz)<3',
            'thetaInCDCAcceptance and clusterNHits>1.5 and [[E>0.08 and clusterReg==1] or [E>0.03 and clusterReg==2] or \
                            [E>0.06 and clusterReg==3]]')
        for name in particleLists:
            ma.appendROEMasks(list_name=name, mask_tuples=[FTDefaultMask], path=path)

    # Start ROE-routine
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

        if useGNN:
            if eventLevel('Expert', weightFiles, categories, roe_path):

                ma.rankByHighest('pi+:inRoe', 'p', numBest=0, allowMultiRank=False, path=roe_path)
                ma.fillParticleListFromDummy('vpho:dummy', path=roe_path)
                ma.variablesToNtuple('vpho:dummy',
                                     alias_list_for_GNN,
                                     treename='tree',
                                     filename=f'{filesDirectory}/FlavorTagger_GNN_sampled{fileId}.root',
                                     signalSideParticleList=particleLists[0],
                                     path=roe_path)

        else:
            if eventLevel(mode, weightFiles, categories, roe_path):
                combinerLevel(mode, weightFiles, categories, variablesCombinerLevel, categoriesCombinationCode, roe_path)

        path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

    elif mode == 'Expert':
        # If trigger returns 1 jump into empty path skipping further modules in roe_path
        # run filter with no cut first to get rid of ROEs that are missing the mask of the signal particle
        ma.signalSideParticleListsFilter(particleLists, '', roe_path, deadEndPath)
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

            if useGNN:
                ma.rankByHighest('pi+:inRoe', 'p', numBest=0, allowMultiRank=False, path=roe_path)
                ma.fillParticleListFromDummy('vpho:dummy', path=roe_path)
                roe_path.add_module('MVAExpert',
                                    listNames='vpho:dummy',
                                    extraInfoName='qrGNN_raw',  # the range of qrGNN_raw is [0,1]
                                    identifier=identifierGNN)

                ma.variableToSignalSideExtraInfo('vpho:dummy', {'extraInfo(qrGNN_raw)*2-1': 'qrGNN'},
                                                 path=roe_path)

        path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

    elif mode == 'Teacher':
        if eventLevelTeacher(weightFiles, categories):
            combinerLevelTeacher(weightFiles, variablesCombinerLevel, categoriesCombinationCode)


if __name__ == '__main__':

    desc_list = []

    function = globals()["flavorTagger"]
    signature = inspect.formatargspec(*inspect.getfullargspec(function))
    desc_list.append((function.__name__, signature + '\n' + function.__doc__))

    from terminal_utils import Pager
    from basf2.utils import pretty_print_description_list
    with Pager('Flavor Tagger function accepts the following arguments:'):
        pretty_print_description_list(desc_list)
