#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import tempfile
import pathlib
import pickle
import numpy as np
import torch as pytorch
from basf2 import B2INFO


class State(object):
    """
    torch state
    """

    def __init__(self, model=None, **kwargs):
        """ Constructor of the state object """
        #: torch model
        self.model = model

        #: list of keys to save. Any attribute in this collection is written to the weightfile and recovered during loading.
        self.collection_keys = []

        # other possible things to save that are needed by the model
        for key, value in kwargs.items():
            self.collection_keys.append(key)
            setattr(self, key, value)


class PickleModule:
    """
    Custom PickleModule with a custom Unpickler that tries to find missing classes in the current global namespace.

    This is needed since the move to per-python-mva-method module instances in which the classes live now.
    """
    class Unpickler(pickle.Unpickler):
        def find_class(self, module, name):
            try:
                return super().find_class(module, name)
            except (ModuleNotFoundError, AttributeError):
                B2INFO(f"Missing class: {module}.{name}")
                if name in globals():
                    B2INFO(f"Using `{name}` from global namespace")
                    return globals()[name]
                else:
                    raise


def feature_importance(state):
    """
    Return a list containing the feature importances.
    Torch does not provide feature importances so return an empty list.
    """
    return []


class myModel(pytorch.nn.Module):
    """
    My dense neural network
    """

    def __init__(self, number_of_features):
        """
        Init the network
        param: number_of_features number of input variables
        """
        super(myModel, self).__init__()

        #: a dense model with one hidden layer
        self.network = pytorch.nn.Sequential(
            pytorch.nn.Linear(number_of_features, 128),
            pytorch.nn.ReLU(),
            pytorch.nn.Linear(128, 128),
            pytorch.nn.ReLU(),
            pytorch.nn.Linear(128, 1),
            pytorch.nn.Sigmoid(),
        )

    def forward(self, x):
        """
        Run the network
        """
        prob = self.network(x)
        return prob


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Returns default torch model
    """

    state = State(myModel(number_of_features).to("cpu"))
    print(state.model)

    if parameters is None:
        parameters = {}

    state.optimizer = pytorch.optim.SGD(state.model.parameters(), parameters.get('learning_rate', 1e-3))

    # we recreate the loss function on each batch so that we can pass in the weights
    # this is a weird feature of how torch handles event weights
    state.loss_fn = pytorch.nn.BCELoss

    return state


def begin_fit(state, Xtest, Stest, ytest, wtest, nBatches):
    """
    Passes in a fraction of events if specific_options.m_training_fraction is set.
    """
    # transform to torch tensor and store the validation sample for later use
    device = "cpu"
    state.Xtest = pytorch.from_numpy(Xtest).to(device)
    state.ytest = pytorch.from_numpy(ytest).to(device)
    state.wtest = pytorch.from_numpy(wtest).to(device)
    return state


def partial_fit(state, X, S, y, w, epoch, batch):
    """
    Pass received data to the torch model and train.

    The epochs and batching are handled by the mva package.
    If you prefer to do this yourself set
             specific_options.m_nIterations = 1
             specific_options.m_mini_batch_size = 0
    which will pass all training data as a single batch once.
    This can then be loaded into torch in any way you want.
    """
    # transform to torch tensor
    device = "cpu"
    tensor_x = pytorch.from_numpy(X).to(device)
    tensor_y = pytorch.from_numpy(y).to(device).type(pytorch.float)
    tensor_w = pytorch.from_numpy(w).to(device)

    # Compute prediction and loss
    loss_fn = state.loss_fn(weight=tensor_w)
    pred = state.model(tensor_x)
    loss = loss_fn(pred, tensor_y)

    # Backpropagation
    state.optimizer.zero_grad()
    loss.backward()
    state.optimizer.step()

    if batch == 0 and epoch == 0:
        state.avg_costs = [loss.detach().numpy()]
        state.epoch = epoch
    elif epoch != state.epoch:
        # we are at the start of a new epoch, print out details of the last epoch
        if len(state.ytest) > 0:
            # run the validation set:
            state.model.eval()
            with pytorch.no_grad():
                test_pred = state.model(state.Xtest)
            test_loss_fn = state.loss_fn(weight=state.wtest)
            test_loss = test_loss_fn(test_pred, state.ytest).item()
            test_correct = (test_pred.round() == state.ytest).type(pytorch.float).sum().item()

            print(f"Epoch: {epoch-1:04d},\t Training Cost: {np.mean((state.avg_costs)):.4f},"
                  f"\t Testing Cost: {test_loss:.4f}, \t Testing Accuracy: {test_correct/len(state.ytest)}")
            state.model.train()
        else:
            print(f"Epoch: {epoch-1:04d},\t Training Cost: {np.mean((state.avg_costs)):.4f}")

        state.avg_costs = [loss.detach().numpy()]
        state.epoch = epoch
    else:
        state.avg_costs.append(loss.detach().numpy())

    if epoch == 100000:
        return False
    return True


def apply(state, X):
    """
    Apply estimator to passed data.
    """
    with pytorch.no_grad():
        r = state.model(pytorch.from_numpy(X)).detach().numpy()
    if r.shape[1] == 1:
        r = r[:, 0]  # cannot use squeeze because we might have output of shape [1,X classes]
    return np.require(r, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def load(obj):
    """
    Load the trained torch model into state.
    """
    with tempfile.TemporaryDirectory() as temp_path:
        temp_path = pathlib.Path(temp_path)

        file_names = obj[0]
        for file_index, file_name in enumerate(file_names):
            path = temp_path.joinpath(pathlib.Path(file_name))
            path.parents[0].mkdir(parents=True, exist_ok=True)

            with open(path, 'w+b') as file:
                file.write(bytes(obj[1][file_index]))

        model = pytorch.load(temp_path.joinpath(file_names[0]), pickle_module=PickleModule)
        model.eval()  # sets dropout and batch norm layers to eval mode
        device = "cpu"
        model.to(device)
        state = State(model)

    # load everything else we saved
    for index, key in enumerate(obj[2]):
        setattr(state, key, obj[3][index])
    return state


def end_fit(state):
    """
    Store torch model
    """
    with tempfile.TemporaryDirectory() as temp_path:

        temp_path = pathlib.Path(temp_path)

        # this creates:
        # path/my_model.pt
        pytorch.save(state.model, temp_path.joinpath('my_model.pt'))

        file_names = ['my_model.pt']
        files = []
        for file_name in file_names:
            with open(temp_path.joinpath(file_name), 'rb') as file:
                files.append(file.read())

        collection_keys = state.collection_keys
        collections_to_store = []
        for key in state.collection_keys:
            collections_to_store.append(getattr(state, key))

    del state
    return [file_names, files, collection_keys, collections_to_store]
