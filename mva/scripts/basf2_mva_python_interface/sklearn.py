#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Thomas Keck 2016

import numpy as np

try:
    import sklearn  # noqa
except ImportError:
    print("Please install sklearn: pip3 install sklearn")
    import sys
    sys.exit(1)

import collections


class State(object):
    """
    SKLearn state
    """

    def __init__(self, estimator=None):
        """ Constructor of the state object """
        #: Pickable sklearn estimator
        self.estimator = estimator


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Create SKLearn classifier and store it in a State object
    """
    from sklearn.ensemble import GradientBoostingClassifier
    if isinstance(parameters, collections.Mapping):
        clf = GradientBoostingClassifier(**parameters)
    elif isinstance(parameters, collections.Sequence):
        clf = GradientBoostingClassifier(*parameters)
    else:
        clf = GradientBoostingClassifier()
    return State(clf)


def feature_importance(state):
    """
    Return a list containing the feature importances
    """
    from sklearn.ensemble import GradientBoostingClassifier
    if isinstance(state.estimator, GradientBoostingClassifier):
        return [x for x in state.estimator.feature_importances_]
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
    state.y = []
    state.w = []
    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Stores received training data.
    SKLearn is usually not able to perform a partial fit.
    """
    state.X.append(X)
    state.y.append(y.flatten())
    state.w.append(w.flatten())
    return True


def end_fit(state):
    """
    Merge received data together and fit estimator
    """
    state.estimator = state.estimator.fit(np.vstack(state.X), np.hstack(state.y), np.hstack(state.w))
    return state.estimator
