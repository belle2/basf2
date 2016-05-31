#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import numpy as np


class State(object):

    def __init__(self):
        pass


def get_model(number_of_features, number_of_events, parameters):
    print("Called get_model")
    print(parameters)
    return State()


def load(obj):
    print("Called load")
    print(obj)
    return State()


def apply(state, X):
    print("Called apply")
    print(state, X)
    return np.zeros(len(X))


def begin_fit(state):
    print("Called begin_fit")
    print(state)
    return state


def partial_fit(state, X, y, w, Xtest, ytest, wtest, epoch):
    print("Called partial_fit")
    print(state, X, y, w, Xtest, ytest, wtest, epoch)
    return True


def end_fit(state):
    print("Called end_fit")
    print(state)
    return "SerializedState"
