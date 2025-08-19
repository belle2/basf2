#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This example creates the file ModelForStandalone.onnx used in tests

import torch
from torch import nn


class Model(nn.Module):
    def __init__(self):
        super().__init__()
        self.linear = nn.Linear(8, 2)

    def forward(self, a, b):
        return self.linear(torch.cat([a, b.reshape(-1, 6).float()], axis=1))


if __name__ == "__main__":
    model = Model()
    model.load_state_dict({
        "linear.weight": torch.tensor(
            [[0.0040, -0.1127, -0.0641, 0.0129, -0.0216, 0.2783, -0.0190, -0.0011],
             [-0.0772, -0.2133, -0.0243, 0.1520, 0.0784, 0.1187, -0.1681, 0.0372]]
        ),
        "linear.bias": torch.tensor([-0.2196, 0.1375]),
    })
    a = torch.tensor([[0.5309, 0.4930]])
    b = torch.tensor([[[1, 0, 1], [1, -1, 0]]])

    torch.set_printoptions(precision=10)
    print("Outputs to test against:", model(a, b))

    torch.onnx.export(
        model,
        (a, b),
        "ModelForStandalone.onnx",
        input_names=["a", "b"],
        output_names=["output"],
    )
