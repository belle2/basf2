#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import basf2_mva
import basf2_mva_util
import time


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Create SKLearn classifier and store it in a State object
    """
    from sklearn.neural_network import MLPClassifier
    from basf2_mva_python_interface.sklearn import State
    if isinstance(parameters, collections.Mapping):
        clf = MLPClassifier(**parameters)
    elif isinstance(parameters, collections.Sequence):
        clf = MLPClassifier(*parameters)
    else:
        clf = MLPClassifier()
    return State(clf)


def end_fit(state):
    """
    Merge received data together and fit estimator.
    Neural network do not support weights at the moment (slearn 0.18.1).
    So these are ignored here!
    """
    state.estimator = state.estimator.fit(np.vstack(state.X), np.hstack(state.y))
    return state.estimator


if __name__ == "__main__":

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

    # Train a MVA method and directly upload it to the database
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_identifier = "SKLearn-NN"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    sklearn_nn_options = basf2_mva.PythonOptions()
    sklearn_nn_options.m_framework = "sklearn"
    sklearn_nn_options.m_steering_file = 'mva/examples/python/sklearn_mlpclassifier.py'
    param = '{"hidden_layer_sizes": [29], "activation": "logistic", "max_iter": 100, "solver": "adam", "batch_size": 100}'
    sklearn_nn_options.m_config = param
    sklearn_nn_options.m_normalize = True

    test_data = ["test.root"] * 10
    training_start = time.time()
    basf2_mva.teacher(general_options, sklearn_nn_options)
    training_stop = time.time()
    training_time = training_stop - training_start
    method = basf2_mva_util.Method(general_options.m_identifier)
    inference_start = time.time()
    p, t = method.apply_expert(basf2_mva.vector(*test_data), general_options.m_treename)
    inference_stop = time.time()
    inference_time = inference_stop - inference_start
    auc = basf2_mva_util.calculate_roc_auc(p, t)
    print("SKLearn", training_time, inference_time, auc)
