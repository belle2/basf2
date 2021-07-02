#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import numpy as np


class State(object):
    """
    Test state
    """

    def __init__(self):
        """ Constructor of the state object """

    def __repr__(self):
        """ Returns representation of this state"""
        return 'State'


def feature_importance(state):
    """
    Return a list containing the feature importances
    """
    print("Called feature importance")
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
    print(state, X.shape)
    p = np.zeros(len(X))
    return np.require(p, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """
    Test begin_fit function
    """
    print("Called begin_fit")
    print(state, Xtest.shape, Stest.shape, ytest.shape, wtest.shape)
    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Test partial_fit function
    """
    print("Called partial_fit")
    print(state, X.shape, S.shape, y.shape, w.shape, epoch)
    return True


def end_fit(state):
    """
    Test end_fit function
    """
    print("Called end_fit")
    print(state)
    return "SerializedState"
