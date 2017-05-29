#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017
# Dennis Weyland

# A example very similar to grid_search.py except using Bayesian optimization instead of grid search.
# GPy and GPyOpt has to be installed.

import basf2_mva
import basf2_mva_util
import multiprocessing
import itertools
import pickle
import numpy as np

import GPy
import GPyOpt

if __name__ == "__main__":

    training_data = basf2_mva.vector("train.root")
    test_data = basf2_mva.vector("test.root")

    # Train model with default parameters
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    general_options.m_treename = "tree"
    general_options.m_identifier = "test.xml"
    general_options.m_variables = basf2_mva.vector('p', 'pz', 'daughter(0, Kid)', 'chiProb', 'M')
    general_options.m_target_variable = "isSignal"

    fastbdt_options = basf2_mva.FastBDTOptions()
    basf2_mva.teacher(general_options, fastbdt_options)

    # Load the model and train it again searching for the best hyperparameters
    def grid_search(hyperparameters):
        nTrees, depth = int(hyperparameters[0, 0]), int(hyperparameters[0, 1])
        method = basf2_mva_util.Method(general_options.m_identifier)
        options = basf2_mva.FastBDTOptions()
        options.m_nTrees = nTrees
        options.m_nLevels = depth
        m = method.train_teacher(training_data, general_options.m_treename, specific_options=options)
        p, t = m.apply_expert(test_data, general_options.m_treename)
        return -basf2_mva_util.calculate_roc_auc(p, t)

    bounds = [{'name': 'nTrees', 'type': 'continuous', 'domain': (10, 1000)},
              {'name': 'nLevels', 'type': 'discrete', 'domain': (2, 4, 5, 6)}]

    initial_guess = np.array([[200, 3]])
    initial_guess_results = np.array([[grid_search(initial_guess)]])

    num_parallel_processes = 5
    num_cycles = 6
    myBopt = GPyOpt.methods.BayesianOptimization(f=grid_search,
                                                 domain=bounds,
                                                 acquisition_type='MPI',
                                                 exact_feval=False,
                                                 batch_size=num_parallel_processes,
                                                 num_cores=num_parallel_processes,
                                                 evaluator_type='local_penalization',
                                                 initial_design_numdata=num_parallel_processes)

    # If you want to give the data in the constructor the number of initial guesses has to be >= initial_design_numdata
    myBopt.X = np.append(myBopt.X, initial_guess, axis=0)
    myBopt.Y = np.append(myBopt.Y, initial_guess_results, axis=0)

    for cycle in range(num_cycles):
        # run parallel optimization
        myBopt.run_optimization(1, max_time=np.inf, verbosity=False)
        # save some pics
        myBopt.plot_convergence('convergence_' + str(cycle) + '.png')
        myBopt.plot_acquisition('acquisition_' + str(cycle) + '.png')  # works only for 1D and 2D problems
        # save all points for continuing at a later time
        pickle.dump({'X': myBopt.X, 'Y': myBopt.Y}, open('opt.p', 'wb'))
