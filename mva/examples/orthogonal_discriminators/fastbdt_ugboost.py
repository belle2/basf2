#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2_mva

if __name__ == "__main__":
    from basf2 import conditions, find_file
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    variables = ['p', 'pt', 'pz', 'phi',
                 'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)', 'daughter(0, phi)',
                 'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)', 'daughter(1, phi)',
                 'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)', 'daughter(2, phi)',
                 'chiProb', 'dr', 'dz', 'dphi',
                 'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, dz)', 'daughter(1, dz)',
                 'daughter(0, dphi)', 'daughter(1, dphi)',
                 'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
                 'daughter(0, kaonID)', 'daughter(0, pionID)', 'daughter(1, kaonID)', 'daughter(1, pionID)',
                 'daughterAngle(0, 1)', 'daughterAngle(0, 2)', 'daughterAngle(1, 2)',
                 'daughter(2, daughter(0, E))', 'daughter(2, daughter(1, E))',
                 'daughter(2, daughter(0, clusterTiming))', 'daughter(2, daughter(1, clusterTiming))',
                 'daughter(2, daughter(0, clusterE9E25))', 'daughter(2, daughter(1, clusterE9E25))',
                 'daughter(2, daughter(0, minC2TDist))', 'daughter(2, daughter(1, minC2TDist))',
                 'M']

    variables2 = ['p', 'pt', 'pz', 'phi',
                  'chiProb', 'dr', 'dz', 'dphi',
                  'daughter(2, chiProb)',
                  'daughter(0, kaonID)', 'daughter(0, pionID)', 'daughter(1, kaonID)', 'daughter(1, pionID)',
                  'daughter(2, daughter(0, E))', 'daughter(2, daughter(1, E))',
                  'daughter(2, daughter(0, clusterTiming))', 'daughter(2, daughter(1, clusterTiming))',
                  'daughter(2, daughter(0, clusterE9E25))', 'daughter(2, daughter(1, clusterE9E25))',
                  'daughter(2, daughter(0, minC2TDist))', 'daughter(2, daughter(1, minC2TDist))']

    train_file = find_file("mva/train_D0toKpipi.root", "examples")
    training_data = basf2_mva.vector(train_file)

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    # Spectators are the variables for which the selection should be uniform
    general_options.m_spectators = basf2_mva.vector('daughterInvM(0, 1)', 'daughterInvM(0, 2)')
    general_options.m_target_variable = "isSignal"
    general_options.m_identifier = "fastbdt"

    fastbdt_options = basf2_mva.FastBDTOptions()
    fastbdt_options.m_nTrees = 100
    fastbdt_options.m_nCuts = 10
    fastbdt_options.m_nLevels = 5
    fastbdt_options.m_shrinkage = 0.1
    fastbdt_options.m_randRatio = 0.5
    fastbdt_options.m_flatnessLoss = 10.0

    basf2_mva.teacher(general_options, fastbdt_options)

    general_options.m_identifier = "fastbdt_baseline"
    fastbdt_options.m_flatnessLoss = -1.0
    basf2_mva.teacher(general_options, fastbdt_options)

    general_options.m_identifier = "fastbdt_drop_feature"
    fastbdt_options.m_flatnessLoss = -1.0
    general_options.m_variables = basf2_mva.vector(*variables2)
    basf2_mva.teacher(general_options, fastbdt_options)
