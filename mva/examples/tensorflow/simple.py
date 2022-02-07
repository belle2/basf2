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
    """
    Return simple tensorflow model
    """

    gpus = tf.config.list_physical_devices('GPU')
    if gpus:
        for gpu in gpus:
            tf.config.experimental.set_memory_growth(gpu, True)

    class my_model(tf.Module):

        def __init__(self, **kwargs):
            super().__init__(**kwargs)

            self.optimizer = tf.optimizers.Adam(0.01)
            shape = [number_of_features, number_of_features]
            self.W_hidden1 = tf.Variable(
                tf.random.truncated_normal(shape, stddev=1.0 / np.sqrt(float(shape[0]))),
                name='hidden1_weights')
            self.b_hidden1 = tf.Variable(tf.zeros(shape=[shape[1]]), name='hidden1_biases')

            shape = [number_of_features, 1]
            self.W_activation = tf.Variable(
                tf.random.truncated_normal(shape, stddev=1.0 / np.sqrt(float(shape[0]))),
                name='activation_weights')
            self.b_activation = tf.Variable(tf.zeros(shape=[shape[1]]), name='activation_biases')

        @tf.function(input_signature=[tf.TensorSpec(shape=[None, number_of_features], dtype=tf.float32)])
        def __call__(self, x):

            # __call__ cannot create any new Variables
            def dense(x, W, b, activation_function):
                return activation_function(tf.matmul(x, W) + b)

            hidden1 = dense(self.clean_nans(x), self.W_hidden1, self.b_hidden1, tf.nn.sigmoid)
            activation = dense(hidden1, self.W_activation, self.b_activation, tf.nn.sigmoid)
            return activation

        @tf.function
        def clean_nans(self, x):
            return tf.where(tf.math.is_nan(x), tf.zeros_like(x), x)

        @tf.function
        def loss(self, predicted_y, target_y, w):
            epsilon = 1e-5
            diff_from_truth = tf.where(target_y == 1., predicted_y, 1. - predicted_y)
            return - tf.reduce_sum(w * tf.math.log(diff_from_truth + epsilon)) / tf.reduce_sum(w)

    state = State(model=my_model())
    return state


if __name__ == "__main__":
    from basf2 import conditions
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_identifier = "Simple"
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
    specific_options.m_steering_file = 'mva/examples/tensorflow/simple.py'
    specific_options.m_nIterations = 100
    specific_options.m_mini_batch_size = 100
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
    auc = basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)
    print("Tensorflow", training_time, inference_time, auc)
