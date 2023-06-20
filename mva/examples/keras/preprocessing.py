#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This example shows how to implement a preprocessing step like equal frequency binning

import basf2_mva
import basf2_mva_util
import time

import numpy as np


from tensorflow.keras.layers import Input, Dense, Dropout, BatchNormalization
from tensorflow.keras.models import Model
from tensorflow.keras.optimizers import Adam
from tensorflow.keras.losses import binary_crossentropy
from tensorflow.keras.activations import sigmoid, tanh
from tensorflow.keras.callbacks import Callback

from basf2_mva_python_interface.keras import State
from basf2_mva_extensions.preprocessing import fast_equal_frequency_binning

old_time = time.time()


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Build feed forward keras model
    """
    input = Input(shape=(number_of_features,))

    net = Dense(units=number_of_features, activation=tanh)(input)
    for i in range(7):
        net = Dense(units=number_of_features, activation=tanh)(net)
        net = BatchNormalization()(net)
    for i in range(7):
        net = Dense(units=number_of_features, activation=tanh)(net)
        net = Dropout(rate=0.4)(net)

    output = Dense(units=1, activation=sigmoid)(net)

    # Pass empty preprocessor state as kwarg in the state class.
    # The interface is designed to automatically save every kwarg, which is passed in the initializer in end_fit.
    state = State(Model(input, output), preprocessor_state=None)

    state.model.compile(optimizer=Adam(lr=0.01), loss=binary_crossentropy, metrics=['accuracy'])

    state.model.summary()

    return state


def begin_fit(state, Xtest, Stest, ytest, wtest, nBatches):
    """
    Returns just the state object
    """

    state.Xtest = Xtest
    state.ytest = ytest

    return state


def partial_fit(state, X, S, y, w, epoch, batch):
    """
    Pass received data to tensorflow session
    """
    # Fit and Apply preprocessor
    preprocessor = fast_equal_frequency_binning()
    preprocessor.fit(X)
    X = preprocessor.apply(X)
    state.Xtest = preprocessor.apply(state.Xtest)

    # save preprocessor state in the State class
    state.preprocessor_state = preprocessor.export_state()

    class TestCallback(Callback):
        def on_epoch_end(self, epoch, logs=None):
            loss, acc = state.model.evaluate(state.Xtest, state.ytest, verbose=0, batch_size=1000)
            loss2, acc2 = state.model.evaluate(X[:10000], y[:10000], verbose=0, batch_size=1000)
            print('\nTesting loss: {}, acc: {}'.format(loss, acc))
            print('Training loss: {}, acc: {}'.format(loss2, acc2))

    state.model.fit(X, y, batch_size=500, epochs=10, callbacks=[TestCallback()])
    return False


def apply(state, X):
    """
    Apply estimator to passed data.
    Has to be overwritten, because also the expert has to apply preprocessing.
    """
    # The preprocessor state is automatically loaded in the load function
    preprocessor = fast_equal_frequency_binning(state.preprocessor_state)
    # Apply preprocessor
    X = preprocessor.apply(X)

    r = state.model.predict(X).flatten()
    return np.require(r, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


if __name__ == "__main__":
    from basf2 import conditions, find_file
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    train_file = find_file("mva/train_D0toKpipi.root", "examples")
    test_file = find_file("mva/test_D0toKpipi.root", "examples")

    training_data = basf2_mva.vector(train_file)
    testing_data = basf2_mva.vector(test_file)

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    general_options.m_identifier = "preprocessed_deep_keras"
    general_options.m_treename = "tree"
    variables = ['M', 'p', 'pt', 'pz',
                 'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)',
                 'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)',
                 'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)',
                 'chiProb', 'dr', 'dz',
                 'daughter(0, dr)', 'daughter(1, dr)',
                 'daughter(0, dz)', 'daughter(1, dz)',
                 'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
                 'daughter(0, kaonID)', 'daughter(0, pionID)',
                 'daughterInvM(0, 1)', 'daughterInvM(0, 2)', 'daughterInvM(1, 2)']
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "keras"
    specific_options.m_steering_file = 'mva/examples/keras/preprocessing.py'
    specific_options.m_normalize = True
    specific_options.m_training_fraction = 0.9

    training_start = time.time()
    basf2_mva.teacher(general_options, specific_options)
    training_stop = time.time()
    training_time = training_stop - training_start
    method = basf2_mva_util.Method(general_options.m_identifier)
    inference_start = time.time()
    p, t = method.apply_expert(testing_data, general_options.m_treename)
    inference_stop = time.time()
    inference_time = inference_stop - inference_start
    auc = basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)

    print("Tensorflow", training_time, inference_time, auc)
