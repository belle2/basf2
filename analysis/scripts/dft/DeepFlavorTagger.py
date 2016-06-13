#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Jochen Gemmler 2016

from ROOT import gSystem
gSystem.Load('libanalysis.so')
from modularAnalysis import *
from ROOT import Belle2

import os
import json


class RemoveEmptyROEModule(Module):

    """
    Detects when a ROE does not contain tracks in order to skip it.
    """

    def __init__(self, extraInfoOutputName, targetExtraInfoOutputName):
        super(RemoveEmptyROEModule, self).__init__()

        # FIXME: confuse boost.pythons reference counting (something deep inside basf2?)
        self.self = self

        self.output_name = extraInfoOutputName
        self.target_name = targetExtraInfoOutputName

    def event(self):
        """ Process for each event """

        self.return_value(0)
        roe = Belle2.PyStoreObj('RestOfEvent')
        signal_b0 = roe.obj().getRelated('Particles')
        variable_manager = Belle2.Variable.Manager.Instance()

        qr_MC = 2 * (variable_manager.evaluate(self.target_name, None) - .5)

        if variable_manager.evaluate('isRestOfEventEmpty', signal_b0) == -2:
            B2WARNING('DFT: No Tracks in RestOfEvent. Discarded for training. Output is set to .5.')
            signal_b0.addExtraInfo('B0Probability', .5)
            signal_b0.addExtraInfo('B0barProbablitiy', .5)
            signal_b0.addExtraInfo('qrCombined', 0)
            signal_b0.addExtraInfo('qrMC', qr_MC)
            self.return_value(1)
            return None

        if variable_manager.evaluate('qrCombined', None) < 0:
            B2WARNING('DFT: No B-Meson in RestOfEvent. Discarded for training. Output set to .5')
            signal_b0.addExtraInfo('B0Probability', .5)
            signal_b0.addExtraInfo('B0barProbablitiy', .5)
            signal_b0.addExtraInfo('qrCombined', 0)
            signal_b0.addExtraInfo('qrMC', -999)
            self.return_value(1)
            return None


class FlavorTaggerInforFiller(Module):
    """
    fills FlaverTaggerInfoMap according to the Default Flavor Tagger style
    """

    def __init__(self, networkExtraInfoOutputName, targetVariableName):
        super(FlavorTaggerInforFiller, self).__init__()

        # FIXME:
        self.self = self

        self.network_output_name = networkExtraInfoOutputName
        self.target_name = targetVariableName

    def event(self):
        roe = Belle2.PyStoreObj('RestOfEvent')
        ext_info = Belle2.PyStoreObj('EventExtraInfo')
        ft_info = roe.obj().getRelated('FlavorTaggerInfos')

        variable_manager = Belle2.Variable.Manager.Instance()
        signal_b0 = roe.obj().getRelated('Particles')

        # tagging output
        b0_probability = ext_info.obj().getExtraInfo(self.network_output_name)
        qr_combined = 2 * (b0_probability - 0.5)
        b0bar_probability = 1 - b0_probability

        # MC truth (qrCombined is the Target variable)
        b0_probability_MC = variable_manager.evaluate(self.target_name, None)
        qr_MC = 2 * (b0_probability_MC - .5)
        ft_info.setUseModeFlavorTagger("Expert")
        ft_info.addMethodMap("TMVA")
        ft_info_map = ft_info.getMethodMap("TMVA")
        ft_info_map.setQrCombined(qr_combined)
        ft_info_map.setB0Probability(b0_probability)
        ft_info_map.setB0barProbability(b0bar_probability)

        # also move the information to the signal side b-meson
        # this is not done in the new flavor tagger scripts anymore
        signal_b0.addExtraInfo('B0Probability', b0_probability)
        signal_b0.addExtraInfo('B0barProbablitiy', b0bar_probability)
        signal_b0.addExtraInfo('qrCombined', qr_combined)
        signal_b0.addExtraInfo('B0ProbablitiyMC', b0_probability_MC)
        signal_b0.addExtraInfo('qrMC', qr_MC)

        if not ft_info:
            B2ERROR('FlavorTaggerInfoFiller: FlavorTaggerInfo does not exist')


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
        root_compatible_list.append(makeROOTCompatible(var))

    return root_compatible_list


def makeROOTCompatibleFallback(var):
    # try the basf2 build-in with import Belle2.Variable.makeROOTCompatible
    r_dict = get_ROOT_Compatible_dict()
    for r_item, r_val in r_dict.items():
        var = var.replace(r_item, r_val)
    return var


def makeROOTCompatible(var):
    """ make variable name root compatible
    :param var:
    :return:
    """
    try:
        root_var = Belle2.Variable.makeROOTCompatible(var)
    except(AttributeError):
        root_var = makeROOTCompatibleFallback(var)
    return root_var


def inverseROOTCompatibleFallback(var):
    r_dict = get_ROOT_Compatible_dict(inverse=True)
    for r_item, r_val in r_dict.items():
        var = var.replace(r_item, r_val)
    return var


def inverseROOTCompatible(var):
    try:
        root_var = Belle2.Variable.inverseROOTCompatible(var)
    except(AttributeError):
        root_var = inverseROOTCompatibleFallback(var)
    return root_var


def get_ROOT_Compatible_dict(inverse=False):
    r_dict = dict([
        (" ", "__sp"),
        (",", "__cm"),
        (":", "__cl"),
        ("=", "__eq"),
        ("<", "__st"),
        (">", "__gt"),
        (".", "__pt"),
        ("+", "__pl"),
        ("\-", "__mi"),
        ("(", "__bo"),
        (")", "__bc"),
        ("{", "__co"),
        ("}", "__cc"),
        ("[", "__so"),
        ("]", "__sc"),
        ("`", "__to"),
        ("´", "__tc"),
        ("^", "__ha"),
        ("°", "__ci"),
        ("$", "__do"),
        ("§", "__pa"),
        ("%", "__pr"),
        ("!", "__em"),
        ("?", "__qm"),
        (";", "__sm"),
        ("#", "__hs"),
        ("*", "__mu"),
        ("/", "__sl"),
        ("\\", "__bl"),
        ("'", "__sq"),
        ("\"", "__dq"),
        ("~", "__ti"),
        ("-", "__da"),
        ("|", "__pi"),
        ("&", "__am"),
        ("@", "__at"),
    ])
    if inverse:
        r_dict = {v: k for k, v in r_dict.items()}
    return r_dict


def DeepFlavorTagger(particleList, mode='expert', working_dir='', uniqueIdentifier='standard', variable_list=None,
                     output_variable='networkOutput', target='qrCombined', overwrite=False,
                     transform_to_probability=False, signal_fraction=-1.0, classifier_args=None, convert_to_cpu=True,
                     train_valid_fraction=.92, dry_run=False, path=analysis_main):
    """
    DeepFlavorTagger classifier function.

    :param particleList: string, particle list of the reconstructing signal
    :param mode: string, valid modes are expert, externTeacher
    :param working_dir: string, working directory for the method
    :param uniqueIdentifier: string, database identifier for the method
    :param variable_list: list of strings, name of the basf2 variables used for discrimination
    :param output_variable: string, variable name returned by the expert
    :param target: string, target variable
    :param overwrite: bool, overwrite already (locally!) existing training
    :param transform_to_probability: bool, enable transformation, can only be set during training
    :param signal_fraction: float, signal fraction overried, can only be set during training
    :param classifier_args:dictionary, costumized arguments for the mlp
    :param convert_to_cpu: bool, enable conversion of the classifier so that is usable on CPU, can only be set during
    training
    :param train_valid_fraction: float, separation between train, valid fraction. if transform to probability is
    enabled, train valid fraction will be splitted to a test set (.5)
    :param dry_run: bool, only print teacher string, will be removed
    :param path: basf2 path
    :return: None
    """

    if mode not in ['expert', 'externTeacher']:
        B2FATAL('Invalid mode  %s' % mode)

    if variable_list is None and mode is 'externTeacher':
        variable_list = ['useCMSFrame(p)', 'useCMSFrame(cosTheta)', 'useCMSFrame(phi)', 'Kid', 'eid', 'muid', 'prid',
                         'nCDCHits', 'nPXDHits', 'nSVDHits', 'dz', 'dr']

    if variable_list is not None and mode is 'expert':
        B2WARNING('DFT: Variables from identifier file are used. Input variables will be ignored.')

    if signal_fraction > -1.0 and mode is 'expert':
        B2ERROR('Signalfraction override is only possible during training. Samplesignalfraction is used.')

    if classifier_args is None:
        classifier_args = {}
    else:
        assert isinstance(classifier_args, dict)

    classifier_args['transform_to_prob'] = transform_to_probability
    classifier_args['convert_to_cpu'] = convert_to_cpu

    output_file_name = os.path.join(working_dir, uniqueIdentifier + '_training_data.root')

    # create roe specific paths
    roe_path = create_path()
    dead_end_path = create_path()

    # create flavor tagger info
    ft_info_builder = register_module('FlavorTaggerInfoBuilder')
    path.add_module(ft_info_builder)

    # define ft specific lists
    roe_particle_list_cut = ''
    roe_particle_list = 'pi+:dft'

    # filter rest of events only for specific particle list
    signalSideParticleFilter(particleList, '', roe_path, dead_end_path)

    # removes empty ROEs
    # sets flavor tagger specific variables to unknown value (.5)

    ROEEmptyTrigger = RemoveEmptyROEModule(output_variable, target)
    roe_path.add_module(ROEEmptyTrigger)
    ROEEmptyTrigger.if_true(dead_end_path)

    # create final state particle lists
    fillParticleList(roe_particle_list, roe_particle_list_cut, path=roe_path)

    particleLists = ['pi+:pos_charged', 'pi+:neg_charged']

    cutAndCopyList(particleLists[0], roe_particle_list, 'charge > 0 and isInRestOfEvent == 1 and p < infinity',
                   writeOut=True, path=roe_path)
    cutAndCopyList(particleLists[1], roe_particle_list, 'charge < 0 and isInRestOfEvent == 1 and p < infinity',
                   writeOut=True, path=roe_path)

    # sort pattern for tagging specific variables
    rankVariable = 'p'

    for particles in particleLists:
        rankByHighest(particles, rankVariable, path=roe_path)

    if mode is 'externTeacher':
        if os.path.isfile(output_file_name) and not overwrite:
            B2FATAL('Outputfile %s already exists. Aborting writeout.')

        tree_name = 'dft_variables'

        # create tagging specific variables
        features = get_variables(particleLists[0], rankVariable, variable_list, particleNumber=5)
        features += get_variables(particleLists[1], rankVariable, variable_list, particleNumber=5)

        # and add target
        all_variables = features + [target]

        # write to ntuples
        variablesToNTuple('', all_variables, tree_name, output_file_name, roe_path)

        # write the command line output for the extern teacher to a file
        extern_command = 'basf2_mva_teacher --datafile {output_file_name} --treename {tree_name}' \
                         ' --weightfile {prefix} --variables "{variables_string}"  --target_variable {target}' \
                         ' --method Python --training_fraction {fraction}' \
                         " --config '{mlp_args}' --framework" \
                         ' pylearn2'.format(output_file_name=output_file_name, tree_name=tree_name, prefix=uniqueIdentifier,
                                            variables_string='" "'.join(features), target=target,
                                            mlp_args=json.dumps(classifier_args), fraction=train_valid_fraction)

        with open(os.path.join(working_dir, uniqueIdentifier + '_teacher_prefix'), 'w') as f:
            f.write(extern_command)
        # FIXME: remove this
        if dry_run:
            print(extern_command)
            exit(0)
        B2INFO('Extern Teacher command:\n %s' % extern_command)
        # TODO: Integrate transform to probability
        B2WARNING('Transformation to probability is not implemented yet.')

    if mode is 'expert':
        expert_module = register_module('MVAExpert')
        expert_module.param('identifier', uniqueIdentifier)
        # adds this parameter to the event extra info
        # if its already built and not cleared B2ERROR is triggered
        expert_module.param('extraInfoName', output_variable)
        expert_module.param('signalFraction', signal_fraction)

        roe_path.add_module(expert_module)

        # filling the flavor tagger info
        # requires FlavorTaggerInfoBuilder in main path
        # also fills the extra info of the specific signal side particle
        ft_info_filler = FlavorTaggerInforFiller(output_variable, target)
        roe_path.add_module(ft_info_filler)

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
