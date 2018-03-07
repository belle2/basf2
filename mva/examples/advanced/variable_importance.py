#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

# We want to find out which variables/features are the most important
# There are three approaches
# 1. You can use the variable importance estimate outputted by the method itself,
#    e.g. FastBDT and TMVA BDTs support calculating the variable importance using the information gain of each applied cut.
# 2. One can estimate the importance in a method-agnostic way, by training N times (where N is the number of variables),
#    each time another variable is removed from the training. The loss in ROC AUC score is used to estimate the importance.
#    This will underestimate the importance of variables whose information is highly correlated to other variables in the training.
# 3. One can estimate the importance in a method-agnostic way, by training N*N / 2 times (where N is the number of variables),
#    the first step is approach 2, afterwards the most-important variable given by approach 2 is removed and approach 2 is run
#    again on the remaining variables.
#    This will take the correlations of variables into account, but takes some time
#
# Approach 2 and 3 can be done in parallel (by using the multiprocessing module of python, see below)

import basf2_mva
import basf2_mva_util
import multiprocessing
import copy


if __name__ == "__main__":

    training_data = basf2_mva.vector("train.root")
    test_data = basf2_mva.vector("test.root")

    variables = ['M', 'p', 'pt', 'pz',
                 'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)',
                 'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)',
                 'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)',
                 'chiProb', 'dr', 'dz',
                 'daughter(0, dr)', 'daughter(1, dr)',
                 'daughter(0, dz)', 'daughter(1, dz)',
                 'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
                 'daughter(0, kaonID)', 'daughter(0, pionID)',
                 'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)', 'daughterInvariantMass(1, 2)']

    # Train model with default parameters
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    general_options.m_treename = "tree"
    general_options.m_identifier = "test.xml"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    fastbdt_options = basf2_mva.FastBDTOptions()
    basf2_mva.teacher(general_options, fastbdt_options)

    def roc_for_variable_set(variables):
        method = basf2_mva_util.Method(general_options.m_identifier)
        options = copy.copy(general_options)
        options.m_variables = basf2_mva.vector(*variables)
        m = method.train_teacher(training_data, general_options.m_treename, general_options=options)
        p, t = m.apply_expert(test_data, general_options.m_treename)
        return basf2_mva_util.calculate_roc_auc(p, t)

    method = basf2_mva_util.Method(general_options.m_identifier)
    p, t = method.apply_expert(test_data, general_options.m_treename)
    global_auc = basf2_mva_util.calculate_roc_auc(p, t)

    # Approach 1: Read out the importance calculted by the method itself
    print("Variable importances returned my method")
    for variable in method.variables:
        print(variable, method.importances.get(variable, 0.0))

    # Approach 2: Calculate the importance using the loss in AUC if a variable is removed
    p = multiprocessing.Pool(None, maxtasksperchild=1)
    results = p.map(roc_for_variable_set, [[v for v in method.variables if v != variable] for variable in method.variables])
    sorted_variables_with_results = list(sorted(zip(method.variables, results), key=lambda x: x[1]))
    print("Variable importances calculated using loss if variable is removed")
    for variable, auc in sorted_variables_with_results:
        print(variable, global_auc - auc)

    # Approach 3: Calculate the importance using the loss in AUC if a variable is removed recursively.
    removed_variables_with_results = sorted_variables_with_results[:1]
    remaining_variables = [v for v, r in sorted_variables_with_results[1:]]
    while len(remaining_variables) > 1:
        results = p.map(roc_for_variable_set,
                        [[v for v in remaining_variables if v != variable] for variable in remaining_variables])
        sorted_variables_with_results = list(sorted(zip(remaining_variables, results), key=lambda x: x[1]))
        removed_variables_with_results += sorted_variables_with_results[:1]
        remaining_variables = [v for v, r in sorted_variables_with_results[1:]]
    removed_variables_with_results += sorted_variables_with_results[1:]

    print("Variable importances calculated using loss if variables are recursively removed")
    last_auc = global_auc
    for variable, auc in removed_variables_with_results:
        print(variable, last_auc - auc)
        last_auc = auc
