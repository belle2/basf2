#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Dennis Weyland 2017

import numpy as np
import tensorflow as tf
import math
import os
import threading
import time
import signal
from basf2_mva_python_interface.tensorflow import State
import pickle

# Array for saving evaluation score in tensorflow summary folder
VAL_ARRAY = []
# Array for saving Validation samples
VAL_QUEUE = []
# Global
CONTINUE_ENQUEUE = True


class save_decider():
    """
    Decides which file in tensorflow summary folder is used for storing variables by comparing the evaluation score of
    the independent validation dataset defined by training fraction.
    If training fraction is 1. The latest file will be used.
    """

    def __init__(self):
        self.lowest_value = np.inf

    def check(self, value, save_state):
        if self.lowest_value > value:
            self.save_state = save_state
            self.lowest_value = value
        if self.lowest_value == np.inf and value == np.nan:
            self.save_state = save_state

    def get_savestate(self):
        return self.save_state

SAVER_DECIDER = save_decider()


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


class preprocessor():
    """
    Preprocessing Python class.
    export_state returns a pickable state to rebuild class for mva expert
    """

    def __init__(self, state=None):
        if state is None:
            self.state = {'binning_array': [], 'number_of_bins': 0}
        else:
            self.state = state

    def fit(self, x, number_of_bins=100):
        for variable in range(len(x[0, :])):
            self.state['binning_array'].append(np.percentile(np.nan_to_num(x[:, variable]),
                                                             range(number_of_bins)))
        self.state['number_of_bins'] = number_of_bins

    def apply(self, x):
        for variable in range(len(x[0, :])):
            x[:, variable] = np.digitize(np.nan_to_num(x[:, variable]),
                                         self.state['binning_array'][variable]) / self.state['number_of_bins']
        return x

    def export_state(self):
        return self.state

PREPROCESSOR = preprocessor()


def inference_model(x, num_of_features, par_weight_decay, reuse_model=False):
    """
    Build Network topology
    :param x: tensorflow placeholder for feeding the data
    :param num_of_features: dimension of feature vector
    :param par_weight_decay: weight decay factor used for training
    :param reuse_model: Used for building tensorflow graphs with shared weights.
                        Param has to be False for creating new weights
    :return: tensorflow output node of Network topology
    """
    with tf.name_scope('inference'):
        def layer(x, shape, name, unit=tf.nn.tanh):
            with tf.name_scope(name), tf.variable_scope(name, reuse=reuse_model):  # prevent names to duplicate
                weights = tf.get_variable('weights', initializer=tf.truncated_normal(shape, stddev=1. / math.sqrt(float(shape[0]))))
                tf.summary.histogram('weights', weights)
                biases = tf.get_variable('biases', initializer=tf.constant(0.0, shape=[shape[1]]))
                tf.summary.histogram('biases', biases)
                weight_decay = tf.reduce_sum(par_weight_decay * tf.nn.l2_loss(weights))
                tf.add_to_collection('losses', weight_decay)  # for adding to the loss in loss model
                layer = unit(tf.matmul(x, weights) + biases)
                tf.summary.histogram('layer_output', layer)
            return layer

        # convert nan to 0 in x
        x_clean = tf.select(tf.is_nan(x), tf.ones_like(x) * 0., x)
        tf.summary.histogram('clean_input', x_clean)
        hidden0 = layer(x_clean, [num_of_features, 180], 'hidden0')
        hidden1 = layer(hidden0, [180, 120], 'hidden1')
        hidden2 = layer(hidden1, [120, 60], 'hidden2')
        hidden3 = layer(hidden2, [60, 30], 'hidden3')
        output = layer(hidden3, [30, 1], 'output', tf.sigmoid)
    return output


def loss_model(y, p):
    """
    Defining a loss (here including weight punishment)
    :param y: labeled Data
    :param p: prediction of the network
    :return: tensorflow node for loss calculation
    """
    with tf.name_scope('loss'):
        tf.summary.scalar('network_output', tf.reduce_sum(p))
        loss = -tf.reduce_sum(y * tf.log(p + 0.00001) + (1.0 - y) * tf.log(1.00001 - p))
        tf.summary.scalar('ce_error', loss)
        tf.summary.scalar('weight_decay', tf.reduce_sum(tf.get_collection('losses')))
        tf.add_to_collection('losses', loss)
    return tf.add_n(tf.get_collection('losses'), name='total_loss')  # returns tensor


def training_operation(loss, par_learning_rate):
    """
    Describes how the network should be trained
    :param loss: tensorflow node for loss calculation
    :param par_learning_rate: tuple for learning rate, which contains:
        Starting learning rate, decay factor and steps after decay factor applies
    :return: tensorflow node for training operation
    """
    with tf.name_scope('gradients'):
        tf.summary.scalar(loss.op.name, loss)
        global_step = tf.Variable(0, name='global_step', trainable=False)
        learning_rate = tf.train.exponential_decay(par_learning_rate[0], global_step,
                                                   par_learning_rate[1], par_learning_rate[2], staircase=True)
        tf.summary.scalar('learning_rate', learning_rate)
        optimizer = tf.train.AdamOptimizer(learning_rate=learning_rate)
        gradients = optimizer.compute_gradients(loss)
        for gradient, variable in gradients:
            tf.summary.histogram(variable.name + '_gradients', gradient)
        train_op = optimizer.apply_gradients(gradients, global_step=global_step)
    return train_op


def evaluation_operation(y, p, val_y, val_p, training_fraction):
    """
    Defines evaluation operation for monitoring training each fro training and validation sample.
    :param y: label for training data
    :param p: prediction for training data
    :param val_y: label for validation data
    :param val_p: prediction for validation data
    :param training_fraction: training_fraction definend in python options
    :return: nodes for calculation evaluation
    """
    with tf.name_scope('evaluation'):
        train_accuracy = -tf.reduce_sum(y * tf.log(p + 0.00001) + (1.0 - y) * tf.log(1.00001 - p))
        tf.summary.scalar('train_accuracy', train_accuracy)
        val_accuracy = None
        if training_fraction < 1:
            val_accuracy = -tf.reduce_sum(val_y * tf.log(val_p + 0.00001) + (1.0 - val_y) * tf.log(1.00001 - val_p))
            tf.summary.scalar('val_accuracy', val_accuracy)
    return train_accuracy, val_accuracy


def execute_train_op(state):
    """
    Do the actual training multithreaded.
    """
    start_time = time.time()
    global COORD
    try:
        epoch = 0
        while (not COORD.should_stop()) and epoch <= state.param['max_epoch']:
            xy_list = state.session.run(state.dequeue)
            feed_dict = {state.x: xy_list[0], state.y: xy_list[1], state.val_x: VAL_QUEUE[0].astype(np.float32),
                         state.val_y: VAL_QUEUE[1].astype(np.float32)}
            if state.param['items_permanent_in_queue']:
                state.session.run(state.enqueue, feed_dict=feed_dict)

            state.session.run(state.optimizer, feed_dict=feed_dict)

            if (epoch % state.param['n_step_summaries'] == 0) or epoch == state.param['max_epoch']:
                train_eva = state.session.run(state.train_evaluation, feed_dict=feed_dict)
                if state.training_fraction < 1:
                    val_eva = state.session.run(state.val_evaluation, feed_dict=feed_dict)
                else:
                    val_eva = np.nan

                print('Step %d (%.3f perc): Train_Accuracy = %.4f, Val_Accuracy = %.8f (%s)' %
                      (epoch, epoch / state.param['max_epoch'] * 100,
                       train_eva, val_eva, duration(start_time, time.time())))
                VAL_ARRAY.append([epoch, val_eva])
                summary_str = state.session.run(state.summary_op, feed_dict=feed_dict)
                state.summary_writer.add_summary(summary_str, epoch)

            if (epoch % state.param['n_step_save_model'] == 0) or epoch == state.param['max_epoch']:
                state.saver.save(state.session, os.path.join(state.param['tensorflow_folder'],
                                                             'mymodel'), global_step=epoch)
                SAVER_DECIDER.check(val_eva, tf.train.latest_checkpoint(state.param['tensorflow_folder']))
            epoch += 1

    except tf.errors.OutOfRangeError:
        print('No more items in closed queue, end of training.')
        COORD.request_stop()
        state.saver.save(state.session, os.path.join(state.param['tensorflow_folder'], 'mymodel'), global_step=epoch)


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Build tensorflow graph, handles parameter and initialise variables
    """
    param = {'visible_gpu': '', 'capacity': 10000000, 'min_after_dequeue': 20, 'max_epoch': np.inf,
             'batch_size': 500, 'tensorflow_folder': 'tensorflow_summary', 'clean_summary_folder': False,
             'weight_decay': 0.001, 'learning_rate': (0.001, 10000, 0.95), 'n_step_summaries': 1000, 'n_step_save_model': 10000,
             'n_step_queue_epoch': 0, 'items_permanent_in_queue': False}

    if isinstance(parameters, dict):
        param.update(parameters)

    if param['visible_gpu'] != '':
        os.environ["CUDA_VISIBLE_DEVICES"] = param['visible_gpu']

    if param['clean_summary_folder']:
        delete_all_files_in_folders([param['tensorflow_folder']])

    x = tf.placeholder("float", [None, number_of_features])
    y = tf.placeholder("float", [None, 1])
    val_x = tf.placeholder("float", [None, number_of_features])
    val_y = tf.placeholder("float", [None, 1])

    activation = inference_model(x, number_of_features, param['weight_decay'])
    cost = loss_model(y, activation)
    optimizer = training_operation(cost, param['learning_rate'])

    val_activation = inference_model(val_x, number_of_features, param['weight_decay'], True)

    init = tf.global_variables_initializer()
    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    session = tf.Session(config=config)
    session.run(init)
    tmp_state = State(x, y, activation, cost, optimizer, session)

    # All steps below are needed in state for training but not needed for mva_expert

    # Give parameters to state
    tmp_state.param = param
    tmp_state.training_fraction = training_fraction
    tmp_state.val_x, tmp_state.val_y = val_x, val_y

    # defining queue, enqueue and dequeue-operation
    tmp_state.queue = tf.RandomShuffleQueue(round(param['capacity'] * training_fraction),
                                            round(param['min_after_dequeue'] * training_fraction),
                                            [tf.float32, tf.float32], shapes=[[number_of_features], [1]])
    tmp_state.enqueue = tmp_state.queue.enqueue_many([x, y])
    tmp_state.dequeue = tmp_state.queue.dequeue_many(round(param['batch_size'] * training_fraction))

    # defining op for evaluation
    tmp_state.train_evaluation, tmp_state.val_evaluation = evaluation_operation(y, activation, val_y,
                                                                                val_activation,
                                                                                training_fraction)
    # defining thread for training
    global COORD
    COORD = tf.train.Coordinator()
    tmp_state.thread = threading.Thread(target=execute_train_op, args=(tmp_state,))

    # Define items for saving
    tmp_state.saver = tf.train.Saver(max_to_keep=1000)
    tmp_state.summary_writer = tf.summary.FileWriter(param['tensorflow_folder'], graph=session.graph,
                                                     max_queue=param['n_step_summaries'])
    tmp_state.summary_op = tf.summary.merge_all()

    return tmp_state


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """
    starting training op async ans save validation sample in global array
    """
    state.thread.start()
    global VAL_QUEUE
    VAL_QUEUE = [Xtest, np.reshape(ytest, ((len(ytest), 1)))]
    print('Filled Validation Queue, Size: %d' % (len(VAL_QUEUE[1])))
    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Apply/fit preprocessor and put data in the queue.
    """
    signal.signal(signal.SIGINT, signal_handler)

    if epoch == 0:
        PREPROCESSOR.fit(X)
        global VAL_QUEUE
        VAL_QUEUE[0] = PREPROCESSOR.apply(VAL_QUEUE[0])

    X = PREPROCESSOR.apply(X)
    state.session.run(state.enqueue, feed_dict={state.x: X, state.y: np.reshape(y, ((len(y), 1)))})

    if epoch % state.param['n_step_queue_epoch'] == 0 and state.param['n_step_queue_epoch'] != 0:
        print("Queue Epoch: %d, Queue Size: %d" %
              (epoch, state.session.run(state.queue.size())))

    return CONTINUE_ENQUEUE


def end_fit(state):
    """
    first close queue and waiting for training to stop, then save state
    """
    print('End of Enqueue')
    signal.signal(signal.SIGINT, signal_handler)
    if not state.param['items_permanent_in_queue']:
        print('Closing Queue.')
        state.session.run(state.queue.close())

    # waiting for training to finish
    global COORD
    COORD.join([state.thread])

    # Save additional evaluation_array in tensorflow summary folder
    pickle.dump(VAL_ARRAY, open(os.path.join(state.param['tensorflow_folder'], 'var.p'), "wb"))

    # save tensorflow training in a pickable state
    state.add_to_collection()
    meta_graph = state.saver.export_meta_graph()
    filename = SAVER_DECIDER.get_savestate()
    print("Using datafile: " + filename)
    with open(filename + str('.data-00000-of-00001'), 'rb') as file1, open(filename + str('.index'), 'rb') as file2:
        data1 = file1.read()
        data2 = file2.read()

    del state
    global VAL_QUEUE
    del VAL_QUEUE

    return [meta_graph, os.path.basename(filename), data1, data2, PREPROCESSOR.export_state()]


def load(obj):
    """
    Load Tensorflow estimator into state
    """
    tf.reset_default_graph()
    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    session = tf.Session(config=config)
    saver = tf.train.import_meta_graph(obj[0])
    with tempfile.TemporaryDirectory() as path:
        with open(os.path.join(path, obj[1] + '.data-00000-of-00001'), 'w+b') as file1, open(
                os.path.join(path, obj[1] + '.index'), 'w+b') as file2:
            file1.write(bytes(obj[2]))
            file2.write(bytes(obj[3]))
        tf.train.update_checkpoint_state(path, obj[1])
        saver.restore(session, os.path.join(path, obj[1]))
    state = State(session=session)
    state.get_from_collection()
    state.preprocessor = preprocessor(obj[4])
    return state


def apply(state, X):
    """
    Apply preprocessing and estimator to passed data.
    """
    X = state.preprocessor.apply(X)
    r = state.session.run(state.activation, feed_dict={state.x: X}).flatten()
    return np.require(r, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def duration(start_time, end_time):
    diff = int(end_time - start_time)
    hours = diff // 3600
    minutes = (diff // 60) - hours * 60
    seconds = diff % 60
    str1 = ''
    for i in [hours, minutes, seconds]:
        if i < 10:
            str1 += '0' + str(i) + ':'
        else:
            str1 += str(i) + ':'
    return str1[:-1]


def delete_all_files_in_folders(folderlist):
    for folder in folderlist:
        for file in os.listdir(folder):
            os.remove(os.path.join(folder, file))

if __name__ == "__main__":
    import basf2_mva
    import json

    training_type = 1

    variables = []
    for i in range(10):
        variables += map(lambda x: x + str(i), ['p_C', 'phi_C', 'cosTheta_C',
                                                "clusterNHits_C", "clusterTiming_C",
                                                "clusterE9E25_C", "clusterReg_C"])

    def get_variables(var_list):
        var = []
        for i in range(5):
            var += map(lambda x: x + "_TP" + str(i), var_list)
        for i in range(5):
            var += map(lambda x: x + "_TM" + str(i), var_list)
        return var

    variables += get_variables(['p', 'phi', 'Theta', 'pErr', 'phiErr', 'ThetaErr',
                                'pValue', 'Kid', 'eid', 'muid', 'prid', 'nCDCHits'])

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("/usr/users/dweyland/ssd/train_BtoKpi.root")
    general_options.m_treename = "variables"
    general_options.m_identifier = "MVAIdentifier"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isContinuumEvent"
    general_options.m_method = "Python"

    events_in_root_file = 742181

    python_options = basf2_mva.PythonOptions()
    python_options.m_framework = "tensorflow"
    python_options.m_steering_file = "tensorflow_multithreaded_feed-forward_net.py"

    # Because MVA use the first fraction of the sample as validation, the data should be shuffled beforehand
    python_options.m_training_fraction = 0.99

    tensorflow_dic = {'batch_size': 5000,
                      'clean_summary_folder': True,
                      'learning_rate': (0.001, 5000, 0.95),
                      'n_step_queue_epoch': 1,
                      'n_step_save_model': 100,
                      'n_step_summaries': 1,
                      'tensorflow_folder': '/usr/users/dweyland/ssd/tensorflow_summary',
                      'visible_gpu': '0',
                      'weight_decay': 0.001}

    # There are 2 different types of training:

    # 1. Directly write all data in queue and train afterwards
    if(training_type == 1):
        tensorflow_dic.update({
            # has to be at least number of events
            'capacity': events_in_root_file + 10,
            # events shouldn't be deleted from queue
            'items_permanent_in_queue': True,
            # number of epochs after training will end
            'max_epoch': 1000000,
            # not importan in this mode
            'min_after_dequeue': 0})

        # describes how many times data will fed into the queue
        # be careful not to forget to calculate with training_fraction
        python_options.m_mini_batch_size = int(700e3)
        python_options.m_nIterations = 1

    # 2. Stream data in queue and train while reading root files
    if (training_type == 2):
        tensorflow_dic.update({
            # Describes how many data the queue will hold.
            # If capacity is reached partial_fit has to wait till number of events in queue is small enough
            'capacity': 3 * events_in_root_file,
            # events should be deleted from queue after used for training
            'items_permanent_in_queue': False,
            # Not necessary in this mode
            'max_epoch': 100000000000000,
            # describes how many items should be at least in queue after dequeue-op
            'min_after_dequeue': 0.5 * events_in_root_file})

        # Duration of training is now controlled, how long data will be streamed into partial fit
        python_options.m_mini_batch_size = int(700e3)
        python_options.m_nIterations = 100

    python_options.m_config = json.dumps(tensorflow_dic)
    basf2_mva.teacher(general_options, python_options)
