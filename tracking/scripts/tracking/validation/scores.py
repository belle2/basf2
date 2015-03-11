import numpy as np


# Functions dependent on the data model
def data_amount(truths, predictions):
    n_predictions = len(predictions)
    n_truths = len(truths)

    if n_predictions != n_truths:
        raise ValueError("Prediction and truth do not represent the same amount of data.")

    return n_predictions


def signal_amount(truths, predictions):
    return np.count_nonzero(truths)


def accepted_amount(truths, predictions):
    return np.count_nonzero(predictions)


def accepted_signal_amount(truths, predictions):
    return np.count_nonzero(predictions * truths)


# Functions independent of the data model

# Amounts #
# ####### #

def background_amount(truths, predictions):
    return data_amount(truths, predictions) - signal_amount(truths, predictions)


def rejected_amount(truths, predictions):
    return data_amount(truths, predictions) - accepted_amount(truths, predictions)


def rejected_signal_amount(truths, predictions):
    return signal_amount(truths, predictions) - accepted_signal_amount(truths, predictions)


def accepted_background_amount(truths, predictions):
    return accepted_amount(truths, predictions) - accepted_signal_amount(truths, predictions)


def rejected_background_amount(truths, predictions):
    return background_amount(truths, predictions) - accepted_background_amount(truths, predictions)


# Ratios #
# ###### #

def purity(truths, predictions):
    return np.divide(1.0 * accepted_signal_amount(truths, predictions), accepted_amount(truths, predictions))


def efficiency(truths, predictions):
    return np.divide(1.0 * accepted_signal_amount(truths, predictions), signal_amount(truths, predictions))


def accuracy(truths, predictions):
    n_correct = accepted_signal_amount(truths, predictions) + rejected_background(truths, predictions)
    n_data = data_amount(truths, predictions)
    return np.divide(1.0 * n_correct, n_data)


def background_rejection(truths, predictions):
    return np.divide(1.0 * rejected_background_amount(truths, predictions), background_amount(truths, predictions))


def signal_background_ratio(truths, predictions):
    n_data = data_amount(truths, predictions)
    n_signal = signal_amount(truths, predictions)
    return np.divide(1.0 * n_signal, n_data - n_signal)
