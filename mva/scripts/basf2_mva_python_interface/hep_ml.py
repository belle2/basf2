#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Thomas Keck 2017

import numpy as np

try:
    import pandas
except ImportError:
    print("Please install pandas: pip3 install pandas")
    import sys
    sys.exit(1)

try:
    import hep_ml
    import hep_ml.uboost
except ImportError:
    print("Please install hep_ml: pip3 install hep_ml")
    import sys
    sys.exit(1)

import collections


class State(object):
    """
    hep_ml state
    """
    def __init__(self, estimator=None):
        """ Constructor of the state object """
        #: Pickable sklearn estimator
        self.estimator = estimator


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Create hep_ml classifier and store it in a State object.
    The features are used as train_features in uboost and the spectators are used as uniform_features.
    You can pass additional parameters as a json-encoded string via m_config to the model.
    I assume that the parameters are passed as a dictionary,
        the key 'base_estimator' is passed to DecisionTreeClassifier as keyword arguments
        other keys are passed to uBoostClassifier as keyword arguments
    """
    if isinstance(parameters, collections.Mapping) and 'base_estimator' in parameters:
        base_tree = hep_ml.uboost.DecisionTreeClassifier(**parameters['base_estimator'])
        del parameters['base_estimator']
    else:
        base_tree = hep_ml.uboost.DecisionTreeClassifier(max_depth=3)

    train_features = list(range(number_of_features))
    uniform_features = [number_of_features + i for i in range(number_of_spectators)]

    if isinstance(parameters, collections.Mapping):
        if 'uniform_label' not in parameters:
            parameters['uniform_label'] = [0, 1]
        parameters['train_features'] = train_features
        parameters['uniform_features'] = uniform_features
        clf = hep_ml.uboost.uBoostClassifier(base_estimator=base_tree, **parameters)
    else:
        clf = hep_ml.uboost.uBoostClassifier(uniform_features=uniform_features, uniform_label=[0, 1],
                                             base_estimator=base_tree, train_features=train_features)
    return State(clf)


def feature_importance(state):
    """
    Return a list containing the feature importances
    """
    return []


def load(obj):
    """
    Load sklearn estimator into state
    """
    return State(obj)


def apply(state, X):
    """
    Apply estimator to passed data.
    If the estimator has a predict_proba it is called, otherwise call just predict.
    """
    X = pandas.DataFrame(X)
    if hasattr(state.estimator, 'predict_proba'):
        x = state.estimator.predict_proba(X)[:, 1]
    else:
        x = state.estimator.predict(X)
    return np.require(x, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def begin_fit(state, X, S, y, w):
    """
    Initialize lists which will store the received data
    """
    state.X = []
    state.S = []
    state.y = []
    state.w = []
    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Stores received training data.
    HepML is usually not able to perform a partial fit.
    """
    state.X.append(X)
    state.S.append(S)
    state.y.append(y.flatten())
    state.w.append(w.flatten())
    return True


def end_fit(state):
    """
    Merge received data together and fit estimator
    """
    X = pandas.DataFrame(np.hstack([np.vstack(state.X), np.vstack(state.S)]))
    state.estimator = state.estimator.fit(X, np.hstack(state.y), np.hstack(state.w))
    return state.estimator
