#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
import basf2_mva
import basf2_mva_util
import multiprocessing
import itertools

if __name__ == "__main__":

    train_file = basf2.find_file("mva/train_D0toKpipi.root", "examples")
    test_file = basf2.find_file("mva/test_D0toKpipi.root", "examples")

    training_data = basf2_mva.vector(train_file)
    test_data = basf2_mva.vector(test_file)

    # Train model with default parameters
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    general_options.m_treename = "tree"
    general_options.m_identifier = "test.xml"
    general_options.m_variables = basf2_mva.vector('p', 'pz', 'daughter(0, kaonID)', 'chiProb', 'M')
    general_options.m_target_variable = "isSignal"

    fastbdt_options = basf2_mva.FastBDTOptions()
    basf2_mva.teacher(general_options, fastbdt_options)

    # Load the model and train it again searching for the best hyperparameters
    def grid_search(hyperparameters):
        nTrees, depth = hyperparameters
        method = basf2_mva_util.Method(general_options.m_identifier)
        options = basf2_mva.FastBDTOptions()
        options.m_nTrees = nTrees
        options.m_nLevels = depth
        m = method.train_teacher(training_data, general_options.m_treename, specific_options=options)
        p, t = m.apply_expert(test_data, general_options.m_treename)
        return hyperparameters, basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)

    p = multiprocessing.Pool(None, maxtasksperchild=1)
    results = p.map(grid_search, itertools.product([10, 50, 100, 500, 1000], [2, 4, 6]))
    for hyperparameters, auc in results:
        print("Hyperparameters", hyperparameters, "AUC", auc)
