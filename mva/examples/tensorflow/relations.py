#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Dennis Weyland 2017

# This example serves as a basic example of implementing Relational networks into basf2 with tensorflow.
# As a toy example it will try to tell if 2 out of multiple lines are hitting each other in three dimensional space.
# Relevant Paper: https://arxiv.org/abs/1706.01427

import tensorflow as tf
from basf2_mva_python_interface.tensorflow import State
import numpy as np


class early_stopping():
    """ Using class to stop training early if it's not getting better"""

    def __init__(self):
        """ init class """
        #: counts how many times training is not getting better
        self.counter = 0
        #: saves best training result
        self.best_result = np.inf

    def check(self, cost):
        """
        Check if validation result is better than the best validation result.
        Decide if training should be continued.
        """
        if cost < self.best_result:
            self.counter = 0
            self.best_result = cost
        else:
            self.counter += 1
            if self.counter >= 20:
                return False
        return True

EARLY_STOPPER = early_stopping()


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """Building Graph inside tensorflow"""
    tf.reset_default_graph()
    x = tf.placeholder(tf.float32, [None, number_of_features])
    y = tf.placeholder(tf.float32, [None, 1])
    # Used as input for pre training data set.
    z = tf.placeholder(tf.float32, [None, number_of_spectators])

    def layer(x, shape, name, unit=tf.sigmoid):
        """Build one hidden layer in feed forward net"""
        with tf.name_scope(name) as scope:
            weights = tf.Variable(tf.truncated_normal(shape, stddev=1.0 / np.sqrt(float(shape[0]))), name='weights')
            biases = tf.Variable(tf.constant(0.0, shape=[shape[1]]), name='biases')
            layer = unit(tf.matmul(x, weights) + biases)
        return layer

    def build_relation_net_variables(shape, name):
        """Build the variables(not the net itself), who will be shared between multiple relations"""
        variables = []
        with tf.name_scope(name), tf.variable_scope(name):
            for i in range(len(shape) - 1):
                weights = tf.get_variable('weights_{}'.format(i),
                                          initializer=tf.truncated_normal(shape[i:i + 2],
                                                                          stddev=1.0 / np.sqrt(float(shape[0]))))
                biases = tf.get_variable('biases_{}'.format(i), initializer=tf.constant(0.0, shape=[shape[i + 1]]))
                variables.append([weights, biases])
        return variables

    def relation_net(x, variables):
        """Build one relation net between 2 object using pre-build variables"""
        net = [x]
        for layer in variables:
            if len(variables) != len(net):
                net.append(tf.nn.tanh(tf.matmul(net[-1], layer[0]) + layer[1]))
            else:
                return tf.nn.sigmoid(tf.matmul(net[-1], layer[0]) + layer[1])

    if parameters['use_relations']:
        # Group input according to relations.
        tracks = []
        [tracks.append(tf.slice(x, [0, i * 6], [-1, 6])) for i in range(int(number_of_features / 6))]

        # Number of features per reation. Each feature is a net with shared variables across all combinations.
        # Number of Features is also the number of different set of variables for relational nets.
        number_of_features_per_relation = 1
        relations = []
        pre_training_relations = []
        for feature_number in range(number_of_features_per_relation):
            # Build the variables, which will be shared across all combinations
            relational_variables = build_relation_net_variables([12, 50, 50, 1],
                                                                'tracks_relational_{}'.format(feature_number))
            # Loop over everx combination of input groups.
            for counter, track1 in enumerate(tracks):
                for track2 in tracks[counter + 1:]:
                    # Build the net wit pre-build variables.
                    relations.append(relation_net(tf.concat([track1, track2], 1), relational_variables))

            if parameters['pre_training_epochs'] > 0:
                # build net for pre-training with the same shared variables.
                pre_training_relations.append(relation_net(z, relational_variables))

        new_x = tf.concat(relations, 1)

    else:
        new_x = x

    if parameters['use_feed_forward']:
        print('Number of variables going into feed_forward:', int(new_x.get_shape()[1]))
        inference_hidden1 = layer(new_x, [int(new_x.get_shape()[1]), 50], 'inference_hidden1')
        inference_hidden2 = layer(inference_hidden1, [50, 50], 'inference_hidden2')
        inference_activation = layer(inference_hidden2, [50, 1], 'inference_sigmoid', unit=tf.sigmoid)
    else:
        print('Number of variables going into reduce_max:', int(new_x.get_shape()[1]))
        inference_activation = layer(new_x, [int(new_x.get_shape()[1]), 1], 'inference_sigmoid', unit=tf.sigmoid)
        print(inference_activation.get_shape())

    epsilon = 1e-5
    inference_loss = -tf.reduce_sum(y * tf.log(inference_activation + epsilon) +
                                    (1.0 - y) * tf.log(1 - inference_activation + epsilon))

    inference_optimizer = tf.train.AdamOptimizer(learning_rate=0.01)
    inference_minimize = inference_optimizer.minimize(inference_loss)

    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    session = tf.Session(config=config)

    state = State(x, y, inference_activation, inference_loss, inference_minimize, session)

    if parameters['pre_training_epochs'] > 0:
        # define training ops for pre-training and save them into state
        new_z = tf.concat(pre_training_relations, 1)
        pre_activation = layer(new_z, [int(new_z.get_shape()[1]), 1], 'pre_output')
        state.pre_loss = -tf.reduce_sum(y * tf.log(pre_activation + epsilon) +
                                        (1.0 - y) * tf.log(1 - pre_activation + epsilon))
        pre_optimizer = tf.train.AdamOptimizer(learning_rate=0.01)
        state.pre_minimize = pre_optimizer.minimize(state.pre_loss)

    state.pre_training_epochs = parameters['pre_training_epochs']
    state.z = z
    init = tf.global_variables_initializer()
    session.run(init)

    return state


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """Saves the training validation set for monitoring."""
    state.val_x = Xtest
    state.val_y = ytest
    state.val_z = Stest

    return state


def partial_fit(state, X, S, y, w, epoch):
    """Pass received data to tensorflow session"""
    feed_dict = {state.x: X, state.y: y, state.z: S}

    # pre training trains shared variables on only 2 lines.
    # In this case there is no relation net which have to compare two lines not hitting each other in a signal event.
    if state.pre_training_epochs > epoch:
        state.session.run(state.pre_minimize, feed_dict=feed_dict)
        if epoch % 1000 == 0:
            avg_cost = state.session.run(state.pre_loss, feed_dict={state.y: state.val_y, state.z: state.val_z})
            print("Pre-Training: Epoch:", '%04d' % (epoch), "cost=", "{:.9f}".format(avg_cost))
            return True

    # Training of the whole network.
    else:
        state.session.run(state.optimizer, feed_dict=feed_dict)
        if epoch % 1000 == 0:
            avg_cost = state.session.run(state.cost, feed_dict={state.y: state.val_y, state.x: state.val_x})
            print("Epoch:", '%04d' % (epoch), "cost=", "{:.9f}".format(avg_cost))
            return EARLY_STOPPER.check(avg_cost)

    return True

if __name__ == "__main__":
    import os
    import pandas
    from root_pandas import to_root
    import tempfile
    import json

    import basf2
    import basf2_mva
    import basf2_mva_util

    # ##############Building Data samples ###########################
    # This is  a dataset for testing relational nets.
    # It consists of number_total_lines lines in 3 dimensional space.
    # Each line has 6 variables.
    # In apprx. half of the cases, two lines are hitting each other.
    # This is considered a signal event.
    # Training results differs from the number of total lines.

    variables = []
    # try using 10 lines and see what happens
    number_total_lines = 5
    # Names for the training data set
    for i in range(number_total_lines):
        variables += ['px_' + str(i), 'py_' + str(i), 'pz_' + str(i), 'dx_' + str(i), 'dy_' + str(i),
                      'dz_' + str(i)]
    # Names for the spectator variables.
    # Used as input variables for pre-training.
    spectators = ['Spx1', 'Spy1', 'Spz1', 'Sdx1', 'Sdy1', 'Sdz1', 'Spx2', 'Spy2', 'Spz2', 'Sdx2', 'Sdy2', 'Sdz2']
    # Number of events in training and test root file.
    number_of_events = 1000000

    def build_signal_event():
        """Building two lines which are hitting each other"""
        p_vec1, p_vec2 = np.random.normal(size=3), np.random.normal(size=3)
        v_cross = np.random.normal(size=3)
        epsilon1, epsilon2 = np.random.rand() * 2 - 1, np.random.rand() * 2 - 1
        v_vec1 = v_cross + (p_vec1 * epsilon1)
        v_vec2 = v_cross + (p_vec2 * epsilon2)
        return np.concatenate([p_vec1, v_vec1]), np.concatenate([p_vec2, v_vec2])

    # This path will delete itself with all data in it after end of program.
    with tempfile.TemporaryDirectory() as path:
        for filename in ['train.root', 'test.root']:
            print('Building ' + filename)
            # Use random numbers to build all training and spectator variables.
            data = np.random.normal(size=[number_of_events, number_total_lines * 6 + 12])
            target = np.zeros([number_of_events], dtype=bool)

            # Overwrite for half of the variables some lines so that they are hitting each other.
            # Write them also at the end for the spectators.
            for index, sample in enumerate(data):
                if np.random.rand() > 0.5:
                    target[index] = True
                    i1, i2 = int(np.random.rand() * number_total_lines), int(np.random.rand() * (number_total_lines - 1))
                    i2 = (i1 + i2) % number_total_lines
                    track1, track2 = build_signal_event()
                    data[index, i1 * 6:(i1 + 1) * 6] = track1
                    data[index, i2 * 6:(i2 + 1) * 6] = track2
                    data[index, number_total_lines * 6:] = np.append(track1, track2)

            # Saving all variables in root files
            dic = {}
            for i, name in enumerate(variables + spectators):
                dic.update({name: data[:, i]})
            dic.update({'isSignal': target})

            df = pandas.DataFrame(dic, dtype=np.float32)
            to_root(df, os.path.join(path, filename), tree_key='variables')

        # ##########################Do Training#################################
        # Do a comparison of different Nets for this task.

        general_options = basf2_mva.GeneralOptions()
        general_options.m_datafiles = basf2_mva.vector(os.path.join(path, 'train.root'))
        general_options.m_treename = "variables"
        general_options.m_variables = basf2_mva.vector(*variables)
        general_options.m_target_variable = "isSignal"
        general_options.m_spectators = basf2_mva.vector(*spectators)

        specific_options = basf2_mva.PythonOptions()
        specific_options.m_framework = "tensorflow"
        specific_options.m_steering_file = 'toy_relations.py'
        specific_options.m_nIterations = 100
        specific_options.m_mini_batch_size = 100
        specific_options.m_training_fraction = 0.999

        print('Train relational net with pre-training')
        general_options.m_identifier = os.path.join(path, 'relation_2.xml')
        specific_options.m_config = json.dumps({'use_relations': True, 'use_feed_forward': False, 'pre_training_epochs': 30000})
        basf2_mva.teacher(general_options, specific_options)

        # Train normal feed forward Net:
        print('Train feed forward net')
        general_options.m_identifier = os.path.join(path, 'feed_forward.xml')
        specific_options.m_config = json.dumps({'use_relations': False, 'use_feed_forward': True, 'pre_training_epochs': 0})
        basf2_mva.teacher(general_options, specific_options)

        # Train Relation Net:
        print('Train relational net')
        general_options.m_identifier = os.path.join(path, 'relation.xml')
        specific_options.m_config = json.dumps({'use_relations': True, 'use_feed_forward': True, 'pre_training_epochs': 0})
        basf2_mva.teacher(general_options, specific_options)

        # ########################Compare Results####################################

        method1 = basf2_mva_util.Method(os.path.join(path, 'feed_forward.xml'))
        method2 = basf2_mva_util.Method(os.path.join(path, 'relation.xml'))
        method3 = basf2_mva_util.Method(os.path.join(path, 'relation_2.xml'))

        test_data = basf2_mva.vector(os.path.join(path, 'test.root'))
        print('Apply feed forward net')
        p1, t1 = method1.apply_expert(test_data, general_options.m_treename)
        print('Apply relational net')
        p2, t2 = method2.apply_expert(test_data, general_options.m_treename)
        print('Aplly special relational net')
        p3, t3 = method3.apply_expert(test_data, general_options.m_treename)

        print('Feed Forward Net AUC: ', basf2_mva_util.calculate_roc_auc(p1, t1))
        print('Relational Net AUC: ', basf2_mva_util.calculate_roc_auc(p2, t2))
        print('Relational Net with pre-training AUC: ', basf2_mva_util.calculate_roc_auc(p3, t3))
