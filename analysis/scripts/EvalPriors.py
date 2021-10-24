#!/usr/bin/env python3

# torch imports
import torch
from torch.nn.functional import one_hot

# other imports
import numpy as np
import pandas as pd
import uproot3 as ur
from sklearn.preprocessing import PolynomialFeatures

# Model
from PriorDataLoaderAndModel import PriorModel

# dataloader


def Dataload(data):
    x = np.sin(np.arccos(data[:, 0])) * data[:, 1]
    x = np.hstack((data, x.reshape(-1, 1)))
    pf = PolynomialFeatures(2, include_bias=False)
    x = pf.fit_transform(x)
    return torch.FloatTensor(x)
# scaling for calibration


def Scaling(dat):
    from netcal.scaling import TemperatureScaling
    data = ur.open(dat)["scale"].pandas.df()
    pdg_list = list(data.columns)
    pdg_list.remove('truth')
    sorted_list = np.sort([e[:-4] for e in pdg_list])
    y = data[sorted_list[0] + '_pdg'].values.reshape(-1, 1)
    for i in range(len(sorted_list) - 1):
        y = np.hstack((y, data[sorted_list[i + 1] + '_pdg'].values.reshape(-1, 1)))
    temp = TemperatureScaling()
    temp.fit(y, one_hot(torch.LongTensor(data["truth"].values)).numpy())
    return temp

# priorevaluation: takes list of pdgs as particlelist, path to the created
# model,scalefile as prescaling and PID as arguments for initializing


class Priors:
    def __init__(self, particlelist, Model, prescaling='none'):
        model = PriorModel(len(particlelist))
        model.load_state_dict(torch.load(Model))
        model.eval()
        if torch.cuda.is_available():
            model = model.to('cuda')
        self.model = model
        if prescaling != 'none':
            scale = Scaling(prescaling)
            self.scale = scale
            self.require_scale = True
        else:
            self.require_scale = False
        self.plist = np.sort(particlelist)

    def calculatePriors(self, momentum, cosTheta):
        y = Dataload(np.hstack((cosTheta.reshape(-1, 1), momentum.reshape(-1, 1))))
        if torch.cuda.is_available():
            y = y.to('cuda')
        out = self.model(y).to('cpu').detach().numpy()
        if self.require_scale:
            out = self.scale.transform(out)
        self.prior = out

    def getPriors(self, pdg='none'):
        if pdg != 'none':
            index = np.where(self.plist == pdg)[0][0]
            return self.prior[:, index]
        else:
            return self.prior

    def getPosterior(self, pid, pdg='none'):
        priorpid = np.multiply(self.prior, pid)
        sumprpid = np.sum(priorpid, axis=1)
        posterior = np.divide(priorpid, sumprpid.reshape(-1, 1))
        if pdg == 'none':
            return posterior
        else:
            index = np.where(self.plist == pdg)[0][0]
            return posterior[:, index]
