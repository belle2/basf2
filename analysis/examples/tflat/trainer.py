#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import ROOT
import argparse

os.environ["KERAS_BACKEND"] = "torch"


if __name__ == "__main__":

    import torch
    import keras
    from fitter import fit
    from tflat.config import config
    from tflat.model import get_tflat_model
    from tflat.utils import get_variables
    from basf2_mva_util import create_onnx_mva_weightfile

    # parse cli arguments
    parser = argparse.ArgumentParser(description='Train TFlat')
    parser.add_argument(  # input parser
        '--train_input',
        metavar='train_input',
        dest='train_input',
        type=str,
        default="dummyin_train.parquet",
        help='Path to training parquet file'
    )
    parser.add_argument(  # input parser
        '--val_input',
        metavar='val_input',
        dest='val_input',
        type=str,
        default="dummyin_val.parquet",
        help='Path to validation parquet file'
    )
    parser.add_argument(  # checkpoint parser
        '--checkpoint',
        metavar='checkpoint',
        dest='checkpoint',
        type=str,
        nargs='+',
        default="./ckpt/checkpoint.model.keras",
        help='Path to checkpoints'
    )
    parser.add_argument(
        '--warmstart',
        help='Start from checkpoint',
        action=argparse.BooleanOptionalAction
    )
    args = parser.parse_args()

    train_file = args.train_input
    val_file = args.val_input
    checkpoint_filepath = args.checkpoint
    warmstart = args.warmstart

    parameters = config['parameters']
    rank_variable = 'p'
    trk_variable_list = config['trk_variable_list']
    ecl_variable_list = config['ecl_variable_list']
    roe_variable_list = config['roe_variable_list']
    variables = get_variables('pi+:tflat', rank_variable, trk_variable_list, particleNumber=parameters['num_trk'])
    variables += get_variables('gamma:tflat', rank_variable, ecl_variable_list, particleNumber=parameters['num_ecl'])
    variables += get_variables('pi+:tflat', rank_variable, roe_variable_list, particleNumber=parameters['num_roe'])

    if not warmstart:
        if os.path.isfile(checkpoint_filepath):
            os.remove(checkpoint_filepath)

        model = get_tflat_model(parameters=parameters, number_of_features=len(variables))

        # configure the optimizer
        cosine_decay_scheduler = keras.optimizers.schedules.CosineDecay(
            initial_learning_rate=config['initial_learning_rate'],
            decay_steps=config['decay_steps'],
            alpha=config['alpha']
        )

        optimizer = keras.optimizers.AdamW(
            learning_rate=cosine_decay_scheduler, weight_decay=config['weight_decay']
        )

        # compile the model
        model.compile(
            optimizer=optimizer,
            loss=keras.losses.binary_crossentropy,
            metrics=[
                'accuracy',
                keras.metrics.AUC()])
    else:
        model = keras.models.load_model(checkpoint_filepath)

    model.summary()

    fit(
        model,
        train_file,
        val_file,
        "tflat_variables",
        variables,
        "qrCombined",
        config,
        checkpoint_filepath
    )

    torch.onnx.export(
        model,
        (torch.randn(1, len(variables)),),
        "model.onnx",
        input_names=["input"],
        output_names=["output"],
    )

    weightfile = create_onnx_mva_weightfile(
        "model.onnx",
        variables=variables,
        target_variable="qrCombined",
    )

    ROOT.Belle2.MVA.Weightfile.saveToDatabase(weightfile, "standard_tflat")
