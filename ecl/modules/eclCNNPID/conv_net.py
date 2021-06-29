# !/usr/bin/env python3
# -*- coding: utf-8 -*-

import torch
import numpy as np
import torch.nn as nn
import torch.nn.functional as F

__author__ = 'Abtin Narimani Charan'
__copyright__ = 'Copyright 2020 - Belle II Collaboration'
__maintainer__ = 'Abtin Narimani Charan'
__email__ = 'abtin.narimani.charan@desy.de'
__updated__ = '31.05.2021'


def findConv2dOutShape(
    H_in,
    W_in,
    conv,
    pool
):
    # get conv argument
    kernel_size = conv.kernel_size
    stride = conv.stride
    padding = conv.padding
    dilation = conv.dilation

    # Ref: https://pytorch.org/docs/stable/nn.html
    H_out = np.floor(
        (H_in + 2 * padding[0] - dilation[0] * (kernel_size[0] - 1) - 1) / stride[0] + 1)
    W_out = np.floor(
        (W_in + 2 * padding[1] - dilation[1] * (kernel_size[1] - 1) - 1) / stride[1] + 1)

    if pool:
        H_out /= pool
        W_out /= pool
    return(int(H_out),
           int(W_out))


class ConvNet(nn.Module):

    def __init__(self, params):

        super(ConvNet, self).__init__()
        C_in, H_in, W_in = params['input_shape']
        num_emb_theta = params['num_emb_theta']
        dim_emb_theta = params['dim_emb_theta']
        num_emb_phi = params['num_emb_phi']
        dim_emb_phi = params['dim_emb_phi']
        num_ext_input = params['num_ext_input']
        init_f = params['initial_filters']
        num_fc1 = params['num_fc1']
        num_classes = params['num_classes']
        self.dropout_rate = params['dropout_rate']
        C_in_array = np.array(
            [True]
        )
        count_C_in = np.count_nonzero(C_in_array)

        self.emb_theta = nn.Embedding(num_emb_theta, dim_emb_theta)
        self.emb_phi = nn.Embedding(num_emb_phi, dim_emb_phi)

        self.conv1 = nn.Conv2d(C_in, init_f, kernel_size=3, padding=1, stride=1)
        h, w = findConv2dOutShape(H_in, W_in, self.conv1, pool=1)

        # compute the flatten size
        self.num_flatten = h * w * init_f

        self.fc1 = nn.Linear(
            self.num_flatten * count_C_in + num_ext_input + dim_emb_theta + dim_emb_phi,
            num_fc1)

        self.fc2 = nn.Linear(num_fc1, num_classes)

    def forward(self,
                energy,
                theta_input,
                phi_input,
                extra_input):

        x1 = F.relu(self.conv1(energy))
        x1 = x1.view(-1, self.num_flatten)

        x = torch.cat(
            (x1,
             extra_input,
             self.emb_theta(theta_input),
             self.emb_phi(phi_input)),
            dim=1
        )

        x = F.relu(self.fc1(x))
        x = F.dropout(x, self.dropout_rate, training=self.training)
        output = self.fc2(x)
        return(output)
