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

os.environ["KERAS_BACKEND"] = "torch"


if __name__ == "__main__":

    import torch

    from tflat.tensorflow_tflat_model import get_tflat_model
    from tflat.fitter import fit
    from tflat.config import config
    from tflat.utils import get_variables
    from basf2_mva_util import create_onnx_mva_weightfile

    train_file = "/home/benjamin/dft/tflat_tools/workdir/standard_tflat_training_data24.root"

    parameters = config['parameters']
    rank_variable = 'p'
    trk_variable_list = config['trk_variable_list']
    ecl_variable_list = config['ecl_variable_list']
    roe_variable_list = config['roe_variable_list']
    variables = get_variables('pi+:tflat', rank_variable, trk_variable_list, particleNumber=parameters['num_trk'])
    variables += get_variables('gamma:tflat', rank_variable, ecl_variable_list, particleNumber=parameters['num_ecl'])
    variables += get_variables('pi+:tflat', rank_variable, roe_variable_list, particleNumber=parameters['num_roe'])

    model = get_tflat_model(parameters=parameters, number_of_features=len(variables))
    fit(
        model,
        train_file,
        "tflat_variables",
        variables,
        "qrCombined",
        config
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
