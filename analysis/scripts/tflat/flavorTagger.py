#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import yaml
from basf2 import B2FATAL
import basf2
from variables import variables as vm
import modularAnalysis as ma
from stdPhotons import stdPhotons
from vertex import kFit
from tflat.utils import get_variables


def fill_particle_lists(config, maskName='TFLATDefaultMask', path=None):
    """
    Fills the particle lists.
    """

    # create particle list with pions
    trk_cut = f'isInRestOfEvent > 0.5 and passesROEMask({maskName}) > 0.5 and p >= 0'
    ma.fillParticleList('pi+:tflat', trk_cut, path=path)

    # create particle list with gammas

    # load MVA's for all gamma
    ma.fillParticleList(
        "gamma:all",
        "",
        path=path,
    )
    ma.getBeamBackgroundProbability("gamma:all", config['VersionBeamBackgroundMVA'], path=path)
    ma.getFakePhotonProbability("gamma:all", config['VersionFakePhotonMVA'], path=path)

    stdPhotons(listtype='tight',  path=path)

    gamma_cut = f'isInRestOfEvent > 0.5 and passesROEMask({maskName}) > 0.5 \
        and beamBackgroundSuppression > 0.4 and fakePhotonSuppression > 0.3'
    ma.cutAndCopyList('gamma:tflat', 'gamma:tight', gamma_cut, path=path)

    ma.reconstructDecay('K_S0:inRoe -> pi+:tflat pi-:tflat', '0.40<=M<=0.60', False, path=path)
    kFit('K_S0:inRoe', 0.01, path=path)


def flavorTagger(particleLists, mode='Expert', working_dir='', uniqueIdentifier='standard_tflat',
                 target='qrCombined', overwrite=False,
                 sampler_id=0,
                 path=None):
    """
    Interfacing for the Transformer FlavorTagger (TFlat). This function can be used for preparation of
    training datasets (``Sampler``) and inference (``Expert``).

    This function requires reconstructed B meson signal particle list and where an RestOfEvent is built.

    :param particleLists:  string or list[string], particle list(s) of the reconstructed signal B meson
    :param mode: string, valid modes are ``Expert`` (default), ``Sampler``
    :param working_dir: string, working directory for the method
    :param uniqueIdentifier: string, database identifier for the method
    :param target: string, target variable
    :param overwrite: bool, overwrite already (locally!) existing training
    :param sampler_id: identifier of sampled file for parallel sampling
    :param path: basf2 path obj
    :return: None
    """

    if isinstance(particleLists, str):
        particleLists = [particleLists]

    if mode not in ['Expert', 'Sampler']:
        B2FATAL(f'Invalid mode  {mode}')

    tree_name = 'tflat_variables'
    rank_variable = 'p'

    config = yaml.full_load(basf2.find_file(f'{uniqueIdentifier}.yaml'))

    # create default ROE-mask
    TFLAT_mask = config['TFLAT_Mask']
    maskName = TFLAT_mask[0]
    for name in particleLists:
        ma.appendROEMasks(list_name=name, mask_tuples=[TFLAT_mask], path=path)

    # create roe specific paths
    roe_path = basf2.create_path()
    dead_end_path = basf2.create_path()

    if mode == 'Sampler':
        trk_variable_list = config['trk_variable_list']
        ecl_variable_list = config['ecl_variable_list']
        roe_variable_list = config['roe_variable_list']
        # create tagging specific variables
        features = get_variables('pi+:tflat', rank_variable, trk_variable_list, particleNumber=config['parameters']['num_trk'])
        features += get_variables('gamma:tflat', rank_variable, ecl_variable_list, particleNumber=config['parameters']['num_ecl'])
        features += get_variables('pi+:tflat', rank_variable, roe_variable_list, particleNumber=config['parameters']['num_roe'])

        output_file_name = os.path.join(working_dir, uniqueIdentifier + f'_training_data{sampler_id}.root')
        if os.path.isfile(output_file_name) and not overwrite:
            B2FATAL(f'Outputfile {output_file_name} already exists. Aborting writeout.')

        # filter rest of events only for specific particle list
        ma.signalSideParticleListsFilter(
            particleLists,
            f'nROE_Charged({maskName}, 0) > 0 and abs(qrCombined) == 1',
            roe_path,
            dead_end_path)

        fill_particle_lists(config, maskName, roe_path)

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
            f'nROE_Charged({maskName}, 0) > 0',
            roe_path,
            dead_end_path)

        path.add_module('FlavorTaggerInfoBuilder')

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
