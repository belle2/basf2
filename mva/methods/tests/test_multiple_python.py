#!/usr/bin/env python3

from textwrap import dedent
import unittest

import pandas as pd
import numpy as np
import uproot

import basf2_mva
import basf2_mva_util


def write_dummy_file(variables, size=10, target_variable="target"):
    data = np.random.normal(size=[size, len(variables) + 1])
    tree = {}
    for i, name in enumerate(variables):
        tree[name] = data[:, i]
    tree[target_variable] = data[:, -1] > 0.5
    with uproot.recreate('dummy.root') as outfile:
        outfile['tree'] = pd.DataFrame(tree, dtype=np.float64)


def train_bdt():
    with open("bdt.py", "w") as f:
        f.write(
            dedent(
                """
                def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
                    from sklearn.ensemble import GradientBoostingClassifier
                    clf = GradientBoostingClassifier()
                    return State(clf)

                def apply(state, X):
                    "redefine this on purpose to ensure we spot if it gets overwritten"
                    from sklearn.ensemble import GradientBoostingClassifier
                    if not isinstance(state.estimator, GradientBoostingClassifier):
                        raise TypeError(f"Wrong classifier, expected GradientBoostingClassifier, got {type(state.estimator)}")
                    assert isinstance(state.estimator, GradientBoostingClassifier), "Wrong classifier"
                    x = state.estimator.predict_proba(X)
                    return np.require(x, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])
                """
            )
        )
    variables = ["var1", "var2"]
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("dummy.root")
    general_options.m_treename = "tree"
    general_options.m_identifier = "SKLearn-BDT.xml"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "target"
    sklearn_nn_options = basf2_mva.PythonOptions()
    sklearn_nn_options.m_framework = "sklearn"
    sklearn_nn_options.m_steering_file = "bdt.py"
    basf2_mva.teacher(general_options, sklearn_nn_options)


def train_mlp():
    with open("mlp.py", "w") as f:
        f.write(
            dedent(
                """
                def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
                    from sklearn.neural_network import MLPClassifier
                    clf = MLPClassifier()
                    return State(clf)

                def end_fit(state):
                    state.estimator = state.estimator.fit(np.vstack(state.X), np.hstack(state.y))
                    return state.estimator

                def apply(state, X):
                    "redefine this on purpose to ensure we spot if it gets overwritten"
                    from sklearn.neural_network import MLPClassifier
                    if not isinstance(state.estimator, MLPClassifier):
                        raise TypeError(f"Wrong classifier, expected MLPClassifier, got {type(state.estimator)}")
                    x = state.estimator.predict_proba(X)
                    return np.require(x, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])
                """
            )
        )
    variables = ["var1", "var2"]
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("dummy.root")
    general_options.m_treename = "tree"
    general_options.m_identifier = "SKLearn-MLP.xml"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "target"
    sklearn_nn_options = basf2_mva.PythonOptions()
    sklearn_nn_options.m_framework = "sklearn"
    sklearn_nn_options.m_steering_file = "mlp.py"
    basf2_mva.teacher(general_options, sklearn_nn_options)


def get_expert(identifier):
    import ROOT

    method = basf2_mva_util.Method(identifier)
    ROOT.Belle2.MVA.AbstractInterface.initSupportedInterfaces()
    interfaces = ROOT.Belle2.MVA.AbstractInterface.getSupportedInterfaces()
    interface = interfaces["Python"]
    expert = interface.getExpert()
    expert.load(method.weightfile)
    return expert


class TestMultiplePython(unittest.TestCase):
    """
    Test if we can run multiple python methods for the same framework at the same time (MR !4244)
    """

    def test_multiple_sklearn(self):
        """
        Try if we can run a sklearn bdt and a sklear mlp at the same time
        """
        import ROOT
        from ROOT import vector

        variables = ["var1", "var2"]
        write_dummy_file(variables)
        train_bdt()
        train_mlp()

        # test by explicitly initializing two experts to provoke clashes
        bdt_expert = get_expert("SKLearn-BDT.xml")
        mlp_expert = get_expert("SKLearn-MLP.xml")

        inputs = vector[vector[float]]([vector[float]([1, 2])])
        spectators = vector[vector[float]]()

        general_options = basf2_mva.GeneralOptions()
        general_options.m_variables = basf2_mva.vector(*variables)
        dataset = ROOT.Belle2.MVA.MultiDataset(general_options, inputs, spectators)

        bdt_expert.apply(dataset)
        mlp_expert.apply(dataset)


if __name__ == '__main__':
    import b2test_utils
    with b2test_utils.clean_working_directory():
        unittest.main()
