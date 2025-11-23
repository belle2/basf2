##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import modularAnalysis as ma
import variables
from variables import utils
from basf2 import B2INFO, B2ERROR
import basf2_mva
# import torch as torch
# Check if GPU is available
# device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
# print("Device is:",device)


def add_default_lambda_Selector_aliases():
    """
    This function is used to set up variables aliases for lambda Selector variables.
    """
    variables.variables.addAlias('daughtersDeltaZ', 'daughterDiffOf(0, 1, dz)')
    variables.variables.addAlias('cosVertexMomentum', 'cosAngleBetweenMomentumAndVertexVector')
    variables.variables.addAlias('pro_nPXDHits', 'daughter(0,nPXDHits)')
    variables.variables.addAlias('pin_nPXDHits', 'daughter(1,nPXDHits)')
    variables.variables.addAlias('pro_nSVDHits', 'daughter(0,nSVDHits)')
    variables.variables.addAlias('pin_nSVDHits', 'daughter(1,nSVDHits)')
    variables.variables.addAlias('daughterAngleDiffInMother', 'useRestFrame(daughterAngle(0, 1))')
    variables.variables.addAlias('pro_p', 'daughter(0,p)')
    variables.variables.addAlias('pin_p', 'daughter(1,p)')
    variables.variables.addAlias('pro_pt', 'daughter(0,pt)')
    variables.variables.addAlias('pin_pt', 'daughter(1,pt)')
    variables.variables.addAlias('pro_dr', 'daughter(0,dr)')
    variables.variables.addAlias('pin_dr', 'daughter(1,dr)')
    variables.variables.addAlias('pro_cosTheta', 'daughter(0,cosTheta)')
    variables.variables.addAlias('pin_cosTheta', 'daughter(1,cosTheta)')
    variables.variables.addAlias('pro_protonID', 'daughter(0,protonID)')
    variables.variables.addAlias('pin_protonID', 'daughter(1,protonID)')
    variables.variables.addAlias('proPDG', 'daughter(0,mcPDG)')
    variables.variables.addAlias('pinPDG', 'daughter(1,mcPDG)')
    variables.variables.addAlias("Mks_Hypo", "useAlternativeDaughterHypothesis(M, 0:pi+)")


def add_variable_collection():
    """
    Call this function to add variable collection for lambdaSelector.
    """
    add_default_lambda_Selector_aliases()
    inputVariablesList = [
        'cosVertexMomentum',
        'flightDistance',
        'flightDistanceErr',
        'significanceOfDistance',
        'cosHelicityAngleMomentum',
        'ImpactXY',
        'decayAngle(0)',
        'decayAngle(1)',
        'daughterAngleDiffInMother',
        'daughtersDeltaZ',
        'pro_nSVDHits', 'pro_nPXDHits',
        'pin_nSVDHits', 'pin_nPXDHits',
        'pro_dr', 'pin_dr',
        'pro_protonID', 'pin_protonID',
        'pro_p', 'pin_p',
        'pro_pt', 'pin_pt',
        'pro_cosTheta', 'pin_cosTheta',
        'pinPDG', 'proPDG',
        'ArmenterosDaughter1Qt',
        'ArmenterosDaughter2Qt',
        'Mks_Hypo'
    ]
    utils.add_collection(inputVariablesList, 'lambda_selector_info')


def V0Selector_Training(
        train_data,
        tree_name="tree",
        mva_identifier="MVA_LightGBM_V0Selector.root",
        target_variable="isSignal",
        parameters={},
        options={}
        ):
    """
    Defines the configuration of V0Selector Training.
    The training data should contain Lambda0 and misreconstructed Lambda0 without KS0.

    @param train_data                   Root file containing Lambda information to be trained.
    @param tree_name                    Tree name for variables.
    @param mva_identifier               Name for output MVA weight file.
    @param target_variable              Target variable for MVA training.
    @param parameters                   hyperparameter for LGBM
    @param options                      MVA options
    """

    # Create The GeneralOptions object as always
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
        'pro_nSVDHits', 'pro_nPXDHits',
        'pin_nSVDHits', 'pin_nPXDHits',
        'pro_dr', 'pin_dr',
    ]

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector(train_data)
    general_options.m_treename = tree_name
    general_options.m_identifier = mva_identifier
    general_options.m_variables = basf2_mva.vector(*trainVars)
    general_options.m_target_variable = target_variable
    general_options.m_max_events = 0 if 'max_events' not in options else options['max_events']

    python_options = basf2_mva.PythonOptions()
    python_options.m_framework = "lightgbm"

    import json
    param = {'num_leaves': 256,
             'objective': 'cross_entropy',
             'learning_rate': 0.1,
             'device_type': "cpu",
             'deterministic': True,
             'metric': 'auc',
             'num_round': 100,
             # 'stop_round' : 30, mute this in case too early stop
             'path': mva_identifier+'.txt',
             'min_data_in_leaf': 1000,
             'max_depth': 8,
             'max_bin': 250,
             'boosting': 'gbdt',
             'trainFraction': 0.8,
             'num_threads': 1
             }
    if isinstance(parameters, dict):
        param.update(parameters)
    config_string = json.dumps(param)
    print("The json config string", config_string)
    python_options.m_config = config_string

    python_options.m_training_fraction = 1
    python_options.m_normalize = False
    python_options.m_nIterations = 1
    python_options.m_mini_batch_size = 0

    basf2_mva.teacher(general_options, python_options)


def KsVeto_Training(
    train_data,
    tree_name="tree",
    mva_identifier="MVA_LightGBM_KsVeto.root",
    target_variable="isSignal",
    parameters={},
    options={}
):
    """
    Defines the configuration of KsVeto Training.
    The training data should contain only Lambda and Ks.

    @param train_data                   Root file containing Ks information to be trained.
    @param tree_name                    Tree name for variables.
    @param mva_identifier               Name for output MVA weight file.
    @param target_variable              Target variable for MVA training.
    @param parameters                   hyperparameter for LGBM
    @param options                      MVA options
    """

    # Create The GeneralOptions object as always
    trainVars = [
        'pro_protonID',
        'pin_protonID',
        'ArmenterosDaughter1Qt',
        'ArmenterosDaughter2Qt',
        'pro_cosTheta',
        'pin_cosTheta',
        'Mks_Hypo',
        'ArmenterosLongitudinalMomentumAsymmetry',
    ]

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector(train_data)
    general_options.m_treename = tree_name
    general_options.m_identifier = mva_identifier
    general_options.m_variables = basf2_mva.vector(*trainVars)
    general_options.m_target_variable = target_variable
    general_options.m_max_events = 0 if 'max_events' not in options else options['max_events']

    python_options = basf2_mva.PythonOptions()
    python_options.m_framework = "lightgbm"

    import json
    param = {'num_leaves': 256,
             'learning_rate': 0.05,
             'device_type': "cpu",
             'deterministic': True,
             'metric': 'auc',
             'num_round': 100,
             # 'stop_round' : 10,
             'path': mva_identifier+'.txt',
             'max_bin': 250,
             'boosting': 'gbdt',
             'max_depth': 8,
             'trainFraction': 0.8,
             'min_data_in_leaf': 300,
             'objective': 'cross_entropy',
             'num_threads': 1
             }
    if isinstance(parameters, dict):
        param.update(parameters)
    config_string = json.dumps(param)
    print("The json config string", config_string)
    python_options.m_config = config_string

    python_options.m_normalize = False  # we do it inside MVA torch
    python_options.m_nIterations = 1
    python_options.m_mini_batch_size = 0

    basf2_mva.teacher(general_options, python_options)

# ****************************************
# Lambda Selector MAIN FUNCTION
# ****************************************


def LambdaSelector(
    particleListName,
    identifier_Lambda="Lambda_LGBM_V0Selector",
    identifier_vKs="Lambda_LGBM_KsVeto",
    output_label_name='',
    extraInfoName_V0Selector='LambdaSelector_V0Selector',
    extraInfoName_KsVeto='LambdaSelector_KsVeto',
    useCustomThreshold=False,
    threshold_V0Selector=0.72,
    threshold_KsVeto=0.27,
    path=None
):
    """
    This function will apply Lambda0 selection MVA on the given particleList.
    By default this function appends MVA output as a extraInfo for the given particleList.
    You can apply preset cut or custom cut by giving parameters. In this case,
    a new particleList is created from the original particleList applying cuts on the MVA output.

    @param particleListName             Reconstructed Lambda0 -> p+ pi- list.
    @param output_label_name            Label of the returned Lambda particleList.
                                        When empty '', no cut is applied and new particleList is not created.
                                        When custom name, the custom threshold is used, and useCustomThreshold
                                        must be True.
                                        When 'standard', 'tight', or 'loose', a cut with Ks efficiency
                                        90%, 95%, and 85% is applied.
    @param extraInfoName_V0Selector     Variable name for V0Selector MVA output.
    @param extraInfoName_KsVeto         Variable name for KsVeto MVA output.
    @param identifier_Lambda            Identifier name for V0Selector weight file.
    @param identifier_vKs               Identifier name for KsVeto weight file.
    @param useCustomThreshold           Flag whether threshold_V0Selector and threshold_KsVeto are used.
    @param threshold_V0Selector         Threshold for V0Selector.
    @param threshold_KsVeto             Threshold for KsVeto.
    @param path                         Basf2 path to execute.

    """

    add_default_lambda_Selector_aliases()

    path.add_module('MVAMultipleExperts',
                    listNames=[particleListName],
                    extraInfoNames=[extraInfoName_V0Selector, extraInfoName_KsVeto],
                    identifiers=[identifier_Lambda, identifier_vKs])

    _effnames = ['standard', 'tight', 'loose']
    outputListName = ''

    if useCustomThreshold:
        if output_label_name in _effnames:
            B2ERROR('LambdaSelector: Specify label name except for \'standard\', \'tight\', and \'loose\' '
                    'when you use custom threshold.')
        elif output_label_name == '':
            B2ERROR('LambdaSelector: Specify label name when you use custom threshold.')
        else:
            outputListName = particleListName.split(':')[0] + ':' + output_label_name
            B2INFO('LambdaSelector: Custom Cut is applied on '+outputListName+'.')
            V0_thr = threshold_V0Selector
            Ks_thr = threshold_KsVeto
            B2INFO('LambdaSelector: Threshold is (' + str(V0_thr) + ', ' + str(Ks_thr) + ')')
            cut_string = 'extraInfo('+extraInfoName_V0Selector+')>'+str(V0_thr) + \
                ' and extraInfo('+extraInfoName_KsVeto+')>'+str(Ks_thr)
            ma.cutAndCopyLists(outputListName, particleListName, cut=cut_string, path=path)
    else:
        if output_label_name in _effnames:
            outputListName = particleListName.split(':')[0] + ':' + output_label_name
            V0_thr = 0
            Ks_thr = 0
            if output_label_name == 'standard':
                B2INFO('LambdaSelector: Standard Cut is applied on '+outputListName+'.')
                V0_thr = 0.72
                Ks_thr = 0.27
            elif output_label_name == 'tight':
                B2INFO('LambdaSelector: Tight Cut is applied on '+outputListName+'.')
                V0_thr = 0.88
                Ks_thr = 0.63
            elif output_label_name == 'loose':
                B2INFO('LambdaSelector: Loose Cut is applied on '+outputListName+'.')
                V0_thr = 0.25
                Ks_thr = 0.11
            B2INFO('LambdaSelector: Threshold is (' + str(V0_thr) + ', ' + str(Ks_thr) + ')')
            cut_string = 'extraInfo('+extraInfoName_V0Selector+')>'+str(V0_thr) + \
                ' and extraInfo('+extraInfoName_KsVeto+')>'+str(Ks_thr)
            ma.cutAndCopyLists(outputListName, particleListName, cut=cut_string, path=path)
        elif output_label_name == '':
            outputListName = particleListName
        else:
            B2ERROR('LambdaSelector: Label should be \'\', \'standard\', \'tight\', or \'loose\' if you do'
                    'not apply custom threshold')

    B2INFO('LambdaSelector: ParticleList '+outputListName+' is returned.')
