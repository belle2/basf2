#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Dennis Weyland 2017

import numpy as np
import tensorflow as tf
import threading
import signal

from basf2_mva_python_interface.tensorflow import State

#: Global
CONTINUE_ENQUEUE = True


def signal_handler(signal, frame):
    """
    Used to safely stopp training. This can only be used in main thread,
    so depending on the training style, it can take a while before execution.
    """
    print('Stopping Training safely')
    global COORD
    global CONTINUE_ENQUEUE
    COORD.request_stop()
    CONTINUE_ENQUEUE = False


def execute_train_op(state):
    """
    Do the actual training multithreaded.
    """
    global COORD
    try:
        epoch = 0
        while (not COORD.should_stop()):
            xy_list = state.session.run(state.dequeue)
            feed_dict = {state.x: xy_list[0], state.y: xy_list[1]}

            state.session.run(state.optimizer, feed_dict=feed_dict)

            if (epoch % 100 == 0):
                train_eva = state.session.run(state.cost, feed_dict=feed_dict)
                print('Step %d: Train cost = %.4f' % (epoch, train_eva))

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

    x = tf.placeholder("float", [None, number_of_features])
    y = tf.placeholder("float", [None, 1])

    def layer(x, shape, name, unit=tf.sigmoid):
        with tf.name_scope(name) as scope:
            weights = tf.Variable(tf.truncated_normal(shape, stddev=1.0 / np.sqrt(float(shape[0]))), name='weights')
            biases = tf.Variable(tf.constant(0.0, shape=[shape[1]]), name='biases')
            layer = unit(tf.matmul(x, weights) + biases)
        return layer

    inference_hidden1 = layer(x, [number_of_features, 20], 'inference_hidden1')
    inference_hidden2 = layer(inference_hidden1, [20, 20], 'inference_hidden2')
    inference_hidden3 = layer(inference_hidden2, [20, 20], 'inference_hidden3')
    inference_hidden4 = layer(inference_hidden3, [20, 20], 'inference_hidden4')
    inference_activation = layer(inference_hidden4, [20, 1], 'inference_sigmoid', unit=tf.sigmoid)

    epsilon = 1e-5
    inference_loss = -tf.reduce_sum(y * tf.log(inference_activation + epsilon) +
                                    (1.0 - y) * tf.log(1 - inference_activation + epsilon))

    inference_optimizer = tf.train.AdamOptimizer()
    inference_global_step = tf.Variable(0, name='inference_global_step', trainable=False)
    inference_minimize = inference_optimizer.minimize(inference_loss, global_step=inference_global_step)

    init = tf.global_variables_initializer()
    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    session = tf.Session(config=config)
    session.run(init)
    tmp_state = State(x, y, inference_activation, inference_loss, inference_minimize, session)

    # All steps below are needed in state for training but not needed for mva_expert

    # defining queue, enqueue and dequeue-operation
    tmp_state.queue = tf.RandomShuffleQueue(int(param['capacity']), int(param['min_after_dequeue']), [tf.float32, tf.float32],
                                            shapes=[[number_of_features], [1]])
    tmp_state.enqueue = tmp_state.queue.enqueue_many([x, y])
    tmp_state.dequeue = tmp_state.queue.dequeue_many(int(param['batch_size']))

    # defining thread for training
    global COORD
    COORD = tf.train.Coordinator()
    tmp_state.thread = threading.Thread(target=execute_train_op, args=(tmp_state,))

    return tmp_state


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """
    Starting training op async
    """
    state.thread.start()

    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Put data in the queue.
    """
    signal.signal(signal.SIGINT, signal_handler)

    state.session.run(state.enqueue, feed_dict={state.x: X, state.y: np.reshape(y, ((len(y), 1)))})

    print("Queue Epoch: %d, Queue Size: %d" % (epoch, state.session.run(state.queue.size())))

    return CONTINUE_ENQUEUE


if __name__ == "__main__":
    import basf2_mva
    import json

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
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
                 'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)', 'daughterInvariantMass(1, 2)']

    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_spectators = basf2_mva.vector('M')
    general_options.m_target_variable = "isSignal"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "tensorflow"
    specific_options.m_steering_file = 'mva/examples/tensorflow/multithreaded.py'
    specific_options.m_nIterations = 400
    specific_options.m_mini_batch_size = 100000

    general_options.m_identifier = "tensorflow_multithreaded"
    specific_options.m_config = json.dumps({'capacity': 2e3, 'min_after_dequeue': 600, 'batch_size': 500})
    basf2_mva.teacher(general_options, specific_options)
