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
import basf2_mva
from basf2 import B2FATAL
import basf2
from variables import variables as vm
import modularAnalysis as ma


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
            var_list.append('getVariableByRank(' + particle_list + ', ' + ranked_variable + ', ' + var + ', ' +
                            str(i_num) + ')')
    return var_list


def FlavorTagger(particle_lists, mode='expert', working_dir='', uniqueIdentifier='standard_tflat',
                 target='qrCombined', overwrite=False,
                 classifier_args=None,
                 train_valid_fraction=.92, mva_steering_file='analysis/scripts/tflat/tensorflow_tflat_interface.py',
                 maskName='all',
                 path=None):
    """
    Interfacing for the Transformer FlavorTagger (TFlat). This function can be used for training (``teacher``), preparation of
    training datasets (``sampler``) and inference (``expert``).

    This function requires reconstructed B meson signal particle list and where an RestOfEvent is built.

    :param particle_lists:  string or list[string], particle list(s) of the reconstructed signal B meson
    :param mode: string, valid modes are ``expert`` (default), ``teacher``, ``sampler``
    :param working_dir: string, working directory for the method
    :param uniqueIdentifier: string, database identifier for the method
    :param target: string, target variable
    :param overwrite: bool, overwrite already (locally!) existing training
    :param classifier_args: dictionary, customized arguments for tflat
    :param train_valid_fraction: float, train-valid fraction (.92).
    :param maskName: get ROE particles from a specified ROE mask
    :param path: basf2 path obj
    :return: None
    """

    if isinstance(particle_lists, str):
        particle_lists = [particle_lists]

    if mode not in ['expert', 'teacher', 'sampler']:
        B2FATAL(f'Invalid mode  {mode}')

    if mode in ['sampler', 'teacher']:
        variable_list = [
            'charge',
            'useCMSFrame(p)',
            'useCMSFrame(cosTheta)',
            'useCMSFrame(phi)',
            'kaonID',
            'electronID',
            'muonID',
            'protonID',
            'nCDCHits',
            'nPXDHits',
            'nSVDHits',
            'dz',
            'dr',
            'chiProb']

    if classifier_args is None:
        classifier_args = {}
    else:
        assert isinstance(classifier_args, dict)

    output_file_name = os.path.join(working_dir, uniqueIdentifier + '_training_data.root')

    # create roe specific paths
    roe_path = basf2.create_path()
    dead_end_path = basf2.create_path()

    # define tflat specific lists to enable multiple calls, if someone really wants to do that
    extension = particle_lists[0].replace(':', '_to_')
    roe_particle_list_cut = ''
    roe_particle_list = 'pi+:tflat' + '_' + extension

    tree_name = 'tflat_variables'

    # filter rest of events only for specific particle list
    ma.signalSideParticleListsFilter(particle_lists, 'hasRestOfEventTracks > 0', roe_path, dead_end_path)

    # create final state particle lists
    ma.fillParticleList(roe_particle_list, roe_particle_list_cut, path=roe_path)

    tflat_particle_lists = ['pi+:pos_charged',]

    pos_cut = 'isInRestOfEvent == 1 and passesROEMask(' + maskName + ') > 0.5 and p < infinity'

    ma.cutAndCopyList(tflat_particle_lists[0], roe_particle_list, pos_cut, writeOut=True, path=roe_path)

    # sort pattern for tagging specific variables
    rank_variable = 'p'
    # rank_variable = 'useCMSFrame(p)'

    # create tagging specific variables
    if mode != 'expert':
        features = get_variables(tflat_particle_lists[0], rank_variable, variable_list, particleNumber=10)

    for particles in tflat_particle_lists:
        ma.rankByHighest(particles, rank_variable, path=roe_path)

    if mode == 'sampler':
        if os.path.isfile(output_file_name) and not overwrite:
            B2FATAL(f'Outputfile {output_file_name} already exists. Aborting writeout.')

        # and add target
        all_variables = features + [target]

        # write to ntuples
        ma.variablesToNtuple('', all_variables, tree_name, output_file_name, roe_path)

        # write the command line output for the extern teacher to a file
        extern_command = f'basf2_mva_teacher --datafile {output_file_name} --treename {tree_name}' + \
                         f' --identifier {uniqueIdentifier} ' + \
                         '--variables "{}"  '.format('" "'.join(features)) + \
                         f'--target_variable {target}' + \
                         f' --method Python --training_fraction {train_valid_fraction}' + \
                         f" --config '{json.dumps(classifier_args)}' --framework tensorflow" + \
                         f' --steering_file {mva_steering_file}'

        with open(os.path.join(working_dir, uniqueIdentifier + '_teacher_command'), 'w') as f:
            f.write(extern_command)

    elif mode == 'teacher':
        if not os.path.isfile(output_file_name):
            B2FATAL('There is no training data file available. Run flavor tagger in sampler mode first.')
        general_options = basf2_mva.GeneralOptions()
        general_options.m_datafiles = basf2_mva.vector(output_file_name)

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

    elif mode == 'expert':

        flavorTaggerInfoBuilder = basf2.register_module('FlavorTaggerInfoBuilder')
        path.add_module(flavorTaggerInfoBuilder)

        expert_module = basf2.register_module('MVAExpert')
        expert_module.param('listNames', particle_lists)
        expert_module.param('identifier', uniqueIdentifier)
        expert_module.param('extraInfoName', 'tflat_output')

        roe_path.add_module(expert_module)

        flavorTaggerInfoFiller = basf2.register_module('FlavorTaggerInfoFiller')
        flavorTaggerInfoFiller.param('TFLATnn', True)
        roe_path.add_module(flavorTaggerInfoFiller)

        # Create standard alias for the output of the flavor tagger
        vm.addAlias('TFLAT_qrCombined', 'qrOutput(TFLAT)')

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
