#!/usr/bin/env python3
##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Convert ModeSelector PyTorch checkpoints to ONNX format for basf2 inference.

Usage:
    python convert_to_onnx.py --input-dir networks/ --output-dir onnx/
"""

import argparse
import os
import torch
import torch.nn as nn

from modeSelector.train import MultiClassNet


def convert_network_to_onnx(pt_path, onnx_path):
    """Convert a ModeSelector .pt checkpoint to ONNX.

    Input size and number of output classes are derived automatically
    from the saved weights.
    """
    checkpoint = torch.load(pt_path, map_location='cpu', weights_only=True)
    state_dict = checkpoint['model_state_dict']

    # Derive dimensions from the saved weights
    input_size = state_dict['network.0.weight'].shape[1]
    num_labels = state_dict['network.13.bias'].shape[0]

    model = MultiClassNet(input_size, num_labels)
    model.load_state_dict(state_dict)
    model.eval()

    # Wrap with softmax: train.py outputs raw logits, basf2 inference expects probabilities
    model_with_softmax = nn.Sequential(model, nn.Softmax(dim=1))

    dummy_input = torch.randn(1, input_size)
    torch.onnx.export(
        model_with_softmax,
        dummy_input,
        onnx_path,
        input_names=['input'],
        output_names=['output'],
        dynamic_axes={'input': {0: 'batch_size'}, 'output': {0: 'batch_size'}},
        opset_version=14,
    )
    print(f"Exported {pt_path} -> {onnx_path} (input={input_size}, labels={num_labels})")


def main():
    parser = argparse.ArgumentParser(description='Convert ModeSelector networks to ONNX')
    parser.add_argument('--input-dir', required=True,
                        help='Directory containing net_category.pt and net_main.pt')
    parser.add_argument('--output-dir', required=True,
                        help='Directory for output ONNX files')
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)

    for pt_name, onnx_name in [
        ('net_category.pt', 'modeSelector_cat.onnx'),
        ('net_main.pt', 'modeSelector_main.onnx'),
    ]:
        pt_path = os.path.join(args.input_dir, pt_name)
        onnx_path = os.path.join(args.output_dir, onnx_name)
        if os.path.exists(pt_path):
            convert_network_to_onnx(pt_path, onnx_path)
        else:
            print(f"Warning: {pt_path} not found")


if __name__ == '__main__':
    main()
