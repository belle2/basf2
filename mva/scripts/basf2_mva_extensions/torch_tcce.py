#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import torch
import torch.nn as nn
import numpy as np


def tcc(y: torch.tensor, y_true: torch.LongTensor, n: int) -> torch.tensor:
    """
    Calculates loss using the required number of Taylor terms of cross entropy loss.

    Parameters:
        y(torch.tensor): The probabilities predicted by ML model.
        y_true(torch.LongTensor): The truth values provided for training purposes (1D tensor).
        n(int): Number of terms to to be taken for the Taylor Series.

    Returns:
        A torch tesor with the value of the calculated Taylor cross entropy loss.

    Note:
        With n = 0, this returns the regular cross entropy loss.

    """
    loss = torch.zeros(len(y_true))
    if torch.cuda.is_available():
        loss = loss.to("cuda")
    ProbTrue = y[np.arange(len(y_true)), y_true]
    if n != 0:
        for i in range(1, n + 1):
            loss += torch.pow(1 - ProbTrue, i) / i
    elif n == 0:
        loss = -1 * torch.log(ProbTrue)
    loss = torch.sum(loss)
    return loss / len(y)


class TCCE(nn.Module):
    """
    Class for calculation of Taylor cross entropy loss.

    Attributes:
        n (int): Number of Taylor series terms to be used for loss calculation.

    """

    def __init__(self, n: int = 0):
        """
        Initialize the loss class.

        Parameters:
            n (int)(optional): Number of Taylor series terms to be used for loss calculation.

        """
        super().__init__()
        #: Number of Taylor terms.
        self.n = n

    def forward(self, y: torch.tensor, y_true: torch.LongTensor) -> torch.tensor:
        """
        Calculates the Taylor categorical cross entropy loss.

        Parameters:
            y(torch.tensor): Tensor containing the output of the model.
            y_true(torch.tensor): 1D tensor containing the truth value for a given set of features.

        Returns:
            The calculated loss as a torch tensor.
        """
        return tcc(y, y_true, self.n)
