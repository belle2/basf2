#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import numpy as np

try:
    import xgboost as xgb
except ImportError:
    print("Please install xgboost: pip3 install xgboost")
    import sys
    sys.exit(1)

import os
import tempfile
import collections


class State(object):
    """
    XGBoost state
    """
    def __init__(self, num_round=0, parameters=None):
        """ Constructor of the state object """
        #: Parameters passed to xgboost model
        self.parameters = parameters
        #: Number of boosting rounds used in xgboost training
        self.num_round = num_round
        #: XGBoost estimator
        self.estimator = None


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Return default xgboost model
    """
    param = {'bst:max_depth': 2, 'bst:eta': 1, 'silent': 1, 'objective': 'binary:logistic'}
    nTrees = 100
    if 'nTrees' in parameters:
        nTrees = parameters['nTrees']
        del parameters['nTrees']
    if isinstance(parameters, collections.Mapping):
        param.update(parameters)
    return State(nTrees, param)


def feature_importance(state):
    """
    Return a list containing the feature importances
    """
    return []


def load(obj):
    """
    Load XGBoost estimator into state
    """
    state = State()
    f = tempfile.NamedTemporaryFile(delete=False)
    f.write(obj)
    f.close()
    state.estimator = xgb.Booster({})
    state.estimator.load_model(f.name)
    os.unlink(f.name)
    return state


def apply(state, X):
    """
    Apply estimator to passed data.
    """
    data = xgb.DMatrix(X)
    result = state.estimator.predict(data)
    return np.require(result, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """
    Initialize lists which will store the received data
    """
    state.X = []
    state.y = []
    state.w = []
    state.Xtest = Xtest
    state.ytest = ytest.flatten()
    state.wtest = wtest.flatten()
    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Stores received training data.
    XGBoost is usually not able to perform a partial fit.
    """
    state.X.append(X)
    state.y.append(y.flatten())
    state.w.append(w.flatten())
    return True


def end_fit(state):
    """
    Merge received data together and fit estimator
    """
    dtrain = xgb.DMatrix(np.vstack(state.X), label=np.hstack(state.y).astype(int), weight=np.hstack(state.w))

    if len(state.Xtest) > 0:
        dtest = xgb.DMatrix(state.Xtest, label=state.ytest.astype(int), weight=state.wtest)
        evallist = [(dtest, 'eval'), (dtrain, 'train')]
    else:
        evallist = [(dtrain, 'train')]

    state.estimator = xgb.train(state.parameters, dtrain, state.num_round, evallist)
    f = tempfile.NamedTemporaryFile(delete=False)
    f.close()
    state.estimator.save_model(f.name)
    with open(f.name, 'rb') as f2:
        content = f2.read()
    os.unlink(f.name)
    return content
