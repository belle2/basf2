#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import numpy as np
import sklearn
import collections


class State(object):
    """
    SKLearn state
    """
    def __init__(self, estimator=None):
        self.estimator = estimator


def get_model(number_of_features, number_of_events, parameters):
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
        return state.estimator.predict_proba(X)[:, 1]
    return state.estimator.predict(X)


def begin_fit(state):
    """
    Initialize lists which will store the received data
    """
    state.X = []
    state.y = []
    state.w = []
    return state


def partial_fit(state, X, y, w, Xtest, ytest, wtest, epoch):
    """
    Stores received training data.
    SKLearn is usually not able to perform a partial fit.
    """
    state.X.append(X)
    state.y.append(y)
    state.w.append(w)
    return True


def end_fit(state):
    """
    Merge received data together and fit estimator
    """
    state.estimator = state.estimator.fit(np.vstack(state.X), np.hstack(state.y), np.hstack(state.w))
    return state.estimator
