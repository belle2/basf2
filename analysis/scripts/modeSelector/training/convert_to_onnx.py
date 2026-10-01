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

import numpy as np
import torch
import torch.nn as nn
from modeSelector import config
from modeSelector.training.train import MultiClassNet
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
    model_with_softmax.eval()

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
    _validate_onnx_export(model_with_softmax, onnx_path, input_size)
    return input_size, num_labels, checkpoint.get('has_inputs')


def _validate_onnx_export(model_with_softmax, onnx_path, input_size):
    """Check that the exported ONNX model produces the same output as the PyTorch model.

    Uses a batch of 4 inputs from a fixed seed to exercise the dynamic batch
    dimension reproducibly. Tolerances are set for float32: torch and onnxruntime
    accumulate the matmuls in a different order, which shifts individual softmax
    probabilities of the 139-class main network by a few 1e-6. The numpy defaults
    (atol=1e-8) are meant for float64 and reject those runs at random.
    The predicted class is compared separately, since that is what the module uses.
    Raises RuntimeError if outputs do not match.
    """
    import onnxruntime as ort

    generator = torch.Generator().manual_seed(0)
    dummy = torch.randn(4, input_size, generator=generator)
    with torch.no_grad():
        pt_out = model_with_softmax(dummy).numpy()
    session = ort.InferenceSession(onnx_path)
    onnx_out = session.run(['output'], {'input': dummy.numpy()})[0]
    if not np.allclose(pt_out, onnx_out, rtol=1e-4, atol=1e-6):
        max_diff = float(np.abs(pt_out - onnx_out).max())
        raise RuntimeError(
            'ONNX validation failed for ' + onnx_path
            + ': outputs do not match PyTorch model (max abs diff '
            + f'{max_diff:.3e})'
        )
    if not (pt_out.argmax(axis=1) == onnx_out.argmax(axis=1)).all():
        raise RuntimeError(
            'ONNX validation failed for ' + onnx_path + ': predicted class does not match PyTorch model'
        )
    print(f"ONNX validation passed for {onnx_path}")


def build_variable_names(has_inputs, input_size, is_main):
    """Build the weightfile variable list describing this model's inputs.

    The names are not resolved through the VariableManager. ModeSelectorModule fills
    the feature vector manually. They are used to record which raw feature indices the
    model was trained on, so the payload carries its own feature selection and a
    retraining can change it without a software release.
    """
    if not has_inputs:
        raise RuntimeError(
            'Checkpoint does not store has_inputs, cannot record the feature selection '
            'in the weightfile. Retrain or export with a checkpoint written by train.py.'
        )
    names = [config.FEATURE_VAR_PREFIX + str(i) for i in has_inputs]
    if is_main:
        names += list(config.MAIN_EXTRA_VARS)
    if len(names) != input_size:
        raise RuntimeError(
            'Feature selection does not match the model: built ' + str(len(names))
            + ' variable names but the network takes ' + str(input_size) + ' inputs.'
        )
    return names


def package_as_mva_weightfile(onnx_path, root_path, variables, n_classes, identifier):
    """Wrap an ONNX file in a basf2 MVA weightfile, saved as .root file.

    The variable list records the raw feature indices the model was trained on. The contract
    version is added as an extra weightfile element, and the identifier holds the training
    name. All three are read back at inference time.
    """
    from basf2_mva_util import create_onnx_mva_weightfile
    wf = create_onnx_mva_weightfile(
        onnx_path,
        variables=variables,
        nClasses=n_classes,
        identifier=identifier,
    )
    wf.addContractVersion(config.MODEL_CONTRACT_VERSION)
    wf.save(root_path)
    print(f"Packaged {onnx_path} -> {root_path} ({len(variables)} features, {n_classes} classes)")
    print(f"  identifier: {identifier}")


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
    parser.add_argument('--cat-payload-name', default=None,
                        help='Payload name for the category model (default: derived from the contract version)')
    parser.add_argument('--main-payload-name', default=None,
                        help='Payload name for the main model (default: derived from the contract version)')
    parser.add_argument('--identifier', default='unspecified',
                        help='Name of this training, stored as the weightfile identifier and '
                             'logged at inference time (for example sample and iteration)')
    parser.add_argument('--first-exp', type=int, default=0,
                        help='First experiment of the interval of validity')
    parser.add_argument('--first-run', type=int, default=0,
                        help='First run of the interval of validity')
    parser.add_argument('--final-exp', type=int, default=-1,
                        help='Final experiment of the interval of validity')
    parser.add_argument('--final-run', type=int, default=-1,
                        help='Final run of the interval of validity')
    args = parser.parse_args()

    print(f"Contract version {config.MODEL_CONTRACT_VERSION}")

    model_specs = [
        ('net_category.pt', 'modeSelector_cat.onnx', 'modeSelector_cat.root', False),
        ('net_main.pt', 'modeSelector_main.onnx', 'modeSelector_main.root', True),
    ]

    # A payload records the contract version of the code that exported it, so refuse to
    # export from code whose contract configuration is inconsistent.
    error = config.contract_consistency_error()
    if error:
        raise RuntimeError('Cannot export: ' + error)

    # The default names encode the contract version of this code, and releases request
    # exactly that name. Exporting under another name bypasses that link.
    if args.cat_payload_name is None:
        args.cat_payload_name = config.DEFAULT_CAT_PAYLOAD
    if args.main_payload_name is None:
        args.main_payload_name = config.DEFAULT_MAIN_PAYLOAD
    if args.add_payloads:
        for given, default in ((args.cat_payload_name, config.DEFAULT_CAT_PAYLOAD),
                               (args.main_payload_name, config.DEFAULT_MAIN_PAYLOAD)):
            if given != default:
                print('Warning: exporting payload ' + given + ' instead of ' + default + '. Releases '
                      'implementing contract version ' + str(config.MODEL_CONTRACT_VERSION) + ' request '
                      + default + ', so they will not load this payload.')

    # Belle2.Database.addPayload() appends to an existing local database rather than
    # replacing it, which would leave two iovs per payload name covering the same range.
    # Checked before the conversion so the run fails immediately rather than at the end.
    localdb = os.path.join('localdb', 'database.txt')
    if args.add_payloads and os.path.exists(localdb):
        raise RuntimeError(
            os.path.abspath(localdb) + ' already exists. Adding payloads would append to it '
            'instead of replacing it, leaving stale entries with overlapping iovs. Move or '
            'delete the localdb directory and run again.'
        )

    missing = [pt_name for pt_name, _onnx, _root, _is_main in model_specs
               if not os.path.exists(os.path.join(args.input_dir, pt_name))]
    if len(missing) == len(model_specs):
        raise RuntimeError(
            'No checkpoints found in --input-dir ' + args.input_dir + ' (looked for '
            + ', '.join(missing) + '). Point --input-dir at the directory holding the '
            'trained .pt files.'
        )

    os.makedirs(args.output_dir, exist_ok=True)

    exported_root = {}
    selections = {}
    for pt_name, onnx_name, root_name, is_main in model_specs:
        pt_path = os.path.join(args.input_dir, pt_name)
        onnx_path = os.path.join(args.output_dir, onnx_name)
        root_path = os.path.join(args.output_dir, root_name)
        if os.path.exists(pt_path):
            input_size, num_labels, has_inputs = convert_network_to_onnx(pt_path, onnx_path)
            variables = build_variable_names(has_inputs, input_size, is_main)
            package_as_mva_weightfile(onnx_path, root_path, variables, num_labels, args.identifier)
            exported_root[root_name] = root_path
            selections[pt_name] = list(has_inputs)
        else:
            print(f"Warning: {pt_path} not found")

    # Both networks read the same raw features, so a disagreement means the two
    # checkpoints come from different trainings and must not be paired in one payload set.
    if len(selections) == 2 and selections['net_category.pt'] != selections['net_main.pt']:
        raise RuntimeError(
            'net_category.pt and net_main.pt were trained on different feature selections. '
            'Re-export from a matching pair of checkpoints.'
        )

    if selections and sorted(selections.values())[0] != list(config.HAS_INPUTS):
        print('Note: the exported feature selection differs from config.HAS_INPUTS. The payload '
              'carries its own selection, so this is only a problem for training.')

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
