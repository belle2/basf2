#!/usr/bin/env python3
import torch
import torch.nn as nn
import numpy as np
# This loss function calculates Taylor series of the known cross entropy
# loss. However, the user can specify zero (0) while choosing the number
# of terms (which is the default value for the Training file). This
# prevents outliers from having large impact on the training process.


def tcc(y, y_true, n):
    loss = torch.zeros(len(y_true))
    if torch.cuda.is_available():
        loss = loss.to('cuda')
    ProbTrue = y[np.arange(len(y_true)), y_true]
    if n != 0:
        for i in range(1, n + 1):
            loss += torch.pow(1 - ProbTrue, i) / i
    elif n == 0:
        loss = -1 * torch.log(ProbTrue)
    loss = torch.sum(loss)
    return loss / len(y)


class TCCE(nn.Module):
    def __init__(self, n=5):
        super(TCCE, self).__init__()
        self.n = n

    def forward(self, y, y_true):
        return tcc(y, y_true, self.n)
