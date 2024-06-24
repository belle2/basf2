#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import time

from basf2_mva_python_interface.torch import State
import torch


class myModel(torch.nn.Module):
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
        self.network = torch.nn.Sequential(
            torch.nn.Linear(number_of_features, 128),
            torch.nn.ReLU(),
            torch.nn.Linear(128, 128),
            torch.nn.ReLU(),
            torch.nn.Linear(128, 1),
            torch.nn.Sigmoid(),
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

    state.optimizer = torch.optim.SGD(state.model.parameters(), parameters.get('learning_rate', 1e-2))

    # we recreate the loss function on each batch so that we can pass in the weights
    state.loss_fn = torch.nn.BCELoss

    # for book keeping
    state.epoch = 0
    state.avg_costs = []
    return state


if __name__ == "__main__":
    from basf2 import conditions
    import basf2_mva
    import basf2_mva_util
    import json

    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
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

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "torch"
    specific_options.m_steering_file = 'mva/examples/torch/simple.py'
    # the number of training epochs
    specific_options.m_nIterations = 64
    specific_options.m_mini_batch_size = 256
    specific_options.m_config = json.dumps({'learning_rate': 1e-2})
    specific_options.m_training_fraction = 0.8
    specific_options.m_normalise = False

    training_start = time.time()
    basf2_mva.teacher(general_options, specific_options)
    training_stop = time.time()
    training_time = training_stop - training_start
    method = basf2_mva_util.Method(general_options.m_identifier)

    inference_start = time.time()
    test_data = ["test.root"]
    p, t = method.apply_expert(basf2_mva.vector(*test_data), general_options.m_treename)
    inference_stop = time.time()
    inference_time = inference_stop - inference_start
    auc = basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)
    print("Torch", training_time, inference_time, auc)
