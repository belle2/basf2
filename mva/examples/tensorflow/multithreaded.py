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
import threading
import os
import sys
import tempfile

from basf2_mva_python_interface.tensorflow import State


def execute_train_op(state):
    """
    Do the actual training multithreaded.
    """
    global COORD
    try:
        epoch = 0
        while (not COORD.should_stop()):
            X, w, y = state.queue.dequeue_many(state.dequeue_batch_size)

            with tf.GradientTape() as tape:
                avg_cost = state.model.loss(state.model(X), y, w)
                grads = tape.gradient(avg_cost, state.model.trainable_variables)

            state.model.optimizer.apply_gradients(zip(grads, state.model.trainable_variables))

            if (epoch % 100 == 0):
                print(f'Step {epoch:d}: Train cost = {avg_cost:.4f}')
            epoch += 1

    except tf.errors.OutOfRangeError:
        print('No more items in closed queue, end of training.')
        COORD.request_stop()


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Build tensorflow graph, handles parameter and initialise variables
    """

    param = {'capacity': 1e6, 'min_after_dequeue': 800, 'batch_size': 500}

    if isinstance(parameters, dict):
        param.update(parameters)

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

            self.n_layers = 3
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
            epsilon = 1e-5
            diff_from_truth = tf.where(target_y == 1., predicted_y, 1. - predicted_y)
            cross_entropy = - tf.reduce_sum(w * tf.math.log(diff_from_truth + epsilon)) / tf.reduce_sum(w)
            return cross_entropy

    state = State(model=my_model())

    # defining queue, enqueue and dequeue-operation
    state.queue = tf.queue.RandomShuffleQueue(int(param['capacity']), int(param['min_after_dequeue']),
                                              [tf.float32, tf.float32, tf.float32],
                                              shapes=[[number_of_features], [1], [1]])

    state.dequeue_batch_size = int(param['batch_size'])

    # defining threads for training
    global COORD
    COORD = tf.train.Coordinator()
    state.threads = [threading.Thread(target=execute_train_op, args=(state,)) for i in range(2)]
    COORD.join(state.threads)
    return state


def begin_fit(state, Xtest, Stest, ytest, wtest, nBatches):
    """
    Starting training op async
    """
    for t in state.threads:
        t.start()
    return state


def partial_fit(state, X, S, y, w, epoch, batch):
    """
    Put data in the queue.
    """
    state.queue.enqueue_many([X, w, y])
    print(f"Queue Epoch: {epoch:d}, Queue Batch: {batch:d}, Queue Size: {state.queue.size():d}")
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

    # close the queue allowing the dequeue operation to grab the last batches
    state.queue.close()

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


if __name__ == "__main__":
    from basf2 import conditions, find_file
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    import basf2_mva
    import json

    train_file = find_file("mva/train_D0toKpipi.root", "examples")
    test_file = find_file("mva/test_D0toKpipi.root", "examples")

    training_data = basf2_mva.vector(train_file)
    testing_data = basf2_mva.vector(test_file)

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    general_options.m_treename = "tree"
    variables = ['p', 'pt', 'pz',
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
    general_options.m_spectators = basf2_mva.vector('M')
    general_options.m_target_variable = "isSignal"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "tensorflow"
    specific_options.m_steering_file = 'mva/examples/tensorflow/multithreaded.py'
    specific_options.m_nIterations = 100
    specific_options.m_mini_batch_size = 0

    general_options.m_identifier = "tensorflow_multithreaded"
    specific_options.m_config = json.dumps({'capacity': 2e3, 'min_after_dequeue': 500, 'batch_size': 500})

    import time
    import basf2_mva_util
    training_start = time.time()
    basf2_mva.teacher(general_options, specific_options)
    training_stop = time.time()
    training_time = training_stop - training_start
    method = basf2_mva_util.Method(general_options.m_identifier)
    inference_start = time.time()
    p, t = method.apply_expert(testing_data, general_options.m_treename)
    inference_stop = time.time()
    inference_time = inference_stop - inference_start
    auc = basf2_mva_util.calculate_roc_auc(p, t)
    print("Tensorflow", training_time, inference_time, auc)
