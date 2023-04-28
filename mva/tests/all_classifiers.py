#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2_mva
import basf2
from subprocess import PIPE, run
import b2test_utils

variables = ['p', 'pz', 'daughter(0, p)', 'daughter(0, pz)', 'daughter(1, p)', 'daughter(1, pz)',
             'chiProb', 'dr', 'dz', 'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, chiProb)', 'daughter(1, chiProb)',
             'daughter(0, kaonID)', 'daughter(0, pionID)', 'daughterAngle(0, 1)']

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
    ]

    # we create payloads so let's switch to an empty, temporary directory
    with b2test_utils.clean_working_directory():
        for identifier, specific_options, framework in methods:
            general_options.m_identifier = identifier
            if framework is not None:
                specific_options.m_framework = framework
            basf2_mva.teacher(general_options, specific_options)

        basf2_mva.expert(basf2_mva.vector(*[i for i, _, _ in methods]),
                         basf2_mva.vector(train_file), 'tree', 'expert.root')

        command = f'basf2_mva_evaluate.py -c -o latex.pdf -train {train_file}'\
            ' -data {test_file} -i {" ".join([i for i, _, _ in methods])}'

        result = run(command,
                     stdout=PIPE, stderr=PIPE,
                     text=True, shell=True)
        assert result.returncode == 0, 'basf2_mva_evaluate.py failed!'
