#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import json
import os
import glob
import basf2_mva
from basf2 import B2FATAL
import basf2
from variables import variables as vm
import modularAnalysis as ma
from stdPhotons import stdPhotons
from vertex import kFit


def get_variables(particle_list, ranked_variable, variables=None, particleNumber=1):
    """ creates variable name pattern requested by the basf2 variable getVariableByRank()
    :param particle_list:
    :param ranked_variable:
    :param variables:
    :param particleNumber:
    :return:
    """
    var_list = []
    for i_num in range(1, particleNumber + 1):
        for var in variables:
            var_list.append('getVariableByRank(' + particle_list + ', ' + ranked_variable + ', ' + var + ', ' + str(i_num) + ')')
    return var_list


def fill_particle_lists(maskName='TFLATDefaultMask', path=None):
    """
    Fills the particle lists.
    """

    # create particle list with pions
    trk_cut = 'isInRestOfEvent > 0.5 and passesROEMask(' + maskName + ') > 0.5 and p >= 0'
    ma.fillParticleList('pi+:tflat', trk_cut, path=path)

    # create particle list with gammas
    stdPhotons(listtype='tight',  path=path)
    gamma_cut = 'isInRestOfEvent > 0.5 and passesROEMask(' + maskName + ') > 0.5'
    ma.cutAndCopyList('gamma:tflat', 'gamma:tight', gamma_cut, path=path)

    ma.reconstructDecay('K_S0:inRoe -> pi+:tflat pi-:tflat', '0.40<=M<=0.60', False, path=path)
    kFit('K_S0:inRoe', 0.01, path=path)


def flavorTagger(particleLists, mode='Expert', working_dir='', uniqueIdentifier='standard_tflat',
                 target='qrCombined', overwrite=False,
                 classifier_args=None,
                 train_valid_fraction=.92, mva_steering_file='analysis/scripts/tflat/tensorflow_tflat_interface.py',
                 maskName='TFLATDefaultMask',
                 sampler_id=0,
                 path=None):
    """
    Interfacing for the Transformer FlavorTagger (TFlat). This function can be used for training (``Teacher``), preparation of
    training datasets (``Sampler``) and inference (``Expert``).

    This function requires reconstructed B meson signal particle list and where an RestOfEvent is built.

    :param particleLists:  string or list[string], particle list(s) of the reconstructed signal B meson
    :param mode: string, valid modes are ``Expert`` (default), ``Teacher``, ``Sampler``
    :param working_dir: string, working directory for the method
    :param uniqueIdentifier: string, database identifier for the method
    :param target: string, target variable
    :param overwrite: bool, overwrite already (locally!) existing training
    :param classifier_args: dictionary, customized arguments for tflat
    :param train_valid_fraction: float, train-valid fraction (.92).
    :param maskName: get ROE particles from a specified ROE mask
    :param sampler_id: identifier of sampled file for parallel sampling
    :param path: basf2 path obj
    :return: None
    """

    if isinstance(particleLists, str):
        particleLists = [particleLists]

    if mode not in ['Expert', 'Teacher', 'Sampler']:
        B2FATAL(f'Invalid mode  {mode}')

    if mode in ['Sampler', 'Teacher']:
        trk_variable_list = [
            'charge',
            'useCMSFrame(p)',
            'useCMSFrame(cosTheta)',
            'useCMSFrame(phi)',
            'electronID',
            'muonID',
            'kaonID',
            'pionID',
            'protonID',
            # 'nCDCHits/56',
            'nPXDHits/2',
            'nSVDHits/8',
            'dz',
            'dr',
            # 'chiProb',
            # 'clusterEoP',
            # 'clusterLAT',
            'BtagToWBosonVariables(recoilMassSqrd,'+maskName+')/15',
            'BtagToWBosonVariables(pMissCMS,'+maskName+')',
            'BtagToWBosonVariables(cosThetaMissCMS,'+maskName+')',
            'BtagToWBosonVariables(EW90,'+maskName+')',
            'cosTPTO('+maskName+')',
        ]

        ecl_variable_list = [
            'useCMSFrame(p)',
            'useCMSFrame(cosTheta)',
            'useCMSFrame(phi)',
            'clusterE1E9',
            'clusterE9E21',
            'clusterLAT',
        ]

        roe_variable_list = [
            'countInList(gamma:tflat)/8',
            'countInList(pi+:tflat)/6',
            'NumberOfKShortsInRoe',
            'ptTracksRoe('+maskName+')',
        ]

    if classifier_args is None:
        classifier_args = {}
    else:
        assert isinstance(classifier_args, dict)

    tree_name = 'tflat_variables'
    rank_variable = 'p'

    # create default ROE-mask
    if maskName == 'TFLATDefaultMask':
        TFLATDefaultMask = (
            'TFLATDefaultMask',
            'thetaInCDCAcceptance and p<infinity and p >= 0 and dr<1 and abs(dz)<3',
            'thetaInCDCAcceptance and clusterNHits>1.5 and [[E>0.08 and clusterReg==1] or [E>0.06 and clusterReg==2] or \
                            [E>0.06 and clusterReg==3]]')
        for name in particleLists:
            ma.appendROEMasks(list_name=name, mask_tuples=[TFLATDefaultMask], path=path)

    # create tagging specific variables
    if mode != 'Expert':
        features = get_variables('pi+:tflat', rank_variable, trk_variable_list, particleNumber=10)
        features += get_variables('gamma:tflat', rank_variable, ecl_variable_list, particleNumber=20)
        features += get_variables('pi+:tflat', rank_variable, roe_variable_list, particleNumber=1)

    # create roe specific paths
    roe_path = basf2.create_path()
    dead_end_path = basf2.create_path()

    if mode == 'Sampler':
        output_file_name = os.path.join(working_dir, uniqueIdentifier + f'_training_data{sampler_id}.root')
        if os.path.isfile(output_file_name) and not overwrite:
            B2FATAL(f'Outputfile {output_file_name} already exists. Aborting writeout.')

        # filter rest of events only for specific particle list
        ma.signalSideParticleListsFilter(
            particleLists,
            'nROE_Charged(' + maskName + ', 0) > 0 and abs(qrCombined) == 1',
            roe_path,
            dead_end_path)

        fill_particle_lists(maskName, roe_path)

        ma.rankByHighest('pi+:tflat', rank_variable, path=roe_path)
        ma.rankByHighest('gamma:tflat', rank_variable, path=roe_path)

        vm.addAlias('refdx', 'getVariableByRank(pi+:tflat, p, dx, 1)')
        vm.addAlias('dxdiff', 'formula(dx-refdx)')
        vm.addAlias('refdy', 'getVariableByRank(pi+:tflat, p, dy, 1)')
        vm.addAlias('dydiff', 'formula(dy-refdy)')
        vm.addAlias('refdz', 'getVariableByRank(pi+:tflat, p, dz, 1)')
        vm.addAlias('dzdiff', 'formula(dz-refdz)')

        # and add target
        all_variables = features + [target]

        # write to ntuples
        ma.variablesToNtuple('', all_variables, tree_name, output_file_name, roe_path)

        path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

    elif mode == 'Expert':

        # filter rest of events only for specific particle list
        ma.signalSideParticleListsFilter(
            particleLists,
            'nROE_Charged(' + maskName + ', 0) > 0',
            roe_path,
            dead_end_path)

        flavorTaggerInfoBuilder = basf2.register_module('FlavorTaggerInfoBuilder')
        path.add_module(flavorTaggerInfoBuilder)

        fill_particle_lists(maskName, roe_path)

        ma.rankByHighest('pi+:tflat', rank_variable, path=roe_path)
        ma.rankByHighest('gamma:tflat', rank_variable, path=roe_path)

        vm.addAlias('refdx', 'getVariableByRank(pi+:tflat, p, dx, 1)')
        vm.addAlias('dxdiff', 'formula(dx-refdx)')
        vm.addAlias('refdy', 'getVariableByRank(pi+:tflat, p, dy, 1)')
        vm.addAlias('dydiff', 'formula(dy-refdy)')
        vm.addAlias('refdz', 'getVariableByRank(pi+:tflat, p, dz, 1)')
        vm.addAlias('dzdiff', 'formula(dz-refdz)')

        expert_module = basf2.register_module('MVAExpert')
        expert_module.param('listNames', particleLists)
        expert_module.param('identifier', uniqueIdentifier)
        expert_module.param('extraInfoName', 'tflat_output')

        roe_path.add_module(expert_module)

        flavorTaggerInfoFiller = basf2.register_module('FlavorTaggerInfoFiller')
        flavorTaggerInfoFiller.param('TFLATnn', True)
        roe_path.add_module(flavorTaggerInfoFiller)

        # Create standard alias for the output of the flavor tagger
        vm.addAlias('qrTFLAT', 'qrOutput(TFLAT)')

        path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

    elif mode == 'Teacher':
        input_files = os.path.join(working_dir, uniqueIdentifier + '_training_data*.root')
        if len(glob.glob(input_files)) == 0:
            B2FATAL('There is no training data file available. Run flavor tagger in Sampler mode first.')
        general_options = basf2_mva.GeneralOptions()
        general_options.m_datafiles = basf2_mva.vector(input_files)

        general_options.m_treename = tree_name
        general_options.m_target_variable = target
        general_options.m_variables = basf2_mva.vector(*features)

        general_options.m_identifier = uniqueIdentifier

        specific_options = basf2_mva.PythonOptions()
        specific_options.m_framework = 'tensorflow'
        specific_options.m_steering_file = mva_steering_file
        specific_options.m_training_fraction = train_valid_fraction

        specific_options.m_config = json.dumps(classifier_args)

        basf2_mva.teacher(general_options, specific_options)
