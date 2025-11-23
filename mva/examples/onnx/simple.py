#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
from tempfile import TemporaryDirectory

import basf2  # noqa
import torch
from torch import nn
import numpy as np
import uproot

import ROOT


class Model(nn.Module):
    """
    My dense neural network
    """

    def __init__(self, number_of_features):
        """
        Parameters:
          number_of_features: number of input features
        """
        super().__init__()

        #: a dense model with one hidden layer
        self.network = nn.Sequential(
            nn.Linear(number_of_features, 128),
            nn.ReLU(),
            nn.Linear(128, 128),
            nn.ReLU(),
            nn.Linear(128, 1),
            nn.Sigmoid(),
        )

    def forward(self, x):
        """
        Run the network
        """
        prob = self.network(x)
        return prob


def fit(model, filename, treename, variables, target_variable):
    with uproot.open({filename: treename}) as tree:
        X = tree.arrays(
            map(ROOT.Belle2.MakeROOTCompatible.makeROOTCompatible, variables),
            library="pd",
        ).to_numpy()
        y = tree[target_variable].array(library="np")
    ds = torch.utils.data.TensorDataset(
        torch.tensor(X, dtype=torch.float32),
        torch.tensor(y, dtype=torch.float32)[:, np.newaxis]
    )
    dl = torch.utils.data.DataLoader(ds, batch_size=256, shuffle=True)
    opt = torch.optim.Adam(model.parameters())
    for epoch in range(50):
        print(f"Epoch {epoch}", end=", ")
        losses = []
        for bx, by in dl:
            opt.zero_grad()
            p = model(bx)
            loss = torch.nn.functional.binary_cross_entropy(p, by)
            loss.backward()
            opt.step()
            losses.append(loss.detach().item())
        print(f"Loss = {np.mean(losses)}", end="\r")
    print()


def save_onnx_to_database(model, general_options, specific_options, identifier):
    with TemporaryDirectory() as tempdir:
        cwd = os.getcwd()
        os.chdir(tempdir)
        print("convert to onnx")
        torch.onnx.export(
            model,
            (torch.randn(1, len(general_options.m_variables)),),
            "model.onnx",
            input_names=["input"],
            output_names=["output"],
        )
        wf = ROOT.Belle2.MVA.Weightfile()
        general_options.m_method = specific_options.getMethod()
        wf.addOptions(general_options)
        wf.addOptions(specific_options)
        wf.addFile("ONNX_Modelfile", "model.onnx")
        os.chdir(cwd)
        print("save to database")
        ROOT.Belle2.MVA.Weightfile.saveToDatabase(wf, identifier)


if __name__ == "__main__":
    import time

    import basf2_mva
    import basf2_mva_util
    from basf2 import conditions
    from basf2 import find_file

    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    train_file = find_file("mva/train_D0toKpipi.root", "examples")
    test_file = find_file("mva/test_D0toKpipi.root", "examples")

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector(train_file)
    general_options.m_identifier = "Simple"
    general_options.m_treename = "tree"
    variables = ['M', 'p', 'pt', 'pz',
                 'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)',
                 'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)',
                 'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)',
                 'chiProb', 'dr', 'dz',
                 'daughter(0, dr)', 'daughter(1, dr)',
                 'daughter(0, dz)', 'daughter(1, dz)',
                 'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
                 'daughter(0, kaonID)', 'daughter(0, pionID)',
                 'daughterInvM(0, 1)', 'daughterInvM(0, 2)', 'daughterInvM(1, 2)']
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    specific_options = basf2_mva.ONNXOptions()

    model = Model(len(variables))
    fit(
        model,
        train_file,
        general_options.m_treename,
        variables,
        general_options.m_target_variable,
    )
    save_onnx_to_database(model, general_options, specific_options, "ONNXTest")

    method = basf2_mva_util.Method("ONNXTest")
    inference_start = time.time()
    test_data = [test_file]
    p, t = method.apply_expert(basf2_mva.vector(*test_data), general_options.m_treename)
    inference_stop = time.time()
    inference_time = inference_stop - inference_start
    auc = basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)
    print("ONNX", inference_time, auc)
