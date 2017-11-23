#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Dennis Weyland 2017

# This example shows how to implement a preprocessing step like equal frequency binning

import basf2_mva
import basf2_mva_util
import time

import tensorflow as tf
import tensorflow.contrib.keras as keras

from keras.layers import Input, Dense, Dropout
from keras.layers.normalization import BatchNormalization
from keras.models import Model
from keras.optimizers import adam
from keras.losses import binary_crossentropy
from keras.activations import sigmoid, tanh
from keras.callbacks import Callback

from basf2_mva_python_interface.contrib_keras import State
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

    state.model.compile(optimizer=adam(lr=0.01), loss=binary_crossentropy, metrics=['accuracy'])

    state.model.summary()

    return state


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """
    Returns just the state object
    """

    state.Xtest = Xtest
    state.ytest = ytest

    return state


def partial_fit(state, X, S, y, w, epoch):
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
        def on_epoch_end(self, epoch, logs={}):
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

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_identifier = "deep_keras"
    general_options.m_treename = "tree"
    variables = ['M', 'p', 'pt', 'pz',
                 'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)',
                 'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)',
                 'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)',
                 'chiProb', 'dr', 'dz',
                 'daughter(0, dr)', 'daughter(1, dr)',
                 'daughter(0, dz)', 'daughter(1, dz)',
                 'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
                 'daughter(0, Kid)', 'daughter(0, piid)',
                 'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)', 'daughterInvariantMass(1, 2)']
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "contrib_keras"
    specific_options.m_steering_file = 'mva/examples/keras/preprocessing.py'
    specific_options.m_normalize = True
    specific_options.m_training_fraction = 0.9

    training_start = time.time()
    basf2_mva.teacher(general_options, specific_options)
    training_stop = time.time()
    training_time = training_stop - training_start
    method = basf2_mva_util.Method(general_options.m_identifier)
    inference_start = time.time()
    test_data = ["test.root"] * 10
    p, t = method.apply_expert(basf2_mva.vector(*test_data), general_options.m_treename)
    inference_stop = time.time()
    inference_time = inference_stop - inference_start
    auc = basf2_mva_util.calculate_roc_auc(p, t)
    print("Tensorflow", training_time, inference_time, auc)
