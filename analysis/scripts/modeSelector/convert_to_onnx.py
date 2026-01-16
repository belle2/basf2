#!/usr/bin/env python3
##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Script to convert ModeSelector PyTorch networks to ONNX format and create
MVA weightfiles for use in basf2.

Usage:
    python convert_to_onnx.py --input-dir /path/to/networks --output-dir /path/to/output
"""

import argparse
import os
import torch
import torch.nn as nn


class MultiLabelNet(nn.Module):
    """
    Neural network for ModeSelector.
    Architecture: 256 -> 128 -> 64 -> 32 -> 16 -> num_labels
    """

    def __init__(self, input_size, num_labels):
        super().__init__()
        self.activation = nn.ReLU()

        self.dense_layers = nn.Sequential(
            nn.Linear(input_size, 256),
            self.activation,
            nn.Linear(256, 128),
            nn.Dropout(0.2),
            self.activation,
            nn.Linear(128, 64),
            nn.Dropout(0.15),
            self.activation,
            nn.Linear(64, 32),
            nn.Dropout(0.1),
            self.activation,
            nn.Linear(32, 16),
            self.activation,
            nn.Linear(16, num_labels)
        )

    def forward(self, x):
        return self.dense_layers(x)


class MultiLabelNetWithSoftmax(nn.Module):
    """
    Wrapper that applies softmax to the output for inference.
    """

    def __init__(self, base_model):
        super().__init__()
        self.base_model = base_model
        self.softmax = nn.Softmax(dim=1)

    def forward(self, x):
        logits = self.base_model(x)
        return self.softmax(logits)


def convert_network_to_onnx(
    pt_path: str,
    onnx_path: str,
    input_size: int,
    num_labels: int,
    apply_softmax: bool = True
):
    """
    Convert a PyTorch network to ONNX format.

    Parameters
    ----------
    pt_path : str
        Path to the PyTorch state dict (.pt file)
    onnx_path : str
        Path for the output ONNX file
    input_size : int
        Number of input features
    num_labels : int
        Number of output classes
    apply_softmax : bool
        Whether to apply softmax to the output
    """
    # Create model and load weights
    model = MultiLabelNet(input_size, num_labels)
    model.load_state_dict(torch.load(pt_path, map_location='cpu', weights_only=True))
    model.eval()

    # Wrap with softmax for inference
    if apply_softmax:
        model = MultiLabelNetWithSoftmax(model)

    # Create dummy input
    dummy_input = torch.randn(1, input_size)

    # Export to ONNX
    torch.onnx.export(
        model,
        dummy_input,
        onnx_path,
        input_names=['input'],
        output_names=['output'],
        dynamic_axes={
            'input': {0: 'batch_size'},
            'output': {0: 'batch_size'}
        },
        opset_version=14
    )
    print(f"Exported {pt_path} -> {onnx_path}")


def create_mva_weightfile(
    onnx_path: str,
    output_path: str,
    variables: list,
    target_variable: str = "isSignal"
):
    """
    Create an MVA weightfile from an ONNX model.

    Parameters
    ----------
    onnx_path : str
        Path to the ONNX model
    output_path : str
        Path for the output weightfile (.xml or .root)
    variables : list
        List of input variable names
    target_variable : str
        Name of the target variable
    """
    from basf2_mva_util import create_onnx_mva_weightfile

    weightfile = create_onnx_mva_weightfile(
        onnx_path,
        variables=variables,
        target_variable=target_variable,
    )
    weightfile.save(output_path)
    print(f"Created weightfile: {output_path}")


def main():
    parser = argparse.ArgumentParser(description='Convert ModeSelector networks to ONNX')
    parser.add_argument('--input-dir', type=str, required=True,
                        help='Directory containing net.pt and net_cat.pt')
    parser.add_argument('--output-dir', type=str, required=True,
                        help='Directory for output ONNX files')
    parser.add_argument('--cat-input-size', type=int, default=1004,
                        help='Input size for category network')
    parser.add_argument('--main-input-size', type=int, default=1008,
                        help='Input size for main network (cat_input_size + 4)')
    parser.add_argument('--num-cat-labels', type=int, default=3,
                        help='Number of category labels (B0, B+, continuum)')
    parser.add_argument('--num-main-labels', type=int, default=6,
                        help='Number of main network labels')
    parser.add_argument('--create-weightfiles', action='store_true',
                        help='Also create MVA weightfiles')
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)

    # Convert category network
    cat_pt_path = os.path.join(args.input_dir, 'net_cat.pt')
    cat_onnx_path = os.path.join(args.output_dir, 'modeSelector_cat.onnx')

    if os.path.exists(cat_pt_path):
        convert_network_to_onnx(
            cat_pt_path,
            cat_onnx_path,
            input_size=args.cat_input_size,
            num_labels=args.num_cat_labels
        )
    else:
        print(f"Warning: {cat_pt_path} not found")

    # Convert main network
    main_pt_path = os.path.join(args.input_dir, 'net.pt')
    main_onnx_path = os.path.join(args.output_dir, 'modeSelector_main.onnx')

    if os.path.exists(main_pt_path):
        convert_network_to_onnx(
            main_pt_path,
            main_onnx_path,
            input_size=args.main_input_size,
            num_labels=args.num_main_labels
        )
    else:
        print(f"Warning: {main_pt_path} not found")

    print("\nConversion complete!")
    print(f"Category network: {cat_onnx_path}")
    print(f"Main network: {main_onnx_path}")

    if args.create_weightfiles:
        print("\nNote: To create MVA weightfiles, you need to define the input variables.")
        print("This requires the feature list from your training configuration.")


if __name__ == '__main__':
    main()
