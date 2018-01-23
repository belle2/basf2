#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import numpy as np
import sys
import os
import tempfile


class State(object):
    """
    Tensorflow state
    """

    def __init__(self, x=None, y=None, activation=None, cost=None, optimizer=None, session=None, **kwargs):
        """ Constructor of the state object """
        #: feature matrix placeholder
        self.x = x
        #: target placeholder
        self.y = y
        #: activation function
        self.activation = activation
        #: cost function
        self.cost = cost
        #: optimizer used to minimize cost function
        self.optimizer = optimizer
        #: tensorflow session
        self.session = session
        #: array to save keys for collection
        self.collection_keys = ['x', 'y', 'activation', 'cost', 'optimizer']

        # other possible things to save into a tensorflow collection
        for key, value in kwargs.items():
            self.collection_keys.append(key)
            setattr(self, key, value)

    def add_to_collection(self):
        """ Add the stored members to the current tensorflow collection """
        try:
            import tensorflow as tf
        except ImportError:
            print("Please install tensorflow: pip3 install tensorflow")
            sys.exit(1)

        for key in self.collection_keys:
            tf.add_to_collection(key, getattr(self, key))

        return self.collection_keys

    def get_from_collection(self, collection_keys=None):
        """ Get members from the current tensorflow collection """
        try:
            import tensorflow as tf
        except ImportError:
            print("Please install tensorflow: pip3 install tensorflow")
            sys.exit(1)

        if collection_keys is not None:
            self.collection_keys = collection_keys

        for key in self.collection_keys:
            setattr(self, key, tf.get_collection(key)[0])


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

    x = tf.placeholder("float", [None, number_of_features])
    y = tf.placeholder("float", [None, 1])
    w = tf.placeholder("float", [None, 1])
    W = tf.Variable(tf.zeros([number_of_features, 1]))
    b = tf.Variable(tf.zeros([1]))

    x_clean = tf.select(tf.is_nan(x), tf.ones_like(x) * 0., x)
    activation = tf.nn.sigmoid(tf.matmul(x_clean, W) + b)

    epsilon = 1e-5
    cost = -tf.reduce_sum(y * w * tf.log(activation + epsilon) + (1 - y) * w * tf.log(1 - activation + epsilon)) / tf.reduce_sum(w)

    learning_rate = 0.001
    global_step = tf.Variable(0, name='global_step', trainable=False)
    optimizer = tf.train.GradientDescentOptimizer(learning_rate).minimize(cost, global_step=global_step)

    init = tf.global_variables_initializer()

    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    session = tf.Session(config=config)
    session.run(init)

    state = State(x, y, activation, cost, optimizer, session)
    state.w = w
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

    tf.reset_default_graph()
    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    session = tf.Session(config=config)
    saver = tf.train.import_meta_graph(obj[0])
    with tempfile.TemporaryDirectory() as path:
        with open(os.path.join(path, obj[1] + '.data-00000-of-00001'), 'w+b') as file1, open(
                os.path.join(path, obj[1] + '.index'), 'w+b') as file2:
            file1.write(bytes(obj[2]))
            file2.write(bytes(obj[3]))
        tf.train.update_checkpoint_state(path, obj[1])
        saver.restore(session, os.path.join(path, obj[1]))
    state = State(session=session)
    if len(obj) > 4:
        state.get_from_collection(obj[4])
        for i, extra in enumerate(obj[5:]):
            setattr(state, 'extra_{}'.format(i), extra)
    else:
        state.get_from_collection()

    return state


def apply(state, X):
    """
    Apply estimator to passed data.
    """
    r = state.session.run(state.activation, feed_dict={state.x: X}).flatten()
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
    state.session.run(state.optimizer, feed_dict={state.x: X, state.y: y, state.w: w})
    avg_cost = state.session.run(state.cost, feed_dict={state.x: X, state.y: y, state.w: w})
    if epoch % 1000 == 0:
        print("Epoch:", '%04d' % (epoch), "cost=", "{:.9f}".format(avg_cost))
    if epoch == 100000:
        return False
    return True


def end_fit(state):
    """
    Store tensorflow session in a graph
    """
    try:
        import tensorflow as tf
    except ImportError:
        print("Please install tensorflow: pip3 install tensorflow")
        sys.exit(1)

    keys = state.add_to_collection()
    saver = tf.train.Saver()
    with tempfile.TemporaryDirectory() as path:
        filename = saver.save(state.session, os.path.join(path, 'mymodel'))
        with open(filename + str('.data-00000-of-00001'), 'rb') as file1, open(filename + str('.index'), 'rb') as file2:
            data1 = file1.read()
            data2 = file2.read()
    meta_graph = saver.export_meta_graph()
    del state
    return [meta_graph, os.path.basename(filename), data1, data2, keys]
