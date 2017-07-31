import os
import tempfile
import h5py
import numpy as np

import tensorflow as tf
import tensorflow.contrib.keras as keras

from keras.layers import Input, Dense, Concatenate
from keras.models import Model, load_model
from keras.optimizers import adam
from keras.losses import binary_crossentropy


class State(object):
    """
    Tensorflow state
    """

    def __init__(self, model=None, custom_objects=None, **kwargs):
        """ Constructor of the state object """
        #: keras model
        self.model = model
        #: used by keras to load custom objects like custom layers
        self.custom_objects = custom_objects
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
    Return default tensorflow model
    """
    input = Input(shape=(number_of_features,))
    net = Dense(units=1)(input)

    state = State(Model(input, net))

    state.model.compile(optimizer=adam(), loss=binary_crossentropy, metrics=['accuracy'])

    state.model.summary()

    return state


def load(obj):
    """
    Load Tensorflow estimator into state
    """
    with tempfile.TemporaryDirectory() as path:
        with open(os.path.join(path, 'weights.h5'), 'w+b') as file:
            file.write(bytes(obj[0]))
        state = State(load_model(os.path.join(path, 'weights.h5'), custom_objects=obj[1]))

    for index, key in enumerate(obj[2]):
        setattr(state, key, obj[index + 3])

    return state


def apply(state, X):
    """
    Apply estimator to passed data.
    """
    r = state.model.predict(X).flatten()
    return np.require(r, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """
    Returns just the state object
    """
    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Pass received data to tensorflow session
    """
    state.model.fit(X, y, batch_size=100, epochs=10)
    return False


def end_fit(state):
    """
    Store tensorflow session in a graph
    """

    with tempfile.TemporaryDirectory() as path:
        state.model.save(os.path.join(path, 'weights.h5'))
        with open(os.path.join(path, 'weights.h5'), 'rb') as file:
            data = file.read()

    obj_to_save = [data, state.custom_objects, state.collection_keys]
    for key in state.collection_keys:
        obj_to_save.append(getattr(state, key))
    del state
    return obj_to_save
