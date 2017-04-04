#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

import numpy as np
import tensorflow as tf
import basf2_mva
import pandas

from basf2_mva_python_interface.tensorflow import State


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):

    tf.reset_default_graph()
    x = tf.placeholder(tf.float32, [None, number_of_features])
    y = tf.placeholder(tf.float32, [None, 1])

    def layer(x, shape, name, unit=tf.sigmoid):
        with tf.name_scope(name) as scope:
            weights = tf.Variable(tf.truncated_normal(shape, stddev=1.0 / np.sqrt(float(shape[0]))), name='weights')
            biases = tf.Variable(tf.constant(0.0, shape=[shape[1]]), name='biases')
            layer = unit(tf.matmul(x, weights) + biases)
        return layer

    inference_hidden1 = layer(x, [number_of_features, 20], 'inference_hidden1')
    inference_hidden2 = layer(inference_hidden1, [20, 20], 'inference_hidden2')
    inference_activation = layer(inference_hidden2, [20, 1], 'inference_sigmoid', unit=tf.sigmoid)

    epsilon = 1e-5
    inference_loss = -tf.reduce_sum(y * tf.log(inference_activation + epsilon) +
                                    (1.0 - y) * tf.log(1 - inference_activation + epsilon))

    inference_optimizer = tf.train.AdamOptimizer(learning_rate=0.01)
    inference_minimize = inference_optimizer.minimize(inference_loss)

    init = tf.global_variables_initializer()

    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    session = tf.Session(config=config)

    session.run(init)
    state = State(x, y, inference_activation, inference_loss, inference_minimize, session)
    return state


def partial_fit(state, X, S, y, w, Xtest, Stest, ytest, wtest, epoch):
    """
    Pass received data to tensorflow session
    """
    feed_dict = {state.x: X, state.y: y}
    state.session.run(state.optimizer, feed_dict=feed_dict)

    if epoch % 100 == 0:
        avg_cost = state.session.run(state.cost, feed_dict=feed_dict)
        print("Epoch:", '%04d' % (epoch), "cost=", "{:.9f}".format(avg_cost))

    if epoch >= 20000:
        return False
    return True


if __name__ == "__main__":

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_identifier = "Simple"
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector('M', 'p', 'pz')
    general_options.m_target_variable = "isSignal"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "tensorflow"
    specific_options.m_steering_file = 'mva/examples/tensorflow_simple.py'
    specific_options.m_nIterations = 0  # Feed data until partial fit returns False
    specific_options.m_mini_batch_size = 100
    basf2_mva.teacher(general_options, specific_options)
