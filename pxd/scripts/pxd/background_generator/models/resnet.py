##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
"""
This module implements the ResNet generator model.
"""

import torch
import torch.nn as nn
import torch.nn.functional as F


##
# Class for the residual block layer.
class ResidualBlock(nn.Module):
    """Residual block layer."""

    ##
    # Constructor to create a new residual block layer.
    def __init__(self, ninput, noutput, upsample=True):
        super().__init__()
        self.upsample = upsample
        # shortcut branch
        self.conv = None
        if upsample or (ninput != noutput):
            self.conv = nn.Conv2d(ninput, noutput, 1, 1, 0)
        # residual branch
        self.norm1 = nn.BatchNorm2d(ninput)
        self.conv1 = nn.Conv2d(ninput, noutput, 3, 1, 1)
        self.norm2 = nn.BatchNorm2d(noutput)
        self.conv2 = nn.Conv2d(noutput, noutput, 3, 1, 1)

    ##
    # Function to perform a forward pass.
    def forward(self, x):
        """Compute the layer output for a given input."""
        # residual branch
        h = x
        h = self.norm1(h)
        h.relu_()
        if self.upsample:
            h = F.interpolate(h, mode="nearest", scale_factor=2)
        h = self.conv1(h)
        h = self.norm2(h)
        h.relu_()
        h = self.conv2(h)
        # shortcut branch
        if self.upsample:
            x = F.interpolate(x, mode="nearest", scale_factor=2)
        if self.conv:
            x = self.conv(x)
        # return sum of both
        return h + x

    ##
    # @var upsample
    # Whether to double the height and width of input

    ##
    # @var conv
    # Convolutional layer in the shortcut branch

    ##
    # @var norm1
    # First batch normalization layer in the residual branch

    ##
    # @var conv1
    # First convolutional layer in the residual branch

    ##
    # @var norm2
    # Second batch normalization layer in the residual branch

    ##
    # @var conv2
    # Second convolutional layer in the residual branch


##
# Class for the ResNet generator model.
class Model(nn.Module):
    """ResNet generator model."""

    ##
    # Constructor to create a new model instance.
    def __init__(self):
        super().__init__()
        # fully-connected inputs
        self.fc = nn.Linear(96, 49152)
        # stack of residual blocks
        self.blocks = nn.ModuleList(
            [
                # (256, 8, 24)
                ResidualBlock(256, 256),
                # (256, 16, 48)
                ResidualBlock(256, 128),
                # (128, 32, 96)
                ResidualBlock(128, 64),
                # (64, 64, 192)
                ResidualBlock(64, 32),
                # (32, 128, 384)
                ResidualBlock(32, 16),
                # (16, 256, 768)
            ]
        )
        # norm and conv outputs
        self.norm = nn.BatchNorm2d(16)
        self.conv = nn.Conv2d(16, 1, 3, 1, 1)

    ##
    # Function to perform a forward pass.
    def forward(self, z):
        """Compute the model output for a given input."""
        z = self.fc(z)
        z = z.view(-1, 256, 8, 24)
        for block in self.blocks:
            z = block(z)
        z = self.norm(z)
        z.relu_()
        z = self.conv(z)
        return z.tanh_()

    ##
    # @var fc
    # Fully-connected layer

    ##
    # @var blocks
    # Sequence of residual block layers

    ##
    # @var norm
    # Batch normalization layer

    ##
    # @var conv
    # Convolutional layer


##
# Function to produce one pseudo-random image for each PXD module
# using the ResNet generator model.
def generate(model):
    """Produce one pseudo-random image for each PXD module
    using the ResNet generator model.
    """
    # infer the device that is in use
    device = next(model.parameters()).device
    # without computing gradients
    with torch.no_grad():
        # initialize the model input(s)
        z = torch.randn(40, 96, device=device)
        # evaluate the model output and crop
        x = model(z)[:, 0, 3:-3, :]
        # delete the reference(s) to model input(s)
        del z
        # always transfer to the CPU
        x = x.cpu()
        # apply the inverse logarithmic transformation
        x = x.mul_(0.5).add_(0.5).clamp_(0.0, 1.0)
        x = torch.pow(256.0, x).sub_(1.0).clamp_(0.0, 255.0)
        # convert to unsigned 8-bit integer data type
        return x.to(torch.uint8)
