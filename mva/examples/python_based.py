#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

import numpy as np
import basf2_mva


def get_custom_objects():
    return [apply, preprocessor]


def preprocessor(X):
    return X*2


def apply(state, X):
    """
    Apply estimator to passed data.
    If the estimator has a predict_proba it is called, otherwise call just predict.
    """
    X = preprocessor(X)
    if hasattr(state.estimator, 'predict_proba'):
        x = state.estimator.predict_proba(X)[:, 1]
    else:
        x = state.estimator.predict(X)
    return np.require(x, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def partial_fit(state, X, S, y, w, Xtest, Stest, ytest, wtest, epoch):
    """
    Stores received training data.
    SKLearn is usually not able to perform a partial fit.
    """
    state.X.append(preprocessor(X))
    state.y.append(y.flatten())
    state.w.append(w.flatten())
    return True


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
    specific_options.m_mini_batch_size = 10000
    specific_options.m_steering_file = 'mva/examples/python_based.py'

    for i, l in enumerate(["sklearn", "xgboost"]):
        general_options.m_identifier = "Python_{}".format(i)
        specific_options.m_nIterations = 1
        specific_options.m_framework = l
        basf2_mva.teacher(general_options, specific_options)
