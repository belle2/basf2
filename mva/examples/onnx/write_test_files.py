#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This example creates the files
# - ModelForStandalone.onnx used in test for the standalone ONNX interface
# - ModelABToAB and ModelAToATwiceA to test the behaviour of the ONNX MVA interface for naming inputs/outputs

import torch
from torch import nn


class Model(nn.Module):
    """
    Example model with 2 different input tensors
    """

    def __init__(self):
        """
        Initialize with a single Linear layer
        """
        super().__init__()

        #: linear Layer with 8 inputs, 2 outputs
        self.linear = nn.Linear(8, 2)

    def forward(self, a, b):
        """
        Run the forward pass - a and b are concatenated and b is flattened
        """
        return self.linear(torch.cat([a, b.reshape(-1, 6).float()], axis=1))


class TrivialModel(nn.Module):
    """
    Wrapper class to create simple non-parametric models with multiple inputs and outputs for ONNX tests
    """

    def __init__(self, forward_fn):
        """
        Initialize with forward pass function passed to the constructor
        """
        super().__init__()

        #: forward pass function
        self.forward_fn = forward_fn

    def forward(self, *args):
        """
        Run the forward pass based on `forward_fn`
        """
        return self.forward_fn(*args)


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
    with torch.no_grad():
        print("Outputs to test against for ModelForStandalone.onnx:", model(a, b))
        torch.onnx.export(
            model,
            (a, b),
            "ModelForStandalone.onnx",
            input_names=["a", "b"],
            output_names=["output"],
        )
        torch.onnx.export(
            TrivialModel(lambda a, b: (a, b)),
            (torch.zeros(4), torch.zeros(4)),
            "ModelABToAB.onnx",
            input_names=["input_a", "input_b"],
            output_names=["output_a", "output_b"],
        )
        torch.onnx.export(
            TrivialModel(lambda a: (a, 2*a)),
            (torch.zeros(4)),
            "ModelAToATwiceA.onnx",
            input_names=["input_a"],
            output_names=["output_a", "output_twice_a"],
        )
