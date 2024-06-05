#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import numpy as np
from basf2 import B2WARNING

try:
    import sklearn  # noqa
except ImportError:
    print("Please install sklearn: pip3 install sklearn")
    import sys
    sys.exit(1)

import collections


class State:
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
        x = state.estimator.predict_proba(X)
        if x.shape[1] == 2:
            x = state.estimator.predict_proba(X)[:, 1]
    else:
        x = state.estimator.predict(X)
    return np.require(x, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def begin_fit(state, Xtest, Stest, ytest, wtest, nBatches):
    """
    Initialize lists which will store the received data
    """
    state.X = []
    state.y = []
    state.w = []
    return state


def partial_fit(state, X, S, y, w, epoch, batch):
    """
    Stores received training data.
    SKLearn is usually not able to perform a partial fit.
    """
    if epoch > 0:
        B2WARNING("The sklearn training interface has been called with specific_options.m_nIterations > 1."
                  " This means duplicates of the training sample will be used during training.")

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
