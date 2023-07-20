##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import pathlib
import tempfile
import numpy as np

from tensorflow.keras.layers import Dense, Input
from tensorflow.keras.models import Model, load_model
from tensorflow.keras.losses import binary_crossentropy
import tensorflow as tf
from basf2 import B2WARNING


class State:
    """
    Tensorflow.keras state
    """

    def __init__(self, model=None, **kwargs):
        """ Constructor of the state object """
        #: keras model
        self.model = model

        #: list of keys to save
        self.collection_keys = []

        # other possible things to save into a tensorflow collection
        for key, value in kwargs.items():
            self.collection_keys.append(key)
            setattr(self, key, value)


def feature_importance(state):
    """
    Return a list containing the feature importances
    """
    return []


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Return default tensorflow.keras model
    """

    input = Input(shape=(number_of_features,))
    net = Dense(units=1)(input)

    state = State(Model(input, net))

    state.model.compile(optimizer="adam", loss=binary_crossentropy, metrics=['accuracy'])

    state.model.summary()

    return state


def load(obj):
    """
    Load Tensorflow.keras model into state
    """
    with tempfile.TemporaryDirectory() as temp_path:

        temp_path = pathlib.Path(temp_path)

        file_names = obj[0]
        for file_index, file_name in enumerate(file_names):
            path = temp_path.joinpath(pathlib.Path(file_name))
            path.parents[0].mkdir(parents=True, exist_ok=True)

            with open(path, 'w+b') as file:
                file.write(bytes(obj[1][file_index]))

        state = State(load_model(pathlib.Path(temp_path) / 'my_model'))

        for index, key in enumerate(obj[2]):
            setattr(state, key, obj[3][index])
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
        r = r[:, 0]  # cannot use squeeze because we might have output of shape [1,X classes]
    return np.require(r, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def begin_fit(state, Xtest, Stest, ytest, wtest, nBatches):
    """
    Returns just the state object
    """
    return state


def partial_fit(state, X, S, y, w, epoch, batch):
    """
    Pass received data to tensorflow.keras session
    """
    if epoch > 0:
        B2WARNING("The keras training interface has been called with specific_options.m_nIterations > 1."
                  " In the default implementation this should not be done as keras handles the number of epochs internally.")

    if batch > 0:
        B2WARNING("The keras training interface has been called with specific_options.m_mini_batch_size > 1."
                  " In the default implementation this should not be done as keras handles the number of batches internally.")

    state.model.fit(X, y, batch_size=100, epochs=10)
    return False


def end_fit(state):
    """
    Store tensorflow.keras session in a graph
    """

    with tempfile.TemporaryDirectory() as temp_path:

        temp_path = pathlib.Path(temp_path)
        state.model.save(temp_path.joinpath('my_model'))

        # this creates:
        # path/my_model/saved_model.pb
        # path/my_model/keras_metadata.pb (sometimes)
        # path/my_model/variables/*
        # path/my_model/assets/*
        file_names = [f.relative_to(temp_path) for f in temp_path.rglob('*') if f.is_file()]
        files = []
        for file_name in file_names:
            with open(temp_path.joinpath(file_name), 'rb') as file:
                files.append(file.read())

        collection_keys = state.collection_keys
        collections_to_store = []
        for key in state.collection_keys:
            collections_to_store.append(getattr(state, key))

    del state
    return [file_names, files, collection_keys, collections_to_store]
