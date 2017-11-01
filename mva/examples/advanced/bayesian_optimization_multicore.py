#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Markus Prim 2017
# Thomas Keck
# Dennis Weyland

# A simple example to use bayesian optimiation for the hyperparameters of a FastBDT.
# The package used in this example is https://github.com/scikit-optimize
# and can be installed with
# pip3 install scikit-optimize

# Training and test sample can be downloaded:
# http://ekpwww.ekp.kit.edu/~tkeck/train.root
# http://ekpwww.ekp.kit.edu/~tkeck/test.root


import basf2_mva
import basf2_mva_util
import skopt
from skopt.space import Real, Integer
from sklearn.externals.joblib import Parallel, delayed
import matplotlib.pyplot as plt


def f(x, general_options, process_number):
    """Returns the figure of merit for the optimization.
    The functions trains the classifier with the given hyperparamters on the training sample and
    calculates the AUC on the independet test sample.
    """
    g_options = general_options
    g_options.m_identifier = "test{}.xml".format(process_number)
    options = basf2_mva.FastBDTOptions()
    options.m_nTrees = int(x[0])
    options.m_nLevels = int(x[1])
    basf2_mva.teacher(g_options, options)
    m = basf2_mva_util.Method(g_options.m_identifier)
    p, t = m.apply_expert(test_data, general_options.m_treename)
    return -basf2_mva_util.calculate_roc_auc(p, t)


if __name__ == "__main__":

    training_data = basf2_mva.vector("train.root")
    test_data = basf2_mva.vector("test.root")

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector('p', 'pz', 'daughter(0, kaonID)', 'chiProb', 'M')
    general_options.m_target_variable = "isSignal"

    # init optimizer
    optimizer = skopt.Optimizer(dimensions=[Integer(10, 1000), Integer(2, 6)], n_initial_points=3)

    # calculate initial guess
    initial_guess = [10, 2]
    initial_res = f(initial_guess, general_options, 0)
    optimizer.tell(initial_guess, initial_res)

    # optimize
    for i in range(10):
        x = optimizer.ask(n_points=2)  # x is a list of n_points points
        y = Parallel(n_jobs=-1)(delayed(f)(v, general_options, index) for index, v in enumerate(x))  # evaluate points in parallel
        res = optimizer.tell(x, y)

    # Give some results
    print(res)
    skopt.plots.plot_convergence(res)
    plt.savefig('convergence.png')
    skopt.plots.plot_evaluations(res)
    plt.savefig('evaluations.png')
    skopt.plots.plot_objective(res)
    plt.savefig('objective.png')

    # Store result of optimization
    skopt.dump(res, 'opt-result.pkl')
