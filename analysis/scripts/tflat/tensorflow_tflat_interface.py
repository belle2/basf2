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
import pathlib
import numpy as np

from basf2_mva_python_interface.tensorflow import State
from tflat.config import config

from keras.losses import binary_crossentropy
import tensorflow as tf


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Specifies and configures the keras model
    :param number_of_features:
    :param number_of_spectators:
    :param number_of_events:
    :param training_fraction:
    :param parameters: as dictionary encoded json object
    :return: State obj
    """

    from tflat.tensorflow_tflat_model import get_tflat_model

    parameters = config['parameters']

    # set random state
    seed = config['parameters']['seed']
    if seed:
        tf.random.set_seed(seed)

    state = State(get_tflat_model(parameters, number_of_features))

    cosine_decay_scheduler = tf.keras.optimizers.schedules.CosineDecay(
        initial_learning_rate=config['initial_learning_rate'],
        decay_steps=config['decay_steps'],
        alpha=config['alpha']
    )

    optimizer = tf.keras.optimizers.AdamW(
        learning_rate=cosine_decay_scheduler, weight_decay=config['weight_decay']
    )

    state.model.compile(
        optimizer=optimizer,
        loss=binary_crossentropy,
        metrics=[
            'accuracy',
            tf.keras.metrics.AUC()])

    state.model.summary()

    # save parameters
    saved_parameters = parameters.copy()
    saved_parameters['number_of_features'] = number_of_features
    state.parameters = json.dumps(saved_parameters)

    return state


def apply(state, X):
    """
    Apply estimator to passed data.
    """
    # convert array input to tensor to avoid creating a new graph for each input
    # calling the model directly is faster than using the predict method in most of our applications
    # as we do a loop over events.
    r = state.model.serve(tf.convert_to_tensor(np.atleast_2d(X), dtype=tf.float32)).numpy()
    if r.shape[1] == 1:
        r = r[:, 0]
    return np.require(r, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def load(obj):
    """
    Load Tensorflow model into state
    """

    with tempfile.TemporaryDirectory() as temp_path:
        temp_path = pathlib.Path(temp_path)
        file_names = obj[0]
        for file_index, file_name in enumerate(file_names):
            path = temp_path.joinpath(pathlib.Path(file_name))
            path.parents[0].mkdir(parents=True, exist_ok=True)

            with open(path, 'w+b') as file:
                file.write(bytes(obj[1][file_index]))

        model = tf.saved_model.load(pathlib.Path(temp_path) / 'my_model.keras')

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

    # assert binary labels
    assert len(np.unique(y)) == 2
    assert len(np.unique(state.ytest)) == 2

    # perform fit() with early stopping callback
    callbacks = [tf.keras.callbacks.EarlyStopping(
        monitor='val_loss',
        min_delta=0,
        patience=config['patience'],
        verbose=1,
        mode='auto',
        baseline=None,
        restore_best_weights=True)]

    state.model.fit(
        X,
        y,
        validation_data=(
            state.Xtest,
            state.ytest),
        batch_size=config['batch_size'],
        epochs=config['epochs'],
        callbacks=callbacks,
        verbose=2)

    return False


def end_fit(state):
    """
    Store trained keras model
    """
    with tempfile.TemporaryDirectory() as path:
        path = pathlib.Path(path)

        state.model.export(os.path.join(path, "my_model.keras"))

        file_names = [f.relative_to(path) for f in path.rglob('*') if f.is_file()]
        files = []
        for file_name in file_names:
            with open(os.path.join(path, file_name), 'rb') as file:
                files.append(file.read())

    del state
    return [file_names, files]
