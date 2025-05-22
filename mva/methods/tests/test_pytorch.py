#!/usr/bin/env python3

from textwrap import dedent
import json
import unittest

import basf2
import basf2_mva
import basf2_mva_util

import numpy as np
import pandas as pd
import uproot


def write_dummy_file(variables, size=10, target_variable="mcPDG"):
    data = np.random.normal(size=[size, len(variables) + 1])
    tree = {}
    for i, name in enumerate(variables):
        tree[name] = data[:, i]
    tree[target_variable] = data[:, -1] > 0.5
    with uproot.recreate('dummy.root') as outfile:
        outfile['tree'] = pd.DataFrame(tree, dtype=np.float64)


class TestPyTorch(unittest.TestCase):
    def test_load_and_apply_existing_torch(self):
        method = basf2_mva_util.Method(basf2.find_file("mva/methods/tests/KLMMuonIDDNNWeightFile.xml"))
        write_dummy_file(list(method.general_options.m_variables))
        out1, out2 = method.apply_expert(
            method.general_options.m_datafiles, method.general_options.m_treename
        )
        self.assertEqual(out1.shape, (10,))
        self.assertEqual(out2.shape, (10,))

    def test_write_new_torch(self):
        variables = ["var1", "var2"]
        general_options = basf2_mva.GeneralOptions()
        general_options.m_datafiles = basf2_mva.vector("dummy.root")
        general_options.m_identifier = "Simple.xml"
        general_options.m_treename = "tree"
        general_options.m_variables = basf2_mva.vector(*variables)
        general_options.m_target_variable = "target"

        write_dummy_file(variables, size=100, target_variable=general_options.m_target_variable)

        specific_options = basf2_mva.PythonOptions()
        specific_options.m_framework = "torch"
        specific_options.m_steering_file = "dummy.py"
        specific_options.m_nIterations = 5
        specific_options.m_mini_batch_size = 8
        specific_options.m_config = json.dumps({"learning_rate": 1e-2})
        specific_options.m_training_fraction = 0.8
        specific_options.m_normalise = False

        with open("dummy.py", "w") as f:
            f.write(
                dedent(
                    """
                    import torch
                    from torch import nn

                    class Model(nn.Module):
                        def __init__(self, number_of_features):
                            super().__init__()
                            self.linear = nn.Linear(number_of_features, 1)

                        def forward(self, x):
                            return self.linear(x).sigmoid()


                    def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
                        state = State(Model(number_of_features).to("cpu"), number_of_features=number_of_features)
                        state.optimizer = torch.optim.SGD(state.model.parameters())
                        state.loss_fn = nn.BCELoss
                        state.epoch = 0
                        state.avg_costs = []
                        return state
                    """
                )
            )

        basf2_mva.teacher(general_options, specific_options)

        method = basf2_mva_util.Method(general_options.m_identifier)
        out1, out2 = method.apply_expert(
            method.general_options.m_datafiles, method.general_options.m_treename
        )
        self.assertEqual(out1.shape, (100,))
        self.assertEqual(out2.shape, (100,))


if __name__ == '__main__':
    import b2test_utils
    with b2test_utils.clean_working_directory():
        unittest.main()
