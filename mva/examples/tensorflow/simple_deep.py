#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

import numpy as np
import tensorflow as tf
import basf2_mva
import basf2_mva_util
import pandas
import time

from basf2_mva_python_interface.tensorflow import State

old_time = time.time()


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):

    number_of_features *= 20
    tf.reset_default_graph()
    x = tf.placeholder(tf.float32, [None, number_of_features])
    y = tf.placeholder(tf.float32, [None, 1])

    def layer(x, shape, name, unit=tf.sigmoid):
        with tf.name_scope(name) as scope:
            weights = tf.Variable(tf.truncated_normal(shape, stddev=1.0 / np.sqrt(float(shape[0]))), name='weights')
            biases = tf.Variable(tf.constant(0.0, shape=[shape[1]]), name='biases')
            weight_decay = tf.reduce_sum(0.001 * tf.nn.l2_loss(weights))
            tf.add_to_collection('losses', weight_decay)  # for adding to the loss in loss model
            layer = unit(tf.matmul(x, weights) + biases)
        return layer

    inference_hidden1 = layer(x, [number_of_features, number_of_features], 'inference_hidden1')
    inference_hidden2 = layer(inference_hidden1, [number_of_features, number_of_features], 'inference_hidden2')
    inference_hidden3 = layer(inference_hidden2, [number_of_features, number_of_features], 'inference_hidden3')
    inference_hidden4 = layer(inference_hidden3, [number_of_features, number_of_features], 'inference_hidden4')
    inference_hidden5 = layer(inference_hidden4, [number_of_features, number_of_features], 'inference_hidden5')
    inference_hidden6 = layer(inference_hidden5, [number_of_features, number_of_features], 'inference_hidden6')
    inference_hidden7 = layer(inference_hidden6, [number_of_features, number_of_features], 'inference_hidden7')
    inference_hidden8 = layer(inference_hidden7, [number_of_features, number_of_features], 'inference_hidden8')
    inference_hidden9 = layer(inference_hidden8, [number_of_features, number_of_features], 'inference_hidden9')
    inference_hidden10 = layer(inference_hidden9, [number_of_features, number_of_features], 'inference_hidden10')
    inference_hidden11 = layer(inference_hidden10, [number_of_features, number_of_features], 'inference_hidden11')
    inference_hidden12 = layer(inference_hidden11, [number_of_features, number_of_features], 'inference_hidden12')
    inference_hidden13 = layer(inference_hidden12, [number_of_features, number_of_features], 'inference_hidden13')
    inference_hidden14 = layer(inference_hidden13, [number_of_features, number_of_features], 'inference_hidden14')
    inference_hidden15 = layer(inference_hidden14, [number_of_features, number_of_features], 'inference_hidden15')
    inference_activation = layer(inference_hidden15, [number_of_features, 1], 'inference_sigmoid', unit=tf.sigmoid)

    epsilon = 1e-5
    inference_loss = (-tf.reduce_sum(y * tf.log(inference_activation + epsilon) +
                                     (1.0 - y) * tf.log(1 - inference_activation + epsilon)) +
                      tf.reduce_sum(tf.get_collection('losses')))

    inference_optimizer = tf.train.AdamOptimizer(learning_rate=0.01)
    inference_minimize = inference_optimizer.minimize(inference_loss)

    init = tf.global_variables_initializer()

    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    session = tf.Session(config=config)

    session.run(init)
    state = State(x, y, inference_activation, inference_loss, inference_minimize, session)
    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Pass received data to tensorflow session
    """
    global old_time
    N = 100
    batch_size = 500
    epoch = 0
    indices = np.arange(len(X))
    for i in range(N):
        np.random.shuffle(indices)
        for pos in range(0, len(indices), batch_size):
            epoch += 1
            if pos + batch_size >= len(indices):
                break
            index = indices[pos: pos + batch_size]
            x_batch = np.repeat(X[index], 20, axis=1)
            y_batch = y[index]

            feed_dict = {state.x: x_batch, state.y: y_batch}
            state.session.run(state.optimizer, feed_dict=feed_dict)

            if epoch % 1000 == 0:
                avg_cost = state.session.run(state.cost, feed_dict=feed_dict)
                new_time = time.time()
                print("Time Difference", new_time - old_time)
                old_time = new_time
                print("Epoch:", '%04d' % (epoch), "cost=", "{:.9f}".format(avg_cost))
    return False

if __name__ == "__main__":

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_identifier = "Tensorflow"
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
                 'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)', 'daughterInvariantMass(1, 2)']
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "tensorflow"
    specific_options.m_steering_file = 'mva/examples/tensorflow/simple_deep.py'
    specific_options.m_normalize = True

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
