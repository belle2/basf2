#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

import numpy as np
import tensorflow as tf
import basf2_mva


if __name__ == "__main__":
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    variables = ['p', 'pz', 'daughter(0, p)', 'daughter(0, pz)', 'daughter(1, p)', 'daughter(1, pz)',
                 'chiProb', 'dr', 'dz', 'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, chiProb)', 'daughter(1, chiProb)',
                 'daughter(0, Kid)', 'daughter(0, piid)', 'daughterAngle(0, 1)']
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_spectators = basf2_mva.vector('M')
    general_options.m_target_variable = "isSignal"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_mini_batch_size = 100

    for i, l in enumerate(["tensorflow", "sklearn", "xgboost"]):
        general_options.m_identifier = "Python_{}".format(i)
        if i == 0:
            specific_options.m_nIterations = 100
        else:
            specific_options.m_nIterations = 1
        specific_options.m_framework = l
        basf2_mva.teacher(general_options, specific_options)
