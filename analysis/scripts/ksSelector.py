#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import B2INFO
import basf2_mva
import modularAnalysis as ma
import variables
from variables import utils


def add_default_ks_Selector_aliases():
    """
    This function is used to set up variables aliases for ks Selector variables.
    """
    B2INFO('KsSelector: creating variables alias.')
    variables.variables.addAlias('M_lambda_p', 'useAlternativeDaughterHypothesis(M, 0:p+)')
    variables.variables.addAlias('M_lambda_antip', 'useAlternativeDaughterHypothesis(M, 1:anti-p-)')
    variables.variables.addAlias('daughtersDeltaZ', 'daughterDiffOf(0, 1, dz)')
    variables.variables.addAlias('cosVertexMomentum', 'cosAngleBetweenMomentumAndVertexVector')
    variables.variables.addAlias('pip_nPXDHits', 'daughter(0,nPXDHits)')
    variables.variables.addAlias('pin_nPXDHits', 'daughter(1,nPXDHits)')
    variables.variables.addAlias('pip_nSVDHits', 'daughter(0,nSVDHits)')
    variables.variables.addAlias('pin_nSVDHits', 'daughter(1,nSVDHits)')
    variables.variables.addAlias('daughterAngleDiffInMother', 'useRestFrame(daughterAngle(0, 1))')
    variables.variables.addAlias('pip_p', 'daughter(0,p)')
    variables.variables.addAlias('pin_p', 'daughter(1,p)')
    variables.variables.addAlias('pip_dr', 'daughter(0,dr)')
    variables.variables.addAlias('pin_dr', 'daughter(1,dr)')
    variables.variables.addAlias('pip_cosTheta', 'daughter(0,cosTheta)')
    variables.variables.addAlias('pin_cosTheta', 'daughter(1,cosTheta)')
    variables.variables.addAlias('pip_protonID', 'daughter(0,protonID)')
    variables.variables.addAlias('pin_protonID', 'daughter(1,protonID)')


def add_variable_collection():
    """
    Call this function to add variable collection for ksSelector.
    """
    add_default_ks_Selector_aliases()
    inputVariablesList = [
        'cosVertexMomentum',
        'flightDistance',
        'significanceOfDistance',
        'cosHelicityAngleMomentum',
        'ImpactXY',
        'decayAngle(0)',
        'decayAngle(1)',
        'daughterAngleDiffInMother',
        'daughtersDeltaZ',
        'pip_nSVDHits', 'pip_nPXDHits',
        'pin_nSVDHits', 'pin_nPXDHits',
        'pip_dr', 'pin_dr',
        'pip_protonID', 'pin_protonID',
        'M_lambda_p', 'M_lambda_antip',
        'pip_p', 'pin_p',
        'pip_cosTheta', 'pin_cosTheta',
    ]
    utils.add_collection(inputVariablesList, 'ks_selector_info')


def V0Selector_Training(
    train_data,
    tree_name="tree",
    mva_identifier="MVAFastBDT_V0Selector.root",
    target_variable="isSignal"
):
    """
    Defines the configuration of V0Selector Training.
    The training data should contain K_S0 and misreconstructed K_S0 without lambda0.

    @param train_data                   Root file containing Ks information to be trained.
    @param tree_name                    Tree name for variables.
    @param mva_identifier               Name for output MVA weight file.
    @param target_variable              Target variable for MVA training.
    """
    trainVars = [
        'cosVertexMomentum',
        'flightDistance',
        'significanceOfDistance',
        'cosHelicityAngleMomentum',
        'ImpactXY',
        'decayAngle(0)',
        'decayAngle(1)',
        'daughterAngleDiffInMother',
        'daughtersDeltaZ',
        'pip_nSVDHits', 'pip_nPXDHits',
        'pin_nSVDHits', 'pin_nPXDHits',
        'pip_dr', 'pin_dr',
    ]

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector(train_data)
    general_options.m_treename = tree_name
    general_options.m_identifier = mva_identifier
    general_options.m_variables = basf2_mva.vector(*trainVars)
    general_options.m_target_variable = target_variable
    fastbdt_options = basf2_mva.FastBDTOptions()
    basf2_mva.teacher(general_options, fastbdt_options)


def LambdaVeto_Training(
    train_data,
    tree_name="tree",
    mva_identifier="MVAFastBDT_LambdaVeto.root",
    target_variable="isSignal"
):
    """
    Defines the configuration of LambdaVeto Training.
    The training data should contain only K_S0 and lambda0.

    @param train_data                   Root file containing Ks information to be trained.
    @param tree_name                    Tree name for variables.
    @param mva_identifier               Name for output MVA weight file.
    @param target_variable              Target variable for MVA training.
    """
    trainVars = [
        'pip_protonID',
        'pin_protonID',
        'M_lambda_p',
        'M_lambda_antip',
        'pip_p',
        'pin_p',
        'pip_cosTheta',
        'pin_cosTheta',
    ]
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector(train_data)
    general_options.m_treename = tree_name
    general_options.m_identifier = mva_identifier
    general_options.m_variables = basf2_mva.vector(*trainVars)
    general_options.m_target_variable = target_variable
    fastbdt_options = basf2_mva.FastBDTOptions()
    basf2_mva.teacher(general_options, fastbdt_options)

# ****************************************
# KS Selector MAIN FUNCTION
# ****************************************


def ksSelector(
    particleListName,
    identifier_Ks,
    identifier_vLambda,
    listtype='all',
    extraInfoName_V0Selector='KsSelector_V0Selector',
    extraInfoName_LambdaVeto='KsSelector_LambdaVeto',
    useThreshold=False,
    threshold_V0Selector=0.90,
    threshold_LambdaVeto=0.11,
    path=None
):
    """
    Defines the configuration of KsSelector process for the input particle list.

    @param particleLists                Reconstructed Ks -> pi+ pi- list.
    @param listtype                     Type of Ks cut. When 'all'(default), no cut is applied on Ks.
                                        When 'standard', 'tight', or 'loose', a cut with Ks efficiency
                                        90%, 95%, and 85% is applied.
    @param extraInfoName_V0Selector     Variable name for V0Selector MVA output.
    @param extraInfoName_LambdaVeto     Variable name for LambdaVeto MVA output.
    @param identifier_Ks                Identifier name for V0Selector weight file.
    @param identifier_vLambda           Identifier name for LambdaVeto weight file.
    @useThreshold                       Flag whether thresholds are specified from payload.
    @threshold_V0Selector               Threshold for V0Selector.
    @threshold_LambdaVeto               Threshold for LambdaVeto.
    @param path                         Basf2 path to execute.
    """
    add_default_ks_Selector_aliases()

    outputListName = particleListName.split(':')[0] + ':'
    if listtype == 'all':
        outputListName += particleListName.split(':')[1]
    else:
        if useThreshold:
            outputListName += 'cut'
        else:
            outputListName += listtype
        ma.copyList(outputListName, particleListName, path=path)

    path.add_module('MVAMultipleExperts',
                    listNames=[outputListName],
                    extraInfoNames=[extraInfoName_V0Selector, extraInfoName_LambdaVeto],
                    identifiers=[identifier_Ks, identifier_vLambda])

    _effnames = ['standard', 'tight', 'loose']
    if listtype not in _effnames and listtype != 'all':
        B2INFO('Invalid List type. '+outputListName+' is created.')
    elif listtype == 'all':
        B2INFO('KsSelector is applied to '+outputListName+'.')
    elif listtype in _effnames or useThreshold:
        V0_thr = 0
        Lambda_thr = 0
        if useThreshold:
            B2INFO('Cut is applied on '+outputListName+'.')
            V0_thr = threshold_V0Selector
            Lambda_thr = threshold_LambdaVeto
        elif listtype == 'standard':
            B2INFO('Standard Cut is applied on '+outputListName+'.')
            V0_thr = 0.90
            Lambda_thr = 0.11
        elif listtype == 'tight':
            B2INFO('Tight Cut is applied on '+outputListName+'.')
            V0_thr = 0.96
            Lambda_thr = 0.27
        elif listtype == 'loose':
            B2INFO('Loose Cut is applied on '+outputListName+'.')
            V0_thr = 0.49
            Lambda_thr = 0.02

        B2INFO('ParticleList '+outputListName+' is created.')
        B2INFO('Threshold is (' + str(V0_thr) + ', ' + str(Lambda_thr) + ')')
        cut_string = 'extraInfo('+extraInfoName_V0Selector+')>'+str(V0_thr) + \
            ' and extraInfo('+extraInfoName_LambdaVeto+')>'+str(Lambda_thr)
        ma.applyCuts(list_name=outputListName,
                     cut=cut_string,
                     path=path)
