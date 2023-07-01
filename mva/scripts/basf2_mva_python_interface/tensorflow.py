#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import numpy as np
import sys
import os
import tempfile


class State:
    """
    Tensorflow state
    """

    def __init__(self, model=None, **kwargs):
        """ Constructor of the state object """

        #: tensorflow model inheriting from tf.Module
        self.model = model


def feature_importance(state):
    """
    Return a list containing the feature importances
    """
    return []


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Return default tensorflow model
    """
    try:
        import tensorflow as tf
    except ImportError:
        print("Please install tensorflow: pip3 install tensorflow")
        sys.exit(1)

    gpus = tf.config.list_physical_devices('GPU')
    if gpus:
        for gpu in gpus:
            tf.config.experimental.set_memory_growth(gpu, True)

    class my_model(tf.Module):

        def __init__(self, **kwargs):

            super().__init__(**kwargs)

            self.W = tf.Variable(tf.ones(shape=(number_of_features, 1)), name="W")
            self.b = tf.Variable(tf.ones(shape=(1, 1)), name="b")

            self.optimizer = tf.optimizers.SGD(0.01)

        @tf.function(input_signature=[tf.TensorSpec(shape=[None, number_of_features], dtype=tf.float32)])
        def __call__(self, x):
            return tf.nn.sigmoid(tf.matmul(self.clean_nans(x), self.W) + self.b)

        def clean_nans(self, x):
            return tf.where(tf.math.is_nan(x), tf.zeros_like(x), x)

        def loss(self, predicted_y, target_y, w):
            # cross entropy
            epsilon = 1e-5
            diff_from_truth = tf.where(target_y == 1., predicted_y, 1. - predicted_y)
            return - tf.reduce_sum(w * tf.math.log(diff_from_truth + epsilon)) / tf.reduce_sum(w)

    state = State(model=my_model())
    return state


def load(obj):
    """
    Load Tensorflow estimator into state
    """
    try:
        import tensorflow as tf
    except ImportError:
        print("Please install tensorflow: pip3 install tensorflow")
        sys.exit(1)

    gpus = tf.config.list_physical_devices('GPU')
    if gpus:
        for gpu in gpus:
            tf.config.experimental.set_memory_growth(gpu, True)

    with tempfile.TemporaryDirectory() as path:

        # recreate the expected folder structure
        for subfolder in ['variables', 'assets']:
            os.makedirs(os.path.join(path, subfolder))

        file_names = obj[0]
        for file_index, file_name in enumerate(file_names):
            with open(f'{path}/{file_name}', 'w+b') as file:
                file.write(bytes(obj[1][file_index]))

        model = tf.saved_model.load(path)

    state = State(model=model)
    return state


def apply(state, X):
    """
    Apply estimator to passed data.
    """
    try:
        import tensorflow as tf
    except ImportError:
        print("Please install tensorflow: pip3 install tensorflow")
        sys.exit(1)

    r = state.model(tf.convert_to_tensor(np.atleast_2d(X), dtype=tf.float32)).numpy()
    if r.shape[1] == 1:
        r = r[:, 0]  # cannot use squeeze because we might have output of shape [1,X classes]
    return np.require(r, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def begin_fit(state, Xtest, Stest, ytest, wtest, nBatches):
    """
    Returns just the state object
    """
    state.nBatches = nBatches
    return state


def partial_fit(state, X, S, y, w, epoch, batch):
    """
    Pass batches of received data to tensorflow
    """
    try:
        import tensorflow as tf
    except ImportError:
        print("Please install tensorflow: pip3 install tensorflow")
        sys.exit(1)

    with tf.GradientTape() as tape:
        avg_cost = state.model.loss(state.model(X), y, w)
        grads = tape.gradient(avg_cost, state.model.trainable_variables)

    state.model.optimizer.apply_gradients(zip(grads, state.model.trainable_variables))

    if batch == 0 and epoch == 0:
        state.avg_costs = [avg_cost]
    elif batch != state.nBatches-1:
        state.avg_costs.append(avg_cost)
    else:
        # end of the epoch, print summary results, reset the avg_costs and update the counter
        print(f"Epoch: {epoch:04d} cost= {np.mean(state.avg_costs):.9f}")
        state.avg_costs = [avg_cost]

    if epoch == 100000:
        return False
    return True


def end_fit(state):
    """
    Store tensorflow model in a graph
    """
    try:
        import tensorflow as tf
    except ImportError:
        print("Please install tensorflow: pip3 install tensorflow")
        sys.exit(1)
    with tempfile.TemporaryDirectory() as path:

        tf.saved_model.save(state.model, path)
        # tf.saved_model.save creates:
        # path/saved_model.pb
        # path/variables/variables.index
        # path/variables/variables.data-00000-of-00001
        # path/assets/*  - This contains additional assets stored in the model.

        file_names = ['saved_model.pb',
                      'variables/variables.index',
                      'variables/variables.data-00000-of-00001']

        # we dont know what, if anything, is saved in assets/
        assets_path = os.path.join(path, 'assets/')
        file_names.extend([f'assets/{f.name}' for f in os.scandir(assets_path) if os.path.isfile(os.path.join(assets_path, f))])

        files = []
        for file_name in file_names:
            with open(os.path.join(path, file_name), 'rb') as file:
                files.append(file.read())
    del state
    return [file_names, files]
