##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""This module contains score functions to quantify the quality of a classification

All score function have the signature
def score(truths, predictions):

comparing the truth information against a model and return a numerical value.
"""

import numpy as np


def data_amount(truths, predictions):
    """Score function: amount of data after a selection"""
    n_predictions = len(predictions)
    n_truths = len(truths)

    if n_predictions != n_truths:
        raise ValueError("Prediction and truth do not represent the same amount of data.")

    return n_predictions


def signal_amount(truths, predictions):
    """Score function: amount of signal of a classification"""
    return np.count_nonzero(truths)


def accepted_amount(truths, predictions):
    """Score function: amount accepted of a classification"""
    return np.count_nonzero(predictions)


def accepted_signal_amount(truths, predictions):
    """Score function: amount of accepted signal of a classification"""
    return np.count_nonzero(predictions * truths)


# Functions independent of the data model

# Amounts #
# ####### #

def background_amount(truths, predictions):
    """Score function: amount of background of a classification"""
    return data_amount(truths, predictions) - signal_amount(truths, predictions)


def rejected_amount(truths, predictions):
    """Score function: amount rejected of a classification"""
    return data_amount(truths, predictions) - accepted_amount(truths, predictions)


def rejected_signal_amount(truths, predictions):
    """Score function: amount of rejected signal of a classification"""
    return signal_amount(truths, predictions) - accepted_signal_amount(truths, predictions)


def accepted_background_amount(truths, predictions):
    """Score function: amount of accepted background of a classification"""
    return accepted_amount(truths, predictions) - accepted_signal_amount(truths, predictions)


def rejected_background_amount(truths, predictions):
    """Score function: amount of rejected background of a classification"""
    return background_amount(truths, predictions) - accepted_background_amount(truths, predictions)


# Ratios #
# ###### #

def purity(truths, predictions):
    """Score function: purity = accepted signal / accepted"""
    return np.divide(1.0 * accepted_signal_amount(truths, predictions), accepted_amount(truths, predictions))


def efficiency(truths, predictions):
    """Score function: efficiency = accepted signal / signal"""
    return np.divide(1.0 * accepted_signal_amount(truths, predictions), signal_amount(truths, predictions))


def accuracy(truths, predictions):
    """Score function: accuracy = (accepted signal + rejected background) / total"""
    n_correct = accepted_signal_amount(truths, predictions) + rejected_background_amount(truths, predictions)
    n_data = data_amount(truths, predictions)
    return np.divide(1.0 * n_correct, n_data)


def background_rejection(truths, predictions):
    """Score function: background rejection = rejected background / background"""
    n_background = background_amount(truths, predictions)
    n_rejected_background = rejected_background_amount(truths, predictions)
    return np.divide(1.0 * n_rejected_background, n_background)


def signal_background_ratio(truths, predictions):
    """Score function: background / signal"""
    n_data = data_amount(truths, predictions)
    n_signal = signal_amount(truths, predictions)
    n_background = n_data - n_signal
    return np.divide(1.0 * n_signal, n_background)
