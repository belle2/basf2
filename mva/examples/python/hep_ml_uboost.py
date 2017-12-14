#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

import numpy as np
import basf2_mva
import basf2_mva_util
import subprocess
import time

if __name__ == "__main__":
    variables = ['p', 'pt', 'pz',
                 'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)',
                 'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)',
                 'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)',
                 'chiProb', 'dr', 'dz',
                 'daughter(0, dr)', 'daughter(1, dr)',
                 'daughter(0, dz)', 'daughter(1, dz)',
                 'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
                 'daughter(0, kaonID)', 'daughter(0, pionID)',
                 'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)', 'daughterInvariantMass(1, 2)']

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    # Spectators are the variables for which the selection should be uniform
    general_options.m_spectators = basf2_mva.vector('M')
    general_options.m_target_variable = "isSignal"
    general_options.m_identifier = "HepMLUBoost"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_steering_file = 'mva/examples/python/hep_ml_uboost.py'
    # Set the parameters of the uBoostClassifier,
    # defaults are 50, which is reasonable, but I want to have a example runtime < 2 minutes
    import json
    specific_options.m_config = json.dumps({'n_neighbors': 5, 'n_estimators': 5})
    specific_options.m_framework = 'hep_ml'

    training_start = time.time()
    basf2_mva.teacher(general_options, specific_options)
    training_stop = time.time()
    training_time = training_stop - training_start
    method = basf2_mva_util.Method(general_options.m_identifier)
    inference_start = time.time()
    p, t = method.apply_expert(basf2_mva.vector("test.root"), general_options.m_treename)
    inference_stop = time.time()
    inference_time = inference_stop - inference_start
    auc = basf2_mva_util.calculate_roc_auc(p, t)
    print("HepML", training_time, inference_time, auc)

    subprocess.call('basf2_mva_evaluate.py -o latex.pdf -train train.root -data test.root -i HepMLUBoost', shell=True)
