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
import numpy as np
import torch
from torch.utils.data import TensorDataset, DataLoader


class State(object):
    """
    torch state
    """

    def __init__(self, model=None, **kwargs):
        """ Constructor of the state object """
        #: torch model
        self.model = model

        #: list of keys to save
        self.collection_keys = []

        # other possible things to save that are needed by the model
        for key, value in kwargs.items():
            self.collection_keys.append(key)
            setattr(self, key, value)


def feature_importance(state):
    """
    Return a list containing the feature importances.
    Torch does not provide feature importances so return an empty list.
    """
    return []


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Returns default torch model
    """

    class myModel(torch.nn.Model):
        def __init__(self):
            super(myModel).__init__()

            # a dense model with one hidden layer
            self.network = torch.nn.Sequential(
                torch.nn.Linear(number_of_features, 128),
                torch.nn.ReLU(),
                torch.nn.Linear(128, 128),
                torch.nn.ReLU(),
                torch.nn.Linear(128, 1),
                torch.nn.Sigmoid(),
            )

            self.loss = torch.nn.BCEntropy()
            self.optimizer = torch.optim.SGD

        def forward(self, x):
            prob = self.network(x)
            return prob

    state = State(myModel().to_device("cuda" if torch.cuda.is_available() else "cpu"))
    print(state.model)

    # one way to pass settings used during training
    state.learning_rate = parameters.get('learning_rate', 1e-3)
    state.batch_size = parameters.get('batch_size', 64)
    state.epochs = parameters.get('epochs', 10)
    state.testing_fraction = parameters.get('testing_fraction', 0.2)
    return state


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """
    Passes in a fraction of events if specific_options.m_training_fraction is set.
    """
    # these are taken to be the first N events.
    # It is better to split during partial_fit so we can ensure the events are shuffled.

    # state.Xtest = Xtest
    # state.Stest = Stest
    # state.ytest = ytest
    # state.wtest = wtest
    return state


def train_loop(dataloader, model, loss_fn, optimizer):
    size = len(dataloader.dataset)

    for batch, (X, y, w) in enumerate(dataloader):
        # Compute prediction and loss
        pred = model(X)
        loss = loss_fn(pred, y, w)

        # Backpropagation
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

        if batch % 100 == 0:
            loss, current = loss.item(), batch * len(X)
            print(f"loss: {loss:>7f}  [{current:>5d}/{size:>5d}]")


def test_loop(dataloader, model, loss_fn):
    size = len(dataloader.dataset)
    num_batches = len(dataloader)
    test_loss, correct = 0, 0

    with torch.no_grad():
        for X, y, w in dataloader:
            pred = model(X)
            test_loss += loss_fn(pred, y, w).item()
            correct += (pred.argmax(1) == y).type(torch.float).sum().item()

    test_loss /= num_batches
    correct /= size
    print(f"Test Error: \n Accuracy: {(100*correct):>0.1f}%, Avg loss: {test_loss:>8f} \n")


def partial_fit(state, X, S, y, w, epoch):
    """
    Pass received data to the torch model and train.

    Note: the `epoch` here refers actually to iBatch + nBatches * iEpoch
          if you pass a batch size and epoch number to the mva options.

          Here we assume that the following options are left at their default vale:
             specific_options.m_nIterations = -1
             specific_options.m_mini_batch_size = -1

          This means that all training events are passed to torch and torch handles
          the batching and epochs internally.

    This implementation follows closely https://pytorch.org/tutorials/beginner/basics/optimization_tutorial.html.
    """
    def weighted_loss(y_pred, y_true, weight):
        return (state.loss(y_pred, y_true)*weight).mean()

    # transform to torch tensor
    tensor_x = torch.Tensor(X)
    tensor_y = torch.Tensor(y)
    tensor_w = torch.Tensor(w)

    dataset = TensorDataset(tensor_x, tensor_y, tensor_w)

    # split into training and testing
    n_testing = np.round(state.testing_fraction * len(dataset))
    dataset, dataset_testing = torch.utils.data.random_split(dataset, (len(dataset) - n_testing, n_testing))

    dataloader = DataLoader(dataset)
    dataloader_testing = DataLoader(dataset_testing)

    optimizer = state.model.optimizer(state.model.parameters(), state.learning_rate)

    for t in range(state.epochs):
        print(f"Epoch {t+1}\n-------------------------------")
        train_loop(dataloader, state.model, weighted_loss, optimizer)
        test_loop(dataloader_testing, state.model, weighted_loss)
    return False


def apply(state, X):
    """
    Apply estimator to passed data.
    """
    r = state.model(torch.from_numpy(X)).numpy()
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

        model = torch.load(file_names[0])
        model.eval()  # sets dropout and batch norm layers to eval mode
        state = State(model())

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
        state.model.save(temp_path.joinpath('my_model.pt'))

        # this creates:
        # path/my_model.pt

        file_names = [f.relative_to(temp_path) for f in temp_path.rglob('*') if f.is_file()]
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
