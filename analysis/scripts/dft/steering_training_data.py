#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os

import basf2 as b2
import modularAnalysis as ma
from dft.DeepFlavorTagger import DeepFlavorTagger
from vertex import treeFit, kFit


def create_train_data(
        working_dir,
        file_names,
        identifier,
        variable_list,
        environmentType='MC5',
        target='qrCombined',
        overwrite=False,
        max_events=0,
        mode='sampler',
        *args,
        **kwargs):
    main = b2.create_path()

    if not os.path.exists(working_dir) and working_dir != '':
        os.makedirs(working_dir)

    ma.inputMdstList(environmentType, filelist=file_names, path=main)

    ma.fillParticleListFromMC('nu_tau:MC', '', path=main)
    ma.reconstructMCDecay('B0:sig -> nu_tau:MC anti-nu_tau:MC', '', writeOut=True, path=main)

    ma.buildRestOfEvent('B0:sig', path=main)

    DeepFlavorTagger('B0:sig', mode, working_dir, identifier, variable_list, target=target, overwrite=overwrite,
                     path=main, *args, **kwargs)

    main.add_module('ProgressBar')

    b2.process(main, max_events)
    print(b2.statistics)


def test_expert(working_dir, file_names, identifier, output_variable='networkOutput', environmentType='MC5',
                max_events=0):
    main = b2.create_path()

    ma.inputMdstList(environmentType, file_names, path=main)

    ma.fillParticleListFromMC('nu_tau:MC', '', path=main)
    ma.reconstructMCDecay('B0:sig -> nu_tau:MC anti-nu_tau:MC', '', writeOut=True, path=main)

    ma.buildRestOfEvent('B0:sig', path=main)

    DeepFlavorTagger('B0:sig', 'expert', working_dir, identifier, path=main)

    # define output variable
    output_variable_name = ''.join('extraInfo(', output_variable, ')')

    ma.variablesToNtuple('B0:sig', ['extraInfo(qrCombined)', output_variable_name],
                         filename=os.path.join(working_dir, identifier + '_test_output.root'),
                         path=main)

    main.add_module('ProgressBar')

    b2.process(main, max_events)
    print(b2.statistics)


def test_expert_jpsi(working_dir, file_names, prefix, environmentType='MC5', max_events=0):
    main = b2.create_path()

    ma.inputMdstList(environmentType, file_names, path=main)

    ma.fillParticleList('pi+:highPID', 'pionID >= .1', path=main)
    ma.fillParticleList('mu+:highPID', 'muonID >= .1', path=main)

    # reconstruct Ks -> pi+ pi- decay
    # keep only candidates with dM<0.25
    ma.reconstructDecay('K_S0:pipi -> pi+:highPID pi-:highPID', '.25 <= M <= .75', path=main)

    # fit K_S0 Vertex
    treeFit('K_S0:pipi', 0., path=main)

    # reconstruct J/psi -> mu+ mu- decay and fit vertex
    ma.reconstructDecay('J/psi:mumu -> mu+:highPID mu-:highPID', '3.0 <= M <= 3.2 ', path=main)

    # applyCuts('J/psi:mumu', '3.07 < M < 3.11', path=main)
    treeFit('J/psi:mumu', 0., massConstraint=['J/psi'], path=main)

    # reconstruct B0 -> J/psi Ks decay
    ma.reconstructDecay('B0:jpsiks -> J/psi:mumu K_S0:pipi', '5.2 <= M <= 5.4', path=main)

    # Fit the B0 Vertex
    kFit('B0:jpsiks', 0., 'vertex', decay_string='B0 -> [J/psi -> ^mu+ ^mu-] K_S0', path=main)

    # perform MC matching (MC truth association). Always before TagV
    ma.matchMCTruth('B0:jpsiks', path=main)

    # build the rest of the event associated to the B0
    ma.buildRestOfEvent('B0:jpsiks', path=main)
    ma.applyCuts('B0:jpsiks', 'isSignal > 0.5', path=main)

    DeepFlavorTagger('B0:jpsiks', 'Expert', working_dir, prefix, transform_to_probability=True, path=main)
    ma.variablesToNtuple('B0:jpsiks', ['extraInfo(qrCombined)', 'extraInfo(qrMC)', 'extraInfo(B0Probability)',
                                       'extraInfo(BOProbabilityMC)'],
                         filename=os.path.join(working_dir, 'test_output.root'), path=main)

    main.add_module('ProgressBar')

    b2.process(main, max_events)
    print(b2.statistics)
