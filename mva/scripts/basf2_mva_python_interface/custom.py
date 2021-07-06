#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


def feature_importance(state):
    """
    Assert
    """
    raise RuntimeError("You have to overwrite feature_importance if you use the custom framework")


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Assert
    """
    raise RuntimeError("You have to overwrite get_model if you use the custom framework")


def load(obj):
    """
    Assert
    """
    raise RuntimeError("You have to overwrite load if you use the custom framework")


def apply(state, X):
    """
    Assert
    """
    raise RuntimeError("You have to overwrite apply if you use the custom framework")


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """
    Assert
    """
    raise RuntimeError("You have to overwrite begin_fit if you use the custom framework")


def partial_fit(state, X, S, y, w, epoch):
    """
    Assert
    """
    raise RuntimeError("You have to overwrite partial_fit if you use the custom framework")


def end_fit(state):
    """
    Assert
    """
    raise RuntimeError("You have to overwrite end_fit if you use the custom framework")
