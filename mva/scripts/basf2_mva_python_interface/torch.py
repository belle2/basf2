##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import pathlib
import tempfile
import numpy as np

import torch
import torch.nn as nn


class State:
    """
    PyTorch state
    """

    def __init__(self, model=None, **kwargs):
        """ Constructor of the state object """
        #: torch model
        self.model = model


class SimpleNN(nn.Module):
    """
    Simple neural network with one layer
    """

    def __init__(self, input_size, output_size):
        super(SimpleNN, self).__init__()
        self.linear = nn.Linear(input_size, output_size)

    def forward(self, x):
        x = self.linear(x)
        return x


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Return simple pytorch model
    """

    model = SimpleNN(number_of_features, 1)
    state = State(model)

    return state


def load(obj):
    """
    Load Pytorch model into state
    """

    f = tempfile.NamedTemporaryFile(delete=False)
    f.write(obj)
    f.close()

    state = State(torch.load(f.name))
    return state


def apply(state, X):
    """
    Apply estimator to passed data.
    """
    # convert array input to tensor to avoid creating a new graph for each input
    # calling the model directly is faster than using the predict method in most of our applications
    # as we do a loop over events.
    r = state.model(torch.tensor(np.atleast_2d(X), dtype=torch.float32)).detach().numpy()

    r = r[:, 1]  # cannot use squeeze because we might have output of shape [1,X classes]
    return np.require(r, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def begin_fit(state, Xtest, Stest, ytest, wtest, nBatches):
    """
    Returns just the state object
    """
    return state


def partial_fit(state, X, S, y, w, epoch, batch):
    """
    Do nothing..
    """
    return False


def end_fit(state):
    """
    Store torch model information
    """

    with tempfile.TemporaryDirectory() as temp_path:

        temp_path = pathlib.Path(temp_path)

        torch.save(state.model, temp_path.joinpath('my_model.pt'))

        with open(temp_path.joinpath('my_model.pt'), 'rb') as f:
            content = f.read()

    del state
    return content


def feature_importance(state):
    """
    Return a list containing the feature importances
    """
    return []
