#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

import numpy as np
import tensorflow as tf
import basf2_mva


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):

    x = tf.placeholder(tf.float32, [None, number_of_features])
    y = tf.placeholder(tf.float32, [None])
    z = tf.placeholder(tf.float32, [None, number_of_spectators])
    w = tf.placeholder(tf.float32, [None])

    def layer(x, shape, name, unit=tf.sigmoid):
        with tf.name_scope(name) as scope:
            weights = tf.Variable(tf.truncated_normal(shape, stddev=1.0 / math.sqrt(float(shape[0]))), name='weights')
            biases = tf.Variable(tf.constant(0.0, shape=[shape[1]]), name='biases')
            layer = unit(tf.matmul(x, weights) + biases)
        return layer

    inference_hidden1 = layer(x, [n_features, 20], 'inference_hidden1')
    inference_hidden2 = layer(inference_hidden1, [20, 20], 'inference_hidden2')
    inference_hidden3 = layer(inference_hidden2, [20, 20], 'inference_hidden3')
    inference_hidden4 = layer(inference_hidden3, [20, 20], 'inference_hidden4')
    inference_activation = layer(inference_hidden4, [20, 1], 'sigmoid', unit=tf.sigmoid)

    epsilon = 1e-5
    inference_loss = -tf.reduce_sum(y * w * tf.log(inference_acitvation + epsilon) +
                                    (1.0 - y) * w * tf.log(1 - inference_activation + epsilon))

    for i in range(number_of_spectators):
        z_single = tf.slice(z, [i], [1])
        adversary_hidden1 = layer(inference_activation, [1, 20], 'adversary_hidden1_{}'.format(i), unit=tf.tanh)
        adversary_hidden2 = layer(hidden1, [20, 20], 'adversary_hidden2_{}'.format(i), unit=tf.nn.relu)
        adversary_means = layer(hidden2, [20, 4], 'adversary_means_{}'.format(i), unit=tf.identity)
        adversary_widths = layer(hidden2, [20, 4], 'adversary_width_{}'.format(i), unit=tf.exp)
        adversary_fractions_not_normed = layer(hidden2, [20, 4], 'adversary_fractions_{}'.format(i), unit=tf.identity)
        adversary_fractions = tf.nn.softmax(adversary_fractions_not_normed)
        adversary_activation = tf.reduce_sum(adversary_fractions *
                                             tf.exp(-(adversary_means - z_single) * (adversary_means - z_single) /
                                                    (2 * adversary_widths)))
        adversary_loss = -tf.reduce_sum(w * tf.log(adversary_activation + epsilon))
        tf.add_to_collection('adversary_losses', adversary_loss)

    adversary_loss = tf.add_n(tf.get_collection('adversary_losses'), name='adversary_loss')
    total_loss = inference_loss - parameters.get('lambda', 50) * adversary_loss

    adversary_vars = [
        'adversary_hidden1_{}/weights:0',
        'adversary_hidden1_{}/biases:0',
        'adversary_hidden2_{}/weights:0',
        'adversary_hidden2_{}/biases:0',
        'adversary_means_{}/weights:0',
        'adversary_width_{}/biases:0',
        'adversary_fractions_{}/weights:0',
        'adversary_fractions_{}/biases:0',
    ]
    adversary_vars = [v.format(i) for i in range(number_of_spectators) for v in adversary_vars]

    adversary_optimizer = tf.train.AdamOptimizer(learning_rate=parameters.get('learning_rate', 0.001))
    adversary_minimize = optimizer.minimize(adversary_loss, var_list=adversary_vars)

    inference_vars = [
        'inference_hidden1/weights:0',
        'inference_hidden1/biases:0',
        'inference_hidden2/weights:0',
        'inference_hidden2/biases:0',
        'inference_hidden3/weights:0',
        'inference_hidden3/biases:0',
        'inference_hidden4/weights:0',
        'inference_hidden4/biases:0',
        'inference_sigmoid/weights:0',
        'inference_sigmoid/biases:0',
    ]

    inference_optimizer = tf.train.AdamOptimizer(learning_rate=parameters.get('learning_rate', 0.001))
    inference_minimize = optimizer.minimize(total_loss, var_list=inference_vars)

    init = tf.global_variables_initializer()
    session = tf.Session()
    session.run(init)
    state = State(x, y, inference_acitvation, total_loss, inference_minimize, session)
    state.adversary_cost = adversary_loss
    state.adversary_optimizer = adversary_minimize
    state.w = w
    state.K_i = 0
    state.K = parameters.get('adversary_steps', 10)
    state.z = z

    return state


def partial_fit(state, X, S, y, w, Xtest, Stest, ytest, wtest, epoch):
    """
    Pass received data to tensorflow session
    """
    if state.K_i == state.K:
        state.session.run(state.optimizer, feed_dict={state.x: X, state.y: y, state.w: w, state.z: S})
        avg_cost = w * state.session.run(state.cost, feed_dict={state.x: X, state.y: y, state.z: S}) / np.sum(w)
        print("Epoch:", '%04d' % (epoch), "cost=", "{:.9f}".format(avg_cost))
        state.K_i = 0
    else:
        state.session.run(state.adversary_optimizer, feed_dict={state.x: X, state.y: y, state.w: w, state.z: S})
        state.K_i += 1
    return True


if __name__ == "__main__":

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_identifier = "Adversary"
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector('M', 'p', 'pz', 'daughter(0, Kid)')
    general_options.m_target_variable = "isSignal"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "tensorflow"
    specific_options.m_model = 'mva/examples/tensorflow_adversary.py'
    basf2_mva.teacher(general_options, specific_options)
