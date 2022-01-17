#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from torch.nn import Linear
from torch.nn import ReLU
from torch.nn import BatchNorm1d
from torch.nn.init import kaiming_uniform_
from torch.nn.init import xavier_uniform_
from torch.nn import Softmax
import torch
from torch.utils.data import Dataset
from torch.utils.data import random_split
from sklearn.preprocessing import LabelEncoder
from sklearn.preprocessing import PolynomialFeatures
import uproot3 as ur
import numpy as np


class PriorDataLoader(Dataset):
    """
    Dataloader for PID prior probability training.

    Attributes:
        x (np.array): Array containing feature data with a second order combination of momentum, cos(theta) and transverse momentum.
        y (np.array): Array containing the label encoded PDG values.

    """

    def __init__(self, path: str, key: str, particlelist: list, labels: list):
        """
        Initialize the dataloader for PID prior training.

        Parameters:
            path (str): Path to the root file containing the data.
            key (str): Key (i.e. path) of the tree within the root file.
            particlelist (list(int)): List of particle PDG values for which the model has to be trained.
            labels (str): Labels of pandas columns containing cos(theta), momentum and PDG values (in this order).

        """
        data = ur.open(path)
        data = data[key].pandas.df(labels)
        df = data.dropna().reset_index(drop=True)
        df.loc[:, labels[2]] = df.loc[:, labels[2]].abs()
        droplist = np.setdiff1d(np.unique(df[labels[2]].values), particlelist)
        for i in droplist:
            df = df.drop(df.loc[df[labels[2]] == i].index).reset_index(drop=True)
        x = df.values[:, 0:2]
        x = np.hstack((x, (np.sin(np.arccos(x[:, 0])) * x[:, 1]).reshape(-1, 1)))
        pol = PolynomialFeatures(2, include_bias=False)
        x = pol.fit_transform(x)
        #: The tensor of features.
        self.x = x.astype("float32")
        y = df.values[:, 2]
        le = LabelEncoder()
        y = le.fit_transform(y)
        #: The tensor of labels.
        self.y = y.astype("int64")

    def __getitem__(self, index):
        """
        Function to get feature and label tensors at the given index location.

        Parameters:
            index (int): The index of required tensors.

        Returns:
            Tensors of features and labels at the given index.
        """
        return [self.x[index], self.y[index]]

    def __len__(self):
        """
        Function to obtain length of a tensor.

        Parameters:
            None.

        Returns:
            Number of feature sets.
        """
        return len(self.x)

    def get_split(self, n_test: float = 0.1) -> torch.tensor:
        """
        Split the input data into training and validation set.

        Parameter:
            n_test (float): Ratio of number of particles to be taken in the validation set to that of training set.

        Return:
            A randomly split data set with the ratio given by 'n_test'.
        """
        test_size = round(n_test * len(self.x))
        train_size = len(self.x) - test_size
        return random_split(self, [train_size, test_size])


class PriorModel(torch.nn.Module):
    """
    Pytorch model for PID prior probability calculation.

    Attributes:
        hidden1: Linear layer with 9 inputs and 128 outputs.
        act1: An RELU activation layer.
        hidden2: A batch normalization layer.
        hidden3: Linear layer with 128 inputs and 64 outputs.
        act2: An RELU activation layer.
        hidden4: A batch normalization layer.
        hidden5: Linear layer with 64 inputs and 32 outputs.
        act3: An RELU activation layer.
        hidden6: A batch normalization layer.
        hidden7: Linear layer with 9 inputs and 128 outputs.
        act4: A softmax activation layer.

    """

    def __init__(self, n_output: int):
        """
        Initialize the PID prior probability model.

        Parameter:
            n_output (int): Number of output nodes.

        """
        super(PriorModel, self).__init__()
        #: Linear layer with 9 inputs and 128 outputs.
        self.hidden1 = Linear(9, 128)
        kaiming_uniform_(self.hidden1.weight, nonlinearity="relu")
        #: ReLU activation layer.
        self.act1 = ReLU()
        #: Batchnormalization layer.
        self.hidden2 = BatchNorm1d(128)
        #: Linear layer with 128 inputs and 64 outputs.
        self.hidden3 = Linear(128, 64)
        kaiming_uniform_(self.hidden3.weight, nonlinearity="relu")
        #: ReLU activation layer.
        self.act2 = ReLU()
        #: Batchnormalization layer.
        self.hidden4 = BatchNorm1d(64)
        #: Linear layer with 64 inputs and 32 outputs.
        self.hidden5 = Linear(64, 32)
        kaiming_uniform_(self.hidden5.weight, nonlinearity="relu")
        #: ReLU activation layer.
        self.act3 = ReLU()
        #: Batchnormalization layer.
        self.hidden6 = BatchNorm1d(32)
        #: Linear layer with 32 inputs and outputs for each particle in the particlelist.
        self.hidden7 = Linear(32, n_output)
        xavier_uniform_(self.hidden7.weight)
        #: Softmax activation layer.
        self.act4 = Softmax(dim=1)

    def forward(self, x: torch.tensor) -> torch.tensor:
        """
        Gives PID prior probabilities for the input features.

        Parameter:
            x (torch.tensor): A 2D tensor containing features for a particle as a row.

        Returns:
            A torch tensor containing PID prior probabilities for the provided features.
        """
        x = self.hidden1(x)
        x = self.act1(x)
        x = self.hidden2(x)
        x = self.hidden3(x)
        x = self.act2(x)
        x = self.hidden4(x)
        x = self.hidden5(x)
        x = self.act3(x)
        x = self.hidden6(x)
        x = self.hidden7(x)
        x = self.act4(x)
        return x
