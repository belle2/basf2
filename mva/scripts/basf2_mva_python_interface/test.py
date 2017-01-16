#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import numpy as np


class State(object):
    """
    Test state
    """
    def __init__(self):
        """ Constructor of the state object """
        pass


def feature_importance(state):
    """
    Return a list containing the feature importances
    """
    return []


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Return default test model
    """
    print("Called get_model")
    print(parameters)
    return State()


def load(obj):
    """
    Test load function
    """
    print("Called load")
    print(obj)
    return State()


def apply(state, X):
    """
    Test apply function
    """
    print("Called apply")
    print(state, X)
    return np.zeros(len(X))


def begin_fit(state):
    """
    Test begin_fit function
    """
    print("Called begin_fit")
    print(state)
    return state


def partial_fit(state, X, S, y, w, Xtest, Stest, ytest, wtest, epoch):
    """
    Test partial_fit function
    """
    print("Called partial_fit")
    print(state, X, S, y, w, Xtest, Stest, ytest, wtest, epoch)
    return True


def end_fit(state):
    """
    Test end_fit function
    """
    print("Called end_fit")
    print(state)
    return "SerializedState"
