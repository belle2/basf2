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
    python3 convert_to_onnx.py --input-dir networks/ --output-dir onnx/
"""

import argparse
import os

import torch
import torch.nn as nn
from modeSelector import config
from modeSelector.train import MultiClassNet
from ROOT import Belle2


def convert_network_to_onnx(pt_path, onnx_path):
    """Convert a ModeSelector .pt checkpoint to ONNX.

    Input size and number of output classes are derived automatically
    from the saved weights. The category network has 3 output classes;
    the main network has N_INPUT_IDS + 3 = 139 output classes.
    """
    checkpoint = torch.load(pt_path, map_location='cpu', weights_only=True)
    state_dict = checkpoint['model_state_dict']

    # Derive dimensions from the saved weights.
    # The last linear layer index depends on architecture depth, so find it dynamically.
    input_size = state_dict['network.0.weight'].shape[1]
    last_layer_idx = max(int(k.split('.')[1]) for k in state_dict if k.endswith('.bias'))
    num_labels = state_dict[f'network.{last_layer_idx}.bias'].shape[0]

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


def package_as_mva_weightfile(onnx_path, root_path, n_features, n_classes):
    """Wrap an ONNX file in a basf2 MVA weightfile, aved as .root file.

    The variable names are placeholders; ModeSelectorModule fills the feature
    vector manually rather than via the VariableManager.
    """
    from basf2_mva_util import create_onnx_mva_weightfile
    wf = create_onnx_mva_weightfile(
        onnx_path,
        variables=[f"f{i}" for i in range(n_features)],
        nClasses=n_classes,
    )
    wf.save(root_path)
    print(f"Packaged {onnx_path} -> {root_path} ({n_features} features, {n_classes} classes)")


def add_onnx_payloads(cat_model, main_model, cat_payload_name, main_payload_name, iov):
    """Copy the ModeSelector MVA weightfiles into localdb/database.txt."""
    database = Belle2.Database.Instance()

    if not database.addPayload(cat_payload_name, cat_model, iov):
        raise RuntimeError(
            'Failed to add category payload '
            + cat_payload_name
            + ' from '
            + cat_model
        )
    if not database.addPayload(main_payload_name, main_model, iov):
        raise RuntimeError(
            'Failed to add main payload '
            + main_payload_name
            + ' from '
            + main_model
        )

    print('Created localdb/database.txt with ModeSelector payloads')


def main():
    """Convert net_category.pt and net_main.pt to ONNX, package as MVA weightfiles, and optionally upload to localdb."""
    parser = argparse.ArgumentParser(description='Convert ModeSelector networks to ONNX')
    parser.add_argument('--input-dir', required=True,
                        help='Directory containing net_category.pt and net_main.pt')
    parser.add_argument('--output-dir', required=True,
                        help='Directory for output ONNX files')
    parser.add_argument('--add-payloads', action='store_true',
                        help='Copy the exported ONNX files into localdb/database.txt')
    parser.add_argument('--cat-payload-name', default='modeSelector_cat_model_v2',
                        help='Payload name for the category model')
    parser.add_argument('--main-payload-name', default='modeSelector_main_model_v2',
                        help='Payload name for the main model')
    parser.add_argument('--first-exp', type=int, default=0,
                        help='First experiment of the interval of validity')
    parser.add_argument('--first-run', type=int, default=0,
                        help='First run of the interval of validity')
    parser.add_argument('--final-exp', type=int, default=-1,
                        help='Final experiment of the interval of validity')
    parser.add_argument('--final-run', type=int, default=-1,
                        help='Final run of the interval of validity')
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)

    cat_n_features = len(config.HAS_INPUTS)
    cat_n_classes = 3
    main_n_features = len(config.HAS_INPUTS) + 4
    main_n_classes = config.N_INPUT_IDS + 3

    model_specs = [
        ('net_category.pt', 'modeSelector_cat.onnx', 'modeSelector_cat.root',
         cat_n_features, cat_n_classes),
        ('net_main.pt', 'modeSelector_main.onnx', 'modeSelector_main.root',
         main_n_features, main_n_classes),
    ]

    exported_root = {}
    for pt_name, onnx_name, root_name, n_features, n_classes in model_specs:
        pt_path = os.path.join(args.input_dir, pt_name)
        onnx_path = os.path.join(args.output_dir, onnx_name)
        root_path = os.path.join(args.output_dir, root_name)
        if os.path.exists(pt_path):
            convert_network_to_onnx(pt_path, onnx_path)
            package_as_mva_weightfile(onnx_path, root_path, n_features, n_classes)
            exported_root[root_name] = root_path
        else:
            print(f"Warning: {pt_path} not found")

    if args.add_payloads:
        missing_files = [
            name for name in ('modeSelector_cat.root', 'modeSelector_main.root')
            if name not in exported_root
        ]
        if missing_files:
            raise RuntimeError(
                'Cannot add payloads because these weightfiles were not created: '
                + ', '.join(missing_files)
            )

        iov = Belle2.IntervalOfValidity(
            args.first_exp,
            args.first_run,
            args.final_exp,
            args.final_run,
        )
        add_onnx_payloads(
            exported_root['modeSelector_cat.root'],
            exported_root['modeSelector_main.root'],
            args.cat_payload_name,
            args.main_payload_name,
            iov,
        )


if __name__ == '__main__':
    main()
