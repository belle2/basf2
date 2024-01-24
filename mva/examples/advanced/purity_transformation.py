#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2_mva
import basf2_mva_util
import time

if __name__ == "__main__":
    from basf2 import conditions, find_file
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    train_file = find_file("mva/train_D0toKpipi.root", "examples")
    test_file = find_file("mva/test_D0toKpipi.root", "examples")

    training_data = basf2_mva.vector(train_file)
    testing_data = basf2_mva.vector(test_file)

    variables = [
        'M',
        'p',
        'pt',
        'pz',
        'daughter(0, p)',
        'daughter(0, pz)',
        'daughter(0, pt)',
        'daughter(1, p)',
        'daughter(1, pz)',
        'daughter(1, pt)',
        'daughter(2, p)',
        'daughter(2, pz)',
        'daughter(2, pt)',
        'chiProb',
        'dr',
        'dz',
        'daughter(0, dr)',
        'daughter(1, dr)',
        'daughter(0, dz)',
        'daughter(1, dz)',
        'daughter(0, chiProb)',
        'daughter(1, chiProb)',
        'daughter(2, chiProb)',
        'daughter(0, kaonID)',
        'daughter(0, pionID)',
        'daughterInvM(0, 1)',
        'daughterInvM(0, 2)',
        'daughterInvM(1, 2)']

    # Train a MVA method and directly upload it to the database
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    general_options.m_treename = "tree"
    general_options.m_identifier = "MVADatabaseIdentifier"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    fastbdt_options = basf2_mva.FastBDTOptions()
    fastbdt_options.m_nTrees = 100
    fastbdt_options.m_nCuts = 10
    fastbdt_options.m_nLevels = 3
    fastbdt_options.m_shrinkage = 0.2
    fastbdt_options.m_randRatio = 0.5

    fastbdt_pt_options = basf2_mva.FastBDTOptions()
    fastbdt_pt_options.m_nTrees = 100
    fastbdt_pt_options.m_nCuts = 10
    fastbdt_pt_options.m_nLevels = 3
    fastbdt_pt_options.m_shrinkage = 0.2
    fastbdt_pt_options.m_randRatio = 0.5
    fastbdt_pt_options.m_purityTransformation = True

    stats = []
    for label, options in [("FastBDT", fastbdt_options), ("FastBDT_PT", fastbdt_pt_options)]:
        training_start = time.time()
        general_options.m_identifier = label
        basf2_mva.teacher(general_options, options)
        training_stop = time.time()
        training_time = training_stop - training_start
        method = basf2_mva_util.Method(general_options.m_identifier)
        inference_start = time.time()
        p, t = method.apply_expert(testing_data, general_options.m_treename)
        inference_stop = time.time()
        inference_time = inference_stop - inference_start
        auc = basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)
        print(label, training_time, inference_time, auc)
        stats.append((label, training_time, inference_time, auc))

    for line in stats:
        print(*line)
