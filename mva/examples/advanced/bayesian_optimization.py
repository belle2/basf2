#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# A simple example to use bayesian optimization for the hyperparameters of a FastBDT.
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
    The functions trains the classifier with the given hyperparameters on the training sample and
    calculates the AUC on the independent test sample.
    """
    import ROOT  # noqa
    g_options = general_options
    g_options.m_identifier = "test.xml"
    options = ROOT.Belle2.MVA.FastBDTOptions()
    options.m_nTrees = int(x[0])
    options.m_nLevels = int(x[1])
    ROOT.Belle2.MVA.teacher(g_options, options)
    m = basf2_mva_util.Method(g_options.m_identifier)
    p, t = m.apply_expert(test_data, general_options.m_treename)
    return -basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)


if __name__ == "__main__":

    import ROOT  # noqa

    training_data = basf2_mva.vector("train.root")
    test_data = basf2_mva.vector("test.root")

    general_options = ROOT.Belle2.GeneralOptions()
    general_options.m_datafiles = training_data
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector('p', 'pz', 'daughter(0, kaonID)', 'chiProb', 'M')
    general_options.m_target_variable = "isSignal"

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
