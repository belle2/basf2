"""

This module implements the ConvNet generator model.
"""

import torch
import torch.nn as nn


class Model(nn.Module):
    def __init__(self):
        super().__init__()
        self.fc = nn.Linear(96, 98304)
        self.features = nn.Sequential(
            # format: (channels, height, width)
            # (512, 8, 24)
            nn.Conv2d(512, 512, 5, 1, 2),
            nn.BatchNorm2d(512),
            nn.ReLU(inplace=True),
            nn.Upsample(scale_factor=2, mode="nearest"),
            # (512, 16, 48)
            nn.Conv2d(512, 256, 5, 1, 2),
            nn.BatchNorm2d(256),
            nn.ReLU(inplace=True),
            nn.Upsample(scale_factor=2, mode="nearest"),
            # (256, 32, 96)
            nn.Conv2d(256, 128, 5, 1, 2),
            nn.BatchNorm2d(128),
            nn.ReLU(inplace=True),
            nn.Upsample(scale_factor=2, mode="nearest"),
            # (128, 64, 192)
            nn.Conv2d(128, 64, 5, 1, 2),
            nn.BatchNorm2d(64),
            nn.ReLU(inplace=True),
            nn.Upsample(scale_factor=2, mode="nearest"),
            # (64, 128, 384)
            nn.Conv2d(64, 32, 5, 1, 2),
            nn.BatchNorm2d(32),
            nn.ReLU(inplace=True),
            nn.Upsample(scale_factor=2, mode="nearest"),
            # (32, 256, 768)
            nn.Conv2d(32, 1, 5, 1, 2),
            # (1, 256, 768)
        )

    def forward(self, z):
        return self.features(self.fc(z).view(-1, 512, 8, 24)).tanh_()


def generate(model):
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
