#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Markus Prim 2017
# Thomas Keck
# Dennis Weland

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
import matplotlib.pyplot as plt


def f(x):
    """Returns the figure of merit for the optimization.
    The functions trains the classifier with the given hyperparamters on the training sample and
    calculates the AUC on the independet test sample.
    """
    nTrees = int(x[0])
    depth = int(x[1])
    method = basf2_mva_util.Method(general_options.m_identifier)
    options = basf2_mva.FastBDTOptions()
    options.m_nTrees = nTrees
    options.m_nLevels = depth
    m = method.train_teacher(training_data, general_options.m_treename, specific_options=options)
    p, t = m.apply_expert(test_data, general_options.m_treename)
    return -basf2_mva_util.calculate_roc_auc(p, t)


if __name__ == "__main__":

    training_data = basf2_mva.vector("train.root")
    test_data = basf2_mva.vector("test.root")

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    general_options.m_treename = "tree"
    general_options.m_identifier = "test.xml"
    general_options.m_variables = basf2_mva.vector('p', 'pz', 'daughter(0, Kid)', 'chiProb', 'M')
    general_options.m_target_variable = "isSignal"

    fastbdt_options = basf2_mva.FastBDTOptions()
    basf2_mva.teacher(general_options, fastbdt_options)

    # Start optimization
    res = skopt.gp_minimize(f,  # the function to minimize
                            [(10, 1000), (2, 6)],  # the bounds on each dimension of x
                            x0=[10, 2],  # initial guess
                            n_calls=20)  # number of evaluations of f
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
