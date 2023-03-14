#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import B2INFO, B2ERROR
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
    The training data should contain K_S0 and misreconstructed K_S0 without Lambda0.

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
    The training data should contain only K_S0 and Lambda0.

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
    output_label_name='',
    extraInfoName_V0Selector='KsSelector_V0Selector',
    extraInfoName_LambdaVeto='KsSelector_LambdaVeto',
    useCustomThreshold=False,
    threshold_V0Selector=0.90,
    threshold_LambdaVeto=0.11,
    path=None
):
    """
    This function will apply K_S0 selection MVA on the given particleList.
    By default this function appends MVA output as a extraInfo for the given particleList.
    You can apply preset cut or custom cut by giving parameters. In this case,
    a new particleList is created from the original particleList applying cuts on the MVA output.

    @param particleLists                Reconstructed Ks -> pi+ pi- list.
    @param output_label_name            Label of the returned Ks particleList.
                                        When empty '', no cut is applied and new particleList is not created.
                                        When custom name, the custom threshold is used, and useCustomThreshold
                                        must be True.
                                        When 'standard', 'tight', or 'loose', a cut with Ks efficiency
                                        90%, 95%, and 85% is applied.
    @param extraInfoName_V0Selector     Variable name for V0Selector MVA output.
    @param extraInfoName_LambdaVeto     Variable name for LambdaVeto MVA output.
    @param identifier_Ks                Identifier name for V0Selector weight file.
    @param identifier_vLambda           Identifier name for LambdaVeto weight file.
    @param useCustomThreshold           Flag whether threshold_V0Selector and threshold_LambdaVeto are used.
    @param threshold_V0Selector         Threshold for V0Selector.
    @param threshold_LambdaVeto         Threshold for LambdaVeto.
    @param path                         Basf2 path to execute.

    """

    add_default_ks_Selector_aliases()

    path.add_module('MVAMultipleExperts',
                    listNames=[particleListName],
                    extraInfoNames=[extraInfoName_V0Selector, extraInfoName_LambdaVeto],
                    identifiers=[identifier_Ks, identifier_vLambda])

    _effnames = ['standard', 'tight', 'loose']
    outputListName = ''

    if useCustomThreshold:
        if output_label_name in _effnames:
            B2ERROR('KsSelector: Specify label name except for \'standard\', \'tight\', and \'loose\' '
                    'when you use custom threshold.')
        elif output_label_name == '':
            B2ERROR('KsSelector: Specify label name when you use custom threshold.')
        else:
            outputListName = particleListName.split(':')[0] + ':' + output_label_name
            B2INFO('KsSelector: Custom Cut is applied on '+outputListName+'.')
            V0_thr = threshold_V0Selector
            Lambda_thr = threshold_LambdaVeto
            B2INFO('KsSelector: Threshold is (' + str(V0_thr) + ', ' + str(Lambda_thr) + ')')
            cut_string = 'extraInfo('+extraInfoName_V0Selector+')>'+str(V0_thr) + \
                ' and extraInfo('+extraInfoName_LambdaVeto+')>'+str(Lambda_thr)
            ma.cutAndCopyLists(outputListName, particleListName, cut=cut_string, path=path)
    else:
        if output_label_name in _effnames:
            outputListName = particleListName.split(':')[0] + ':' + output_label_name
            V0_thr = 0
            Lambda_thr = 0
            if output_label_name == 'standard':
                B2INFO('KsSelector: Standard Cut is applied on '+outputListName+'.')
                V0_thr = 0.90
                Lambda_thr = 0.11
            elif output_label_name == 'tight':
                B2INFO('KsSelector: Tight Cut is applied on '+outputListName+'.')
                V0_thr = 0.96
                Lambda_thr = 0.27
            elif output_label_name == 'loose':
                B2INFO('KsSelector: Loose Cut is applied on '+outputListName+'.')
                V0_thr = 0.49
                Lambda_thr = 0.02
            B2INFO('KsSelector: Threshold is (' + str(V0_thr) + ', ' + str(Lambda_thr) + ')')
            cut_string = 'extraInfo('+extraInfoName_V0Selector+')>'+str(V0_thr) + \
                ' and extraInfo('+extraInfoName_LambdaVeto+')>'+str(Lambda_thr)
            ma.cutAndCopyLists(outputListName, particleListName, cut=cut_string, path=path)
        elif output_label_name == '':
            outputListName = particleListName
        else:
            B2ERROR('KsSelector: Label should be \'\', \'standard\', \'tight\', or \'loose\' if you do'
                    'not apply custom threshold')

    B2INFO('KsSelector: ParticleList '+outputListName+' is returned.')
