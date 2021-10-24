#!/usr/bin/env python3

from torch.nn import Linear
from torch.nn import ReLU
from torch.nn import BatchNorm1d
from torch.nn.init import kaiming_uniform_
from torch.nn.init import xavier_uniform_
from torch.nn import Sequential
from torch.nn import BatchNorm2d
from torch.nn import Softmax
import torch
from torch.utils.data import Dataset
from torch.utils.data import random_split
from sklearn.preprocessing import LabelEncoder
from sklearn.preprocessing import PolynomialFeatures
import uproot3 as ur
import numpy as np


class PriorDataLoader(Dataset):
    def __init__(self, path, key, particlelist, labels):
        data = ur.open(path)
        data = data[key].pandas.df(labels)
        df = data.dropna().reset_index(drop=True)
        df.loc[:, labels[2]] = df.loc[:, labels[2]].abs()
        droplist = [e for e in np.unique(df[labels[2]].values) if e not in particlelist]
        for i in droplist:
            df = df.drop(df.loc[df[labels[2]] == i].index).reset_index(drop=True)
        self.x = df.values[:, 0:2]
        self.x = np.hstack((self.x, (np.sin(np.arccos(self.x[:, 0])) * self.x[:, 1]).reshape(-1, 1)))
        pol = PolynomialFeatures(2, include_bias=False)
        self.x = pol.fit_transform(self.x)
        self.y = df.values[:, 2]
        self.x = self.x.astype('float32')
        le = LabelEncoder()
        self.y = le.fit_transform(self.y)
        self.y = self.y.astype('int64')

    def __getitem__(self, index):
        return [self.x[index], self.y[index]]

    def __len__(self):
        return len(self.x)

    def get_split(self, n_test=0.1):
        test_size = round(n_test * len(self.x))
        train_size = len(self.x) - test_size
        return random_split(self, [train_size, test_size])


class PriorModel(torch.nn.Module):
    def __init__(self, n_output):
        super(PriorModel, self).__init__()
        self.hidden1 = Linear(9, 128)
        kaiming_uniform_(self.hidden1.weight, nonlinearity='relu')
        self.act1 = ReLU()
        self.hidden2 = BatchNorm1d(128)
        self.hidden3 = Linear(128, 64)
        kaiming_uniform_(self.hidden3.weight, nonlinearity='relu')
        self.act2 = ReLU()
        self.hidden4 = BatchNorm1d(64)
        self.hidden5 = Linear(64, 32)
        kaiming_uniform_(self.hidden5.weight, nonlinearity='relu')
        self.act3 = ReLU()
        self.hidden6 = BatchNorm1d(32)
        self.hidden7 = Linear(32, n_output)
        xavier_uniform_(self.hidden7.weight)
        self.act4 = Softmax(dim=1)

    def forward(self, x):
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
