#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import numpy as np
import tensorflow as tf


class State(object):
    """
    Tensorflow state
    """
    def __init__(self, x=None, y=None, activation=None, cost=None, optimizer=None, session=None):
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

    def add_to_collection(self):
        """ Add the stored members to the current tensorflow collection """
        tf.add_to_collection('x', self.x)
        tf.add_to_collection('y', self.x)
        tf.add_to_collection('activation', self.activation)
        tf.add_to_collection('cost', self.cost)
        tf.add_to_collection('optimizer', self.optimizer)

    def get_from_collection(self):
        """ Get members from the current tensorflow collection """
        self.x = tf.get_collection('x')[0]
        self.y = tf.get_collection('y')[0]
        self.activation = tf.get_collection('activation')[0]
        self.cost = tf.get_collection('cost')[0]
        self.optimizer = tf.get_collection('optimizer')[0]


def feature_importance(state):
    """
    Return a list containing the feature importances
    """
    return []


def get_model(number_of_features, number_of_events, parameters):
    """
    Return default tensorflow model
    """
    x = tf.placeholder("float", [None, number_of_features])
    y = tf.placeholder("float", [None])
    W = tf.Variable(tf.zeros([number_of_features, 1]))
    b = tf.Variable(tf.zeros([1]))
    activation = tf.nn.softmax(tf.matmul(x, W) + b)

    cost = tf.reduce_mean(-tf.reduce_sum(y * tf.log(activation), reduction_indices=1))

    learning_rate = parameters.get('learning_rate', 0.1)
    optimizer = tf.train.GradientDescentOptimizer(learning_rate).minimize(cost)

    init = tf.initialize_all_variables()
    session = tf.Session()
    session.run(init)
    return State(x, y, activation, cost, optimizer, session)


def load(obj):
    """
    Load Tensorflow estimator into state
    """
    session = tf.Session()
    saver = tf.import_meta_graph(obj)
    saver.restore(session, obj)
    state = State(session=session)
    stte.get_from_collection()
    return state


def apply(state, X):
    """
    Apply estimator to passed data.
    """
    result = state.session.run(state.activation, feed_dict={state.x: X})
    return result


def begin_fit(state):
    """
    Returns just the state object
    """
    return state


def partial_fit(state, X, y, w, Xtest, ytest, wtest, epoch):
    """
    Pass received data to tensorflow session
    """
    state.session.run(state.optimizer, feed_dict={state.x: X, state.y: y})
    avg_cost = state.session.run(state.cost, feed_dict={state.x: X, state.y: y}) / len(y)
    print("Epoch:", '%04d' % (epoch), "cost=", "{:.9f}".format(avg_cost))
    return True


def end_fit(state):
    """
    Store tensorflow session in a graph
    """
    state.add_to_collection()
    graph = tf.get_default_graph()
    saver = tf.train.Saver()
    meta_graph = tf.export_meta_graph(graph_def=graph.as_graph_def(), saver_def=saver.as_saver_def())
    return meta_graph
