#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import base64
import basf2_mva
import basf2
from subprocess import PIPE, run
import b2test_utils

variables = ['p', 'pz', 'daughter(0, p)', 'daughter(0, pz)', 'daughter(1, p)', 'daughter(1, pz)',
             'chiProb', 'dr', 'dz', 'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, chiProb)', 'daughter(1, chiProb)',
             'daughter(0, kaonID)', 'daughter(0, pionID)', 'daughterAngle(0, 1)']

# base64-encoded ONNX model in mva/methods/tests/ONNX.xml
# created from mva/methods/tests/test_write_onnx.py
onnx_model_b64 = (
    b'CAgSB3B5dG9yY2gaBTIuMi4yOv8BCloKBWlucHV0CgZ3ZWlnaHQKBGJpYXMSBm9'
    b'1dHB1dBoFL0dlbW0iBEdlbW0qDwoFYWxwaGEVAACAP6ABASoOCgRiZXRhFQAAgD'
    b'+gAQEqDQoGdHJhbnNCGAGgAQISCm1haW5fZ3JhcGgqUAgBCBAQAUIGd2VpZ2h0S'
    b'kC4r0M+4XpUPo4GcL16Nms+93VgvTtwTj3ZPfm9vVIWPoqwYT42zTu+5INePlux'
    b'Pz3/IT0+utoKPbTI9j28lhC9KhAIARABQgRiaWFzSgQmU0U+WhcKBWlucHV0Eg4'
    b'KDAgBEggKAggBCgIIEGIYCgZvdXRwdXQSDgoMCAESCAoCCAEKAggBQgIQEQ=='
)

if __name__ == "__main__":

    # Skip test if files are not available
    try:
        train_file = basf2.find_file('mva/train_D0toKpipi.root', 'examples', False)
        test_file = basf2.find_file('mva/test_D0toKpipi.root', 'examples', False)
    except BaseException:
        b2test_utils.skip_test('Necessary files "train.root" and "test.root" not available.')

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector(train_file)
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"
    general_options.m_max_events = 200

    methods = [
        ('Trivial.xml', basf2_mva.TrivialOptions(), None),
        ('FastBDT.xml', basf2_mva.FastBDTOptions(), None),
        ('TMVAClassification.xml', basf2_mva.TMVAOptionsClassification(), None),
        ('FANN.xml', basf2_mva.FANNOptions(), None),
        ('Python_sklearn.xml', basf2_mva.PythonOptions(), 'sklearn'),
        ('Python_xgb.xml', basf2_mva.PythonOptions(), 'xgboost'),
        ('Python_tensorflow.xml', basf2_mva.PythonOptions(), 'tensorflow'),
        ('Python_torch.xml', basf2_mva.PythonOptions(), 'torch'),
        ('ONNX.xml', basf2_mva.ONNXOptions(), None),
    ]

    # we create payloads so let's switch to an empty, temporary directory
    with b2test_utils.clean_working_directory():
        for identifier, specific_options, framework in methods:
            general_options.m_identifier = identifier
            if framework is not None:
                specific_options.m_framework = framework
            if isinstance(specific_options, type(basf2_mva.ONNXOptions())):
                # ONNX doesn't do training, so we need to provide a model
                specific_options.m_modelFilename = "model.onnx"
                with open("model.onnx", "wb") as f:
                    f.write(base64.b64decode(onnx_model_b64))
            basf2_mva.teacher(general_options, specific_options)

        basf2_mva.expert(basf2_mva.vector(*[i for i, _, _ in methods]),
                         basf2_mva.vector(train_file), 'tree', 'expert.root')

        # don't compile the evaluation as this fails on the build bot due to missing latex files
        command = f'basf2_mva_evaluate.py -o latex.pdf -train {train_file}'\
            f' -data {test_file} -i {" ".join([i for i, _, _ in methods])}'

        result = run(command,
                     stdout=PIPE, stderr=PIPE,
                     text=True, shell=True)
        assert result.returncode == 0, 'basf2_mva_evaluate.py failed!'
