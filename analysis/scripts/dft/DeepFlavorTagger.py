#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Jochen Gemmler 2016

from ROOT import gSystem
gSystem.Load('libanalysis.so')
from modularAnalysis import *
from ROOT import Belle2

import basf2_mva

# make ROOT compatible available
Belle2.Variable.Manager
Belle2.Variable.Manager.Instance()

import os
import json


def get_variables(particle_list, ranked_variable, variables=None, particleNumber=1):
    """ creates variable name pattern requested by the basf2 variable getVariableByRank()
    :param particle_list:
    :param ranked_variable:
    :param variables:
    :param particleNumber:
    :return:
    """
    var_list = []
    for var in variables:
        for i_num in range(1, particleNumber + 1):
            var_list.append('getVariableByRank(' + particle_list + ', ' + ranked_variable + ', ' + var + ', ' +
                            str(i_num) + ')')
    return var_list


def construct_default_variable_names(particle_lists=None, ranked_variable='p', variables=None, particleNumber=5):
    """ construct default variables (that are sorted by charge and ranked by momentum)
    :param particle_lists:
    :param ranked_variable:
    :param variables:
    :param particleNumber:
    :return:
    """
    if particle_lists is None:
        particle_lists = ['pi+:pos_charged', 'pi+:neg_charged']

    variable_names = []
    for p_list in particle_lists:
        variable_names += get_variables(p_list, ranked_variable, variables, particleNumber)

    # make root compatible
    root_compatible_list = []
    for var in variable_names:
        root_compatible_list.append(Belle2.makeROOTCompatible(var))

    return root_compatible_list


def DeepFlavorTagger(particle_list, mode='expert', working_dir='', uniqueIdentifier='standard', variable_list=None,
                     output_variable='networkOutput', target='qrCombined', overwrite=False,
                     transform_to_probability=False, signal_fraction=-1.0, classifier_args=None,
                     train_valid_fraction=.92, mva_steering_file='analysis/scripts/dft/tensorflow_dnn_interface.py',
                     path=analysis_main):
    """
    DeepFlavorTagger classifier function.

    :param particle_list: string, particle list of the reconstructing signal
    :param mode: string, valid modes are expert, teacher, sampler
    :param working_dir: string, working directory for the method
    :param uniqueIdentifier: string, database identifier for the method
    :param variable_list: list of strings, name of the basf2 variables used for discrimination
    :param output_variable: string, variable name returned by the expert
    :param target: string, target variable
    :param overwrite: bool, overwrite already (locally!) existing training
    :param transform_to_probability: bool, enable transformation, can only be set during training
    :param signal_fraction: float, signal fraction override, can only be set during training
    :param classifier_args:dictionary, costumized arguments for the mlp
    :param train_valid_fraction: float, train-valid fraction. if transform to probability is
    enabled, train valid fraction will be splitted to a test set (.5)
    :param path: basf2 path obj
    :return: None
    """

    if mode not in ['expert', 'teacher', 'sampler']:
        B2FATAL('Invalid mode  %s' % mode)

    if variable_list is None and mode in ['sampler', 'teacher']:
        variable_list = ['useCMSFrame(p)', 'useCMSFrame(cosTheta)', 'useCMSFrame(phi)', 'Kid', 'eid', 'muid', 'prid',
                         'nCDCHits', 'nPXDHits', 'nSVDHits', 'dz', 'dr', 'chiProb']

    if variable_list is not None and mode is 'expert':
        B2ERROR('DFT: Variables from identifier file are used. Input variables will be ignored.')

    if classifier_args is None:
        classifier_args = {}
    else:
        assert isinstance(classifier_args, dict)

    classifier_args['transform_to_prob'] = transform_to_probability

    output_file_name = os.path.join(working_dir, uniqueIdentifier + '_training_data.root')

    # create roe specific paths
    roe_path = create_path()
    dead_end_path = create_path()

    # define dft specific lists
    roe_particle_list_cut = ''
    roe_particle_list = 'pi+:dft'

    tree_name = 'dft_variables'

    # filter rest of events only for specific particle list
    signalSideParticleFilter(particle_list, 'hasRestOfEventTracks > 0', roe_path, dead_end_path)

    # TODO: particles with empty rest of events seems not to show up in efficiency statistics anymore

    # create final state particle lists
    fillParticleList(roe_particle_list, roe_particle_list_cut, path=roe_path)

    particle_lists = ['pi+:pos_charged', 'pi+:neg_charged']

    cutAndCopyList(particle_lists[0], roe_particle_list, 'charge > 0 and isInRestOfEvent == 1 and p < infinity',
                   writeOut=True, path=roe_path)
    cutAndCopyList(particle_lists[1], roe_particle_list, 'charge < 0 and isInRestOfEvent == 1 and p < infinity',
                   writeOut=True, path=roe_path)

    # sort pattern for tagging specific variables
    rank_variable = 'p'
    # rank_variable = 'useCMSFrame(p)'

    # create tagging specific variables
    if mode is not 'expert':
        features = get_variables(particle_lists[0], rank_variable, variable_list, particleNumber=5)
        features += get_variables(particle_lists[1], rank_variable, variable_list, particleNumber=5)

    for particles in particle_lists:
        rankByHighest(particles, rank_variable, path=roe_path)

    if mode is 'sampler':
        if os.path.isfile(output_file_name) and not overwrite:
            B2FATAL('Outputfile %s already exists. Aborting writeout.' % output_file_name)

        # and add target
        all_variables = features + [target]

        # write to ntuples
        variablesToNtuple('', all_variables, tree_name, output_file_name, roe_path)

        # write the command line output for the extern teacher to a file
        extern_command = 'basf2_mva_teacher --datafile {output_file_name} --treename {tree_name}' \
                         ' --identifier {identifier} --variables "{variables_string}"  --target_variable {target}' \
                         ' --method Python --training_fraction {fraction}' \
                         " --config '{classifier_args}' --framework tensorflow" \
                         ' --steering_file {steering_file}'\
                         ''.format(output_file_name=output_file_name, tree_name=tree_name,
                                   identifier=uniqueIdentifier,
                                   variables_string='" "'.join(features), target=target,
                                   classifier_args=json.dumps(classifier_args), fraction=train_valid_fraction,
                                   steering_file=mva_steering_file)

        with open(os.path.join(working_dir, uniqueIdentifier + '_teacher_command'), 'w') as f:
            f.write(extern_command)

    elif mode is 'teacher':
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

    elif mode is 'expert':
        # TODO: implement filling flavor tagger info in the FlavorTaggerInfoMap

        # flavor tagger info
        # mod_ft_info_builder = register_module('FlavorTaggerInfoBuilder')
        # path.add_module(mod_ft_info_builder)

        # fill the flavor tagger info
        # mod_ft_info_filler = register_module('FlavorTaggerInfoFiller')

        expert_module = register_module('MVAExpert')
        expert_module.param('identifier', uniqueIdentifier)

        expert_module.param('extraInfoName', output_variable)
        expert_module.param('signalFraction', signal_fraction)

        roe_path.add_module(expert_module)

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
