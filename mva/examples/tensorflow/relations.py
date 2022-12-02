#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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

    gpus = tf.config.list_physical_devices('GPU')
    if gpus:
        for gpu in gpus:
            tf.config.experimental.set_memory_growth(gpu, True)

    class my_model(tf.Module):

        def __init__(self, **kwargs):
            super().__init__(**kwargs)

            self.optimizer = tf.optimizers.Adam(0.01)
            self.pre_optimizer = tf.optimizers.Adam(0.01)

            def create_layer_variables(shape, name, activation_function):
                weights = tf.Variable(
                    tf.random.truncated_normal(shape, stddev=1.0 / np.sqrt(float(shape[0]))),
                    name=f'{name}_weights')
                biases = tf.Variable(tf.zeros(shape=[shape[1]]), name=f'{name}_biases')
                return weights, biases, activation_function

            n = int(number_of_features / 6)
            self.number_of_track_pairs = int(n * (n - 1) / 2)
            self.number_of_features_per_relation = 1
            self.parameters = parameters

            # build the relation net variables
            self.rel_net_vars = []
            shape = [12, 50, 50, 1]
            for i in range(len(shape) - 1):
                unit = tf.nn.tanh
                if i == len(shape) - 2:
                    unit = tf.nn.sigmoid
                self.rel_net_vars.append(create_layer_variables(shape[i:i + 2], f'relation_{i}', unit))

            self.inference_vars = []

            n_features_in = (self.number_of_track_pairs if self.parameters['use_relations'] else number_of_features)

            if self.parameters['use_feed_forward']:
                self.inference_vars.append(create_layer_variables([n_features_in, 50], 'inf_hidden_1', tf.nn.relu))
                self.inference_vars.append(create_layer_variables([50, 50], 'inf_hidden_2', tf.nn.relu))
                self.inference_vars.append(create_layer_variables([50, 1], 'inf_activation', tf.nn.sigmoid))
            else:
                self.inference_vars.append(create_layer_variables([n_features_in, 1], 'inf_activation', tf.nn.sigmoid))

        def dense(self, x, W, b, activation_function):
            return activation_function(tf.matmul(x, W) + b)

        @tf.function(input_signature=[tf.TensorSpec(shape=[None, number_of_features], dtype=tf.float32)])
        def prepare_x(self, x):
            """ prepare the features for use in the relation net """
            if not self.parameters['use_relations']:
                return x

            tracks = []
            for i in range(int(number_of_features / 6)):
                tracks.append(tf.slice(x, [0, i * 6], [-1, 6]))

            relations = []

            for feature_number in range(self.number_of_features_per_relation):
                # Loop over every combination of input groups.
                for counter, track1 in enumerate(tracks):
                    for track2 in tracks[counter + 1:]:
                        relations.append(self.relation_net(tf.concat([track1, track2], 1), self.rel_net_vars))

            # workaround required for saving the model.
            # Otherwise ValueErrors for trying to concatenate a list of length 0.
            if relations:
                return tf.concat(relations, 1)
            else:
                return x

        @tf.function(input_signature=[tf.TensorSpec(shape=[None, 12], dtype=tf.float32)])
        def pre_train(self, z):
            """build net for pre-training with the same shared variables """
            if not self.parameters['use_relations']:
                return z

            pre_training_relations = []
            for feature_number in range(self.number_of_features_per_relation):
                if self.parameters['pre_training_epochs'] > 0:
                    pre_training_relations.append(self.relation_net(z, self.rel_net_vars))

            if pre_training_relations:
                return tf.concat(pre_training_relations, 1)
            else:
                return z

        def relation_net(self, x, variables):
            """Build one relation net between 2 object using pre-build variables"""
            for layer in variables:
                x = self.dense(x, *layer)
            return x

        @tf.function(input_signature=[tf.TensorSpec(shape=[None, number_of_features], dtype=tf.float32)])
        def __call__(self, x):
            x = self.prepare_x(x)
            for variables in self.inference_vars:
                x = self.dense(x, *variables)
            return x

        @tf.function
        def loss(self, predicted_y, target_y, w):
            epsilon = 1e-5
            diff_from_truth = tf.where(target_y == 1., predicted_y, 1. - predicted_y)
            return - tf.reduce_sum(w * tf.math.log(diff_from_truth + epsilon)) / tf.reduce_sum(w)

    state = State(model=my_model())
    return state


def begin_fit(state, Xtest, Stest, ytest, wtest, nBatches):
    """Saves the training validation set for monitoring."""
    state.val_x = Xtest
    state.val_y = ytest
    state.val_z = Stest

    state.nBatches = nBatches
    return state


def partial_fit(state, X, S, y, w, epoch, batch):
    """Pass received data to tensorflow session"""

    def variable_is_trainable(var, parameters, pre_training=False):
        out = True
        if not parameters['use_relations'] and 'relation_' in var.name:
            out = False
        if pre_training and 'inf_' in var.name:
            out = False
        return out

    # pre training trains shared variables on only 2 lines.
    # In this case there is no relation net which have to compare two lines not hitting each other in a signal event.
    if state.model.parameters['pre_training_epochs'] > epoch:
        pre_trainable_variables = [
            x for x in state.model.trainable_variables if variable_is_trainable(
                x, state.model.parameters, True)]
        with tf.GradientTape() as tape:
            avg_cost = state.model.loss(state.model.pre_train(S), y, w)
            grads = tape.gradient(avg_cost, pre_trainable_variables)
        state.model.pre_optimizer.apply_gradients(zip(grads, pre_trainable_variables))

        if state.epoch == epoch:
            state.avg_costs.append()
        else:
            # started a new epoch, reset the avg_costs and update the counter
            print(f"Pre-Training: Epoch: {epoch-1:05d}, cost={np.mean(state.avg_costs):.5f}")
            state.avg_costs = [avg_cost]
            state.epoch = epoch

    # Training of the whole network.
    else:
        trainable_variables = [
            x for x in state.model.trainable_variables if variable_is_trainable(
                x, state.model.parameters, False)]
        with tf.GradientTape() as tape:
            avg_cost = state.model.loss(state.model(X), y, w)
            grads = tape.gradient(avg_cost, trainable_variables)

        state.model.optimizer.apply_gradients(zip(grads, trainable_variables))

        if batch == 0 and epoch == 0:
            state.avg_costs = [avg_cost]
        elif batch != state.nBatches-1:
            state.avg_costs.append(avg_cost)
        else:
            # started a new epoch, reset the avg_costs and update the counter
            if epoch == state.model.parameters['pre_training_epochs']:
                print(f"Pre-Training: Epoch: {epoch:05d}, cost={np.mean(state.avg_costs):.5f}")
            else:
                print(f"Epoch: {epoch:05d}, cost={np.mean(state.avg_costs):.5f}")

            early_stopper_flag = EARLY_STOPPER.check(np.mean(state.avg_costs))
            state.avg_costs = [avg_cost]
            return early_stopper_flag
    return True


if __name__ == "__main__":
    import os
    import pandas
    from uproot import recreate
    import tempfile
    import json

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
    number_of_events = 100000

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
            with recreate(os.path.join(path, filename)) as outfile:
                outfile['variables'] = df

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
        specific_options.m_steering_file = 'mva/examples/tensorflow/relations.py'
        specific_options.m_nIterations = 100
        specific_options.m_mini_batch_size = 100

        print('Train relational net with pre-training')
        general_options.m_identifier = os.path.join(path, 'relation_2.xml')
        specific_options.m_config = json.dumps({'use_relations': True, 'use_feed_forward': False, 'pre_training_epochs': 10})
        basf2_mva.teacher(general_options, specific_options)

        print('Train feed forward net')
        general_options.m_identifier = os.path.join(path, 'feed_forward.xml')
        specific_options.m_config = json.dumps({'use_relations': False, 'use_feed_forward': True, 'pre_training_epochs': 0})
        basf2_mva.teacher(general_options, specific_options)

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
        print('Apply special relational net')
        p3, t3 = method3.apply_expert(test_data, general_options.m_treename)

        print('Feed Forward Net AUC: ', basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p1, t1))
        print('Relational Net AUC: ', basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p2, t2))
        print('Relational Net with pre-training AUC: ', basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p3, t3))
