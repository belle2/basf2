#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Jochen Gemmler 2016

from dft.DeepFlavorTagger import *


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
    main = create_path()

    if not os.path.exists(working_dir) and working_dir is not '':
        os.makedirs(working_dir)

    inputMdstList(environmentType, filelist=file_names, path=main)

    findMCDecay('B0:sig', 'B0 -> nu_tau anti-nu_tau', writeOut=True, path=main)
    matchMCTruth('B0:sig', main)
    applyCuts('B0:sig', 'isSignal > 0.5', path=main)

    buildRestOfEvent('B0:sig', path=main)

    DeepFlavorTagger('B0:sig', mode, working_dir, identifier, variable_list, target=target, overwrite=overwrite,
                     path=main, *args, **kwargs)

    main.add_module('ProgressBar')

    process(main, max_events)
    print(statistics)


def test_expert(working_dir, file_names, identifier, output_variable='networkOutput', environmentType='MC5',
                max_events=0):
    main = create_path()

    inputMdstList(environmentType, file_names, path=main)

    findMCDecay('B0:sig', 'B0 -> nu_tau anti-nu_tau', writeOut=True, path=main)
    matchMCTruth('B0:sig', main)
    applyCuts('B0:sig', 'isSignal > 0.5', path=main)

    buildRestOfEvent('B0:sig', path=main)

    # main.add_module('PrintCollections')
    DeepFlavorTagger('B0:sig', 'expert', working_dir, identifier, path=main)

    # define output variable
    output_variable_name = ''.join('extraInfo(', output_variable, ')')

    variablesToNtuple('B0:sig', ['extraInfo(qrCombined)', output_variable_name],
                      filename=os.path.join(working_dir, identifier + '_test_output.root'),
                      path=main)

    main.add_module('ProgressBar')

    process(main, max_events)
    print(statistics)


def test_expert_jpsi(working_dir, file_names, prefix, environmentType='MC5', max_events=0):
    main = create_path()

    inputMdstList(environmentType, file_names, path=main)

    fillParticleList('pi+:highPID', 'piid >= .1', path=main)
    fillParticleList('mu+:highPID', 'muid >= .1', path=main)

    # reconstruct Ks -> pi+ pi- decay
    # keep only candidates with dM<0.25
    reconstructDecay('K_S0:pipi -> pi+:highPID pi-:highPID', '.25 <= M <= .75', path=main)
    # fit K_S0 Vertex

    fitVertex('K_S0:pipi', 0., '', 'rave', 'vertex', '', False, path=main)

    # reconstruct J/psi -> mu+ mu- decay and fit vertex
    reconstructDecay('J/psi:mumu -> mu+:highPID mu-:highPID', '3.0 <= M <= 3.2 ', path=main)

    # applyCuts('J/psi:mumu', '3.07 < M < 3.11', path=main)
    applyCuts('J/psi:mumu', '', path=main)
    massVertexRave('J/psi:mumu', 0., '', path=main)

    # reconstruct B0 -> J/psi Ks decay
    reconstructDecay('B0:jpsiks -> J/psi:mumu K_S0:pipi', '5.2 <= M <= 5.4', path=main)

    # Fit the B0 Vertex
    vertexRave('B0:jpsiks', 0., 'B0 -> [J/psi -> ^mu+ ^mu-] K_S0', '', path=main)

    # perform MC matching (MC truth asociation). Always before TagV
    matchMCTruth('B0:jpsiks', path=main)

    # build the rest of the event associated to the B0
    buildRestOfEvent('B0:jpsiks', path=main)
    applyCuts('B0:jpsiks', 'isSignal > 0.5', path=main)
    # main.add_module('PrintCollections')

    DeepFlavorTagger('B0:jpsiks', 'Expert', working_dir, prefix, transform_to_probability=True, path=main)
    variablesToNtuple('B0:jpsiks', ['extraInfo(qrCombined)', 'extraInfo(qrMC)', 'extraInfo(B0Probability)',
                                    'extraInfo(BOProbabilityMC)'],
                      filename=os.path.join(working_dir, 'test_output.root'), path=main)

    main.add_module('ProgressBar')

    process(main, max_events)
    print(statistics)
