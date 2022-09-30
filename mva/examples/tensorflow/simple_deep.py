#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import numpy as np
import tensorflow as tf
import basf2_mva
import basf2_mva_util
import time

from basf2_mva_python_interface.tensorflow import State


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):

    number_of_features *= 5

    gpus = tf.config.list_physical_devices('GPU')
    if gpus:
        for gpu in gpus:
            tf.config.experimental.set_memory_growth(gpu, True)

    class my_model(tf.Module):

        def __init__(self, **kwargs):
            super().__init__(**kwargs)

            self.optimizer = tf.optimizers.Adam(0.01)

            def create_layer_variables(shape, name, activation_function):
                weights = tf.Variable(
                    tf.random.truncated_normal(shape, stddev=1.0 / np.sqrt(float(shape[0]))),
                    name=f'{name}_weights')
                biases = tf.Variable(tf.zeros(shape=[shape[1]]), name=f'{name}_biases')
                return weights, biases, activation_function

            self.n_layers = 10
            self.layer_variables = []

            shape = [number_of_features, number_of_features]
            for i in range(self.n_layers - 1):
                self.layer_variables.append(create_layer_variables(shape, f'inference_hidden{i}', tf.nn.relu))
            self.layer_variables.append(create_layer_variables([number_of_features, 1], 'inference_sigmoid', tf.nn.sigmoid))

        @tf.function(input_signature=[tf.TensorSpec(shape=[None, number_of_features], dtype=tf.float32)])
        def __call__(self, x):

            def dense(x, W, b, activation_function):
                return activation_function(tf.matmul(x, W) + b)

            for i in range(self.n_layers):
                x = dense(x, *self.layer_variables[i])
            return x

        @tf.function
        def loss(self, predicted_y, target_y, w):
            lam = 1e-15
            epsilon = 1e-5
            l2_loss = lam * tf.math.add_n([tf.nn.l2_loss(n) for n in self.trainable_variables
                                           if '_weights' in n.name and 'sigmoid' not in n.name])

            diff_from_truth = tf.where(target_y == 1., predicted_y, 1. - predicted_y)
            cross_entropy = - tf.reduce_sum(w * tf.math.log(diff_from_truth + epsilon)) / tf.reduce_sum(w)
            return cross_entropy + l2_loss

    state = State(model=my_model())
    state.epoch = 0
    state.avg_costs = []  # keeps track of the avg costs per batch over an epoch

    return state


def partial_fit(state, X, S, y, w, epoch, batch):
    """
    Pass batches of received data to tensorflow
    """
    X = np.repeat(X, 5, axis=1)

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


def apply(state, X):
    """
    Apply estimator to passed data.
    """
    X = np.repeat(X, 5, axis=1)
    r = state.model(X).numpy().flatten()
    return np.require(r, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


if __name__ == "__main__":
    from basf2 import conditions
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

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
                 'daughterInvM(0, 1)', 'daughterInvM(0, 2)', 'daughterInvM(1, 2)']
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "tensorflow"
    specific_options.m_steering_file = 'mva/examples/tensorflow/simple_deep.py'
    specific_options.m_normalize = True
    specific_options.m_nIterations = 100
    specific_options.m_mini_batch_size = 500

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
    auc = basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)
    print("Tensorflow", training_time, inference_time, auc)
