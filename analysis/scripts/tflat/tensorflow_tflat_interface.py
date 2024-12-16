#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import json
import tempfile
import numpy as np

from basf2_mva_python_interface.tensorflow import State

from tflat.tensorflow_tflat_model import get_preprocessor, get_tflat_model, get_merged_model

from keras.losses import binary_crossentropy
import tensorflow as tf


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Specifies the and configures the keras model
    :param number_of_features:
    :param number_of_spectators:
    :param number_of_events:
    :param training_fraction:
    :param parameters: as dictionary encoded json object
    :return: State obj
    """

    # get all parameters, if they are not available, use default values
    if parameters is None:
        parameters = {}
    else:
        if not isinstance(parameters, dict):
            raise TypeError('parameters must be a dictionary')

    batch_size = parameters.get('batch_size', 100)
    seed = parameters.get('seed', None)

    # set random state
    if seed:
        tf.set_random_seed(seed)

    state = State(get_tflat_model(parameters, number_of_features))

    learning_rate = 0.001
    weight_decay = 0.0001
    optimizer = tf.keras.optimizers.AdamW(
        learning_rate=learning_rate, weight_decay=weight_decay
    )

    state.model.compile(
        optimizer=optimizer,
        loss=binary_crossentropy,
        metrics=[
            'accuracy',
            tf.keras.metrics.AUC()])

    state.model.summary()

    # training object is required in partial fit
    state.batch_size = batch_size

    # save parameters
    saved_parameters = parameters.copy()
    saved_parameters['number_of_features'] = number_of_features
    state.parameters = json.dumps(saved_parameters)
    state.seed = seed
    return state


def apply(state, X):
    """
    Apply estimator to passed data.
    """
    # convert array input to tensor to avoid creating a new graph for each input
    # calling the model directly is faster than using the predict method in most of our applications
    # as we do a loop over events.
    r = state.model(tf.convert_to_tensor(np.atleast_2d(X), dtype=tf.float32), training=False).numpy()
    if r.shape[1] == 1:
        r = r[:, 0]
    return np.require(r, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def load(obj):
    """
    Load Tensorflow model into state
    """

    gpus = tf.config.list_physical_devices('GPU')
    if gpus:
        for gpu in gpus:
            tf.config.experimental.set_memory_growth(gpu, True)

    with tempfile.TemporaryDirectory() as path:
        file_names = obj[0]
        for file_index, file_name in enumerate(file_names):
            with open(f'{path}/{file_name}', 'w+b') as file:
                file.write(bytes(obj[1][file_index]))

        model = tf.keras.models.load_model(f'{path}/{file_names[0]}', safe_mode=True)

    state = State(model=model)
    return state


def begin_fit(state, Xtest, Stest, ytest, wtest, nBatches):
    """
    Use test sets for monitoring
    """
    state.Xtest = Xtest
    state.ytest = ytest
    return state


def partial_fit(state, X, S, y, w, epoch, batch):
    """
    Pass received data to keras model and fit it
    """

    # transform labels
    if y.min() != 0:
        y[y == y.min()] = 0
    if state.ytest.min() != 0:
        state.ytest[state.ytest == state.ytest.min()] = 0

    # not binary
    assert len(np.unique(y)) == 2
    assert len(np.unique(state.ytest)) == 2

    # configure and adapt preprocessor
    preprocessor = get_preprocessor(X)

    X = preprocessor(X)
    state.Xtest = preprocessor(state.Xtest)

    # perform fit() with early stopping callback
    callbacks = [tf.keras.callbacks.EarlyStopping(
        monitor='val_loss',
        min_delta=0,
        patience=25,
        verbose=1,
        mode='auto',
        baseline=None,
        # start_from_epoch=200,
        restore_best_weights=True)]

    state.model.fit(X, y, validation_data=(state.Xtest, state.ytest), batch_size=128, epochs=30, callbacks=callbacks)

    # create a keras model that includes the preprocessing step
    state.model = get_merged_model(preprocessor, state.model, number_of_features=X.shape[1])
    return False


def end_fit(state):
    """
    Store trained keras model
    """
    with tempfile.TemporaryDirectory() as path:
        state.model.save(os.path.join(path, "my_model.keras"))
        # model.save creates a single file: my_model.keras
        file_names = ['my_model.keras']

        files = []
        for file_name in file_names:
            with open(os.path.join(path, file_name), 'rb') as file:
                files.append(file.read())

    del state
    return [file_names, files]
