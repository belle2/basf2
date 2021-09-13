#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import os
import sys
import time
import numpy
import pickle

import tensorflow as tf


class Layer:
    """
    definition of a layer obj
    """

    def __init__(self, name, tf_activation_str, dim_input, dim_output, p_bias, p_w,
                 operation_seed=None):
        """
        :param name:
        :param tf_activation: string, name of an available tensorflow activations function
        :param dim_input:
        :param dim_output:
        :param p_bias: initial constant
        :param p_w: stddev of uniform distribution to initialize
        :return:
        """
        #: layer parameters
        self.name = name

        tf_activation_dict = {
            'tanh': tf.tanh,
            'sigmoid': tf.sigmoid,
            'relu': tf.nn.relu,
            'leaky_relu': tf.nn.leaky_relu,
        }

        if tf_activation_str not in tf_activation_dict:
            raise ValueError

        #: activation function
        self.tf_activation = tf_activation_dict[tf_activation_str]

        #: random seed
        self.seed = None

        #: layer shape
        self.shape = [dim_input, dim_output]

        #: init parameters for uniform distribution
        self.w = self._init_weight(self.shape, p_w, operation_seed)

        #: init parameters for bias
        self.b = self._init_bias(self.shape[1], p_bias)

        #: input
        self.input = None

        #: output
        self.output = None

        #: check if initialized (input and output are connected)
        self.is_initialized = False

        #: variables to summary
        self._add_all_to_summary()

    def _init_bias(self, width, init_val, name=None):
        """
        define bias variables
        """
        if name is None:
            name = self.name + '_b'
        initial = tf.constant(init_val, shape=[width], name=name)
        return tf.Variable(initial, name=name)

    def _init_weight(self, shape, stddev, operation_seed, name=None):
        """
        define weight variables
        """
        if name is None:
            name = self.name + '_w'
        initial = tf.truncated_normal(shape, stddev=stddev, seed=operation_seed, name=name)
        return tf.Variable(initial, name=name)

    def initialize(self, layer_input):
        """
        initialization, requires layer input
        """
        if self.is_initialized:
            raise RuntimeError('Layer %s is already initialized.' % self.name)

        self.input = layer_input
        name = self.name + '_output'
        with tf.name_scope('output'):
            self.output = self.tf_activation(tf.matmul(layer_input, self.w) + self.b, name)
            tf.add_to_collection('output', self.output)

        # set initialized status
        self.is_initialized = True

    def _add_var_to_summary(self, var):
        """
        add variables of this layer to tensorboard
        :param var: tensorflow variable
        :return:
        """
        with tf.name_scope('summaries'):
            mean = tf.reduce_mean(var)
            tf.summary.scalar('mean', mean)

            with tf.name_scope('stddev'):
                stddev = tf.sqrt(tf.reduce_mean(tf.square(var - mean)))

            tf.summary.scalar('stddev', stddev)
            tf.summary.scalar('max', tf.reduce_max(var))
            tf.summary.scalar('min', tf.reduce_min(var))

            tf.summary.histogram('histogram', var)

    def _add_all_to_summary(self):
        """
        adds tuneable parameters to summary
        """
        with tf.name_scope(self.name):
            with tf.name_scope('weights'):
                self._add_var_to_summary(self.w)
            with tf.name_scope('biases'):
                self._add_var_to_summary(self.b)


class MultilayerPerceptron:
    """
    class combine layer obj
    """

    def __init__(self, layers):
        """
        initialization
        """

        #: layer objs
        self.layers = layers

        #: input
        self.input = None

        #: output
        self.output = None

        #: weights
        self.w = None

        #: biases
        self.b = None

        #: set all mlp parameters
        self.is_initialized = False

    @classmethod
    def from_list(cls, layers):
        """
        define layers from list
        """
        layer_obj = []
        for layer in layers:
            layer_obj.append(Layer(*layer))

        mlp = cls(layer_obj)
        return mlp

    def _set_weights_and_biases(self):
        """
        collect tunable parameters
        """
        self.w = []
        self.b = []
        for layer in self.layers:
            self.w.append(layer.w)
            self.b.append(layer.b)

    def _connect_layers(self, layer_input):
        """
        connect layers
        """
        for layer in self.layers:
            layer.initialize(layer_input)
            layer_input = layer.output

    def initialize(self, layer_input):
        """
        initialize
        """
        if self.is_initialized:
            raise RuntimeError

        self.is_initialized = True
        # check shape
        for _idx in range(len(self.layers) - 1):
            assert self.layers[_idx].shape[1] == self.layers[_idx + 1].shape[0]

        self._connect_layers(layer_input)
        self._set_weights_and_biases()

        self.input = self.layers[0].input
        self.output = self.layers[-1].output


class DefaultModel:
    """
    define the default model
    """

    def __init__(self, mlp, mom_init=.9, mom_max=.99, mom_epochs=200, lr_init=.05, lr_min=1e-6,
                 lr_dec_rate=.976,
                 stop_epochs=10, min_epochs=200, max_epochs=1000, wd_coeffs=None, change_optimizer=None,
                 staircase=True, smooth_cross_entropy=False):
        """
        initialization function
        """

        if wd_coeffs is not None:
            assert len(wd_coeffs) == len(mlp.layers)

        #: weight decay coefficients
        self.wd_coeffs = wd_coeffs

        #: network model
        self.mlp = mlp

        #: output
        self.output = None

        with tf.name_scope('global_step'):
            #: global step
            self.global_step = tf.Variable(0, trainable=False)

        # optimizer params
        #: initial momentum
        self.c_mom_init = mom_init
        #: maximum momentum
        self.c_mom_max = mom_max
        #: momentum epochs
        self.c_mom_epochs = mom_epochs

        #: initial learning rate
        self.c_lr_init = lr_init
        #: minimal learning rate
        self.c_lr_min = lr_min
        #: learning rate decay factor
        self.c_lr_dec_rate = lr_dec_rate

        #: number of epochs until stopping
        self.c_stop_epochs = stop_epochs

        #: use staircase
        self.c_staircase = staircase

        #: batches per epoch unknown. needs to be set with initialize
        self.batches_per_epoch = None

        #: define multiple optimizers
        self.optimizers = []
        # list with epochs in which optimizers will be changed, if None is given, only the default optimizer will be
        #: used opimizers
        self.optimizer_change_epochs = change_optimizer
        if change_optimizer is not None:
            self.optimizer_change_epochs.insert(0, 0)
            self.optimizer_change_epochs.append(sys.maxsize)

        #: params for monitoring
        self.monitoring_params = None
        #: params for labeling
        self.monitoring_labels = None

        #: further monitoring variables
        self.mon_dict = dict()

        #: variable space must be known
        self.x = None
        #: targets
        self.y_ = None
        #: weights
        self.weights = None

        #: other variables
        self.max_epochs = max_epochs

        # termination criterion
        #: min epochs
        self.min_epochs = min_epochs

        #: termination criterion
        self.termination_criterion = None
        #: recent params
        self.recent_params = []
        #: the best value will be set a default start value, then updated with the termination criterion
        self.best_value = numpy.inf

        #: step countdown
        self.step_countdown = self.c_stop_epochs

        #: True for a small epsilon addition, false for a clipped network output
        self.smooth_cross_entropy = smooth_cross_entropy

        #: check if initialized
        self.is_initialized = False

    def _set_optimizer(self):
        """
        set optimizer
        """

        t_learning_rate = tf.train.exponential_decay(self.c_lr_init, self.global_step, self.batches_per_epoch,
                                                     self.c_lr_dec_rate, staircase=self.c_staircase)

        t_lr_min = tf.Variable(self.c_lr_min, trainable=False)

        with tf.name_scope('learning_rate'):
            t_limited_lr = tf.maximum(t_learning_rate, t_lr_min)
            tf.summary.scalar('learning rate', t_limited_lr)

        # linear increasing momentum
        c_mom_dec_rate = (self.c_mom_max - self.c_mom_init) / self.c_mom_epochs

        t_mom_dec_rate = tf.Variable(c_mom_dec_rate, trainable=False, dtype=tf.float32)

        t_momentum = tf.Variable(self.c_mom_init, trainable=False, dtype=tf.float32)

        t_mom_max = tf.Variable(self.c_mom_max, trainable=False, dtype=tf.float32)

        t_batches_per_epoch = tf.Variable(self.batches_per_epoch, trainable=False, dtype=tf.float32)

        with tf.name_scope('momentum'):
            # tf.floor -- 'staircase = true'
            global_step = tf.cast(self.global_step, tf.float32)
            if self.c_staircase:
                t_limited_mom = tf.minimum(t_momentum + t_mom_dec_rate *
                                           tf.floor(global_step / t_batches_per_epoch), t_mom_max)
            else:
                t_limited_mom = tf.minimum(t_momentum + t_mom_dec_rate *
                                           (global_step / t_batches_per_epoch), t_mom_max)
            tf.summary.scalar('momentum', t_limited_mom)

        self.mon_dict['learning_rate'] = t_limited_lr
        self.mon_dict['momentum'] = t_limited_mom

        with tf.name_scope('optimizer'):
            self.optimizers.append(tf.train.MomentumOptimizer(t_limited_lr, t_limited_mom))
            self.optimizers.append(tf.train.GradientDescentOptimizer(t_limited_lr))

    def _set_loss(self):
        """
        set loss
        """

        _y = self.mlp.output
        _y_ = self.y_

        epsilon = 1e-10
        t_epsilon = tf.constant(epsilon)

        # sum over batch
        with tf.name_scope('cross_entropy'):
            if self.smooth_cross_entropy:
                cross_entropy = -tf.reduce_mean(tf.reduce_sum((_y_ * tf.log(_y + t_epsilon) + (1 - _y_) *
                                                               tf.log(1 - _y + t_epsilon)), 1))
            else:
                cross_entropy = -tf.reduce_mean(tf.reduce_sum((_y_ * tf.log(tf.clip_by_value(_y, epsilon, 1))) +
                                                              ((1 - _y_) * tf.log(tf.clip_by_value((1 - _y),
                                                                                                   epsilon, 1))), 1))
            tf.summary.scalar('cross entropy', cross_entropy)

        loss = cross_entropy

        if self.wd_coeffs:
            wd_coeffs = self.wd_coeffs
            weights = self.mlp.w

            wd = [tf.constant(coeff) * tf.nn.l2_loss(w) for coeff, w in zip(wd_coeffs, weights)]
            loss += sum(wd)

        with tf.name_scope('loss_function'):
            #: cross entropy with weight decay
            self.loss = loss
            tf.summary.scalar('loss', loss)

    def _set_monitoring_params(self):
        """
        sets monitoring params
        """

        _y = self.mlp.output
        _y_ = self.y_

        epsilon = 1e-10
        t_epsilon = tf.constant(epsilon)

        with tf.name_scope('mean_cross_entropy'):
            if self.smooth_cross_entropy:
                mean_cross_entropy = -tf.reduce_mean(tf.reduce_sum((_y_ * tf.log(_y + t_epsilon) + (1 - _y_) *
                                                                    tf.log(1 - _y + t_epsilon)), 1))
            else:
                mean_cross_entropy = -tf.reduce_mean(tf.reduce_sum((_y_ * tf.log(tf.clip_by_value(_y, epsilon, 1))) +
                                                                   ((1 - _y_) * tf.log(tf.clip_by_value((1 - _y),
                                                                                                        epsilon, 1))), 1))
            tf.summary.scalar('mean cross entropy', mean_cross_entropy)
        loss = self.loss

        self.monitoring_labels = ['mean_cross_entropy', 'loss']
        self.monitoring_params = [mean_cross_entropy, loss]

    def _default_termination_criterion(self, monitoring_params, epoch, label_name='mean_cross_entropy',
                                       prop_dec=1e-5):
        """
        :param monitoring_params:
        :param epoch:
        :param label_name:
        :param steps:
        :param prop_dec:
        :return:
        """

        loss_idx = self.monitoring_labels.index(label_name)
        mon_param = monitoring_params[loss_idx]

        if epoch < self.min_epochs:
            return False

        if mon_param < self.best_value * (1. - prop_dec):
            self.step_countdown = self.c_stop_epochs
            self.best_value = mon_param
        else:
            self.step_countdown -= 1

        if self.step_countdown > 0:
            return False

        return True

    def get_minimizer(self, epoch=0):
        """
        get minimizer
        """

        if self.optimizer_change_epochs is None:
            return self.optimizers[0].minimize(self.loss, self.global_step)

        if len(self.optimizer_change_epochs) > len(self.optimizers) + 1:
            raise RuntimeError

        # switch optimizer for given epoch
        for i in range(1, len(self.optimizer_change_epochs)):
            if self.optimizer_change_epochs[i - 1] <= epoch < self.optimizer_change_epochs[i]:
                return self.optimizers[i - 1].minimize(self.loss, self.global_step)

    # needs to be called since parameters depends on number of batches
    def initialize(self, data_set, input_placeholders=None):
        """
        initialized
        """
        if self.is_initialized:
            raise RuntimeError
        self.is_initialized = True

        self.batches_per_epoch = data_set.batches
        # placeholders
        if input_placeholders is None:
            with tf.name_scope('input'):
                self.x = tf.placeholder(tf.float32, shape=[None, data_set.feature_number], name='x-input')
                self.y_ = tf.placeholder(tf.float32, shape=[None, 1], name="y-input")
                # TODO: implement weights
                self.weights = tf.placeholder(tf.float32, shape=[None, 1], name="weight-input")
        else:
            self.x = input_placeholders[0]
            self.y_ = input_placeholders[1]

        # layer input, calls cascade initialization
        self.mlp.initialize(self.x)

        self._set_optimizer()
        self._set_loss()
        self._set_monitoring_params()

        # termination criterion
        self.termination_criterion = self._default_termination_criterion


class Trainer:
    """
    handling the training of the network model
    """

    def __init__(self, model, data_set, sess, log_dir=None, save_name=None, monitoring_size=100000,
                 input_placeholders=None):
        """
        class to train a predefined model
        :param model: DefaultModel obj
        :param data_set: TFData obj
        :param sess: tensorflow.Session obj
        :param log_dir: str, directory name of tensorboard logging
        :param save_name: str, path and name for saving the weightfiles
        :param monitoring_size: int, number of events of training fraction used for monitoring
        :param input_placeholders: list of tf.placeholders, [features, targets]
        """

        #: time
        self._time = time.time()
        #: model
        self.model = model
        #: data set
        self.data_set = data_set
        #: monitoring size
        self.monitoring_size = monitoring_size

        #: tf.session
        self.sess = sess

        #: log directory
        self.log_dir = log_dir

        if input_placeholders is None:
            with tf.name_scope('input'):
                #: input placeholder features
                self.x = tf.placeholder(tf.float32, shape=[None, data_set.feature_number], name='x-input')

                #: input placeholder targets
                self.y_ = tf.placeholder(tf.float32, shape=[None, 1], name="y-input")
        else:
            #: input placeholder features
            self.x = input_placeholders[0]
            #: input placeholder targets
            self.y_ = input_placeholders[1]

        self.model.initialize(data_set, input_placeholders=[self.x, self.y_])

        #: monitoring params for early stopping criterion, loss function, etc
        self.monitoring_params = self.model.monitoring_params
        #: termination criterion
        self.termination_criterion = self.model.termination_criterion

        #: global_training_parameters
        self.max_epochs = self.model.max_epochs

        #: current epoch
        self.current_epoch = 0

        #: optimizer
        self.minimizer = self.model.get_minimizer()

        #: train_log_dict
        self.train_log_dict = {}
        for label in self.model.monitoring_labels:
            self.train_log_dict['train_' + label] = []
            self.train_log_dict['valid_' + label] = []

        for label in self.model.mon_dict.keys():
            self.train_log_dict[label] = []

        self._add_to_basf2_collections()

        if log_dir is not None:
            self._prepare_tensorboard(log_dir)

        #: saver
        self.saver = tf.train.Saver()

        init_op = tf.global_variables_initializer()

        self.sess.run(init_op)

        if save_name is None:
            time_str = time.strftime("%Y%m%d-%H%M%S")
            #: save name
            self.save_name = os.path.join(os.getcwd(), '_'.join([self.data_set.selection, time_str,
                                                                 'model.ckpt']))

        else:
            #: save name
            self.save_name = save_name

        self._prepare_monitoring()

    def _prepare_monitoring(self):
        """
        checking dataset sizes for evaluation
        """
        #: train_monitor
        self.train_monitor = -1
        #: valid monitor
        self.valid_monitor = -1
        if self.data_set.train_events > self.monitoring_size:
            self.train_monitor = self.monitoring_size

        if self.data_set.valid_events > self.monitoring_size:
            self.valid_monitor = self.monitoring_size

    def _prepare_tensorboard(self, log_dir):
        """
        prepare tensorboard
        """
        log_dir_train = os.path.join(log_dir, 'train')
        log_dir_test = os.path.join(log_dir, 'test')

        #: train writer
        self.train_writer = tf.summary.FileWriter(log_dir_train, self.sess.graph)
        #: test writer
        self.test_writer = tf.summary.FileWriter(log_dir_test, self.sess.graph)

        #: summary
        self.merged_summary = tf.summary.merge_all()

    def _add_to_basf2_collections(self):
        """
        add to basf2 collection
        """
        tf.add_to_collection('x', self.x)
        tf.add_to_collection('y', self.y_)
        tf.add_to_collection('activation', self.model.mlp.output)
        tf.add_to_collection('cost', self.model.loss)
        tf.add_to_collection('optimizer', self.minimizer)

    def _save_best_state(self, monitoring_params, label_name='mean_cross_entropy'):
        """
        save model only if a global minimum is reached on validation set
        :return:
        """
        # current last state
        self.saver.save(self.sess, self.save_name.replace('.ckpt', '_current.ckpt'))

        # check for a not set best value
        if self.model.best_value == numpy.inf:
            return

        loss_idx = self.model.monitoring_labels.index(label_name)
        mon_param = monitoring_params[loss_idx]

        if mon_param < self.model.best_value:
            self.train_log_dict['best_epoch'] = self.current_epoch
            self.saver.save(self.sess, self.save_name)

    def _closing_ops(self):
        """
        closing ops
        """
        if self.log_dir is not None:
            self.train_writer.close()
            self.test_writer.close()
            with open(os.path.join(self.log_dir, 'training_params.pkl'), 'wb') as f:
                pickle.dump(self.train_log_dict, f)

        # FIXME: raises an exception when calling session._del -> session.TF_DeleteStatus() !
        # FIXME: not closing a session can cause memory leaks!
        # self.sess.close()
        # del self.sess

    def _train_epoch(self, current_epoch):
        """
        train epoch
        """

        # set minimizer for this epoch
        self.minimizer = self.model.get_minimizer(current_epoch)

        batch_iter = self.data_set.batch_iterator()

        for i in range(self.data_set.batches):
            batch = next(batch_iter)
            feed = {self.x: batch[0], self.y_: batch[1]}
            self.sess.run(self.minimizer, feed_dict=feed)
            if i % int(.2 * self.data_set.batches) == 0:
                print('Epoch status: %1.2f' % (i / self.data_set.batches))

        train_mon_dict = {self.x: self.data_set.train_x[:self.train_monitor],
                          self.y_: self.data_set.train_y[:self.train_monitor]}

        valid_dict = {self.x: self.data_set.valid_x[:self.valid_monitor],
                      self.y_: self.data_set.valid_y[:self.valid_monitor]}

        # depending on the fact, if the log should be monitored or not
        if self.log_dir is not None:
            # todo raise exception if not empty

            summary = self.sess.run(self.merged_summary, feed_dict=train_mon_dict)
            self.train_writer.add_summary(summary, current_epoch)

            #: epoch parameters
            self.epoch_parameters = self.sess.run(self.monitoring_params + [self.merged_summary],
                                                  feed_dict=valid_dict)
            epoch_parameters_train = self.sess.run(
                self.monitoring_params + [self.merged_summary], feed_dict=train_mon_dict)

            # merged summary will be at last position
            self.test_writer.add_summary(self.epoch_parameters.pop(-1), current_epoch)
        else:
            self.epoch_parameters = self.sess.run(self.monitoring_params,
                                                  feed_dict=valid_dict)
            epoch_parameters_train = self.sess.run(self.monitoring_params, feed_dict=train_mon_dict)

        model_mon_params = []
        for key, entry in self.model.mon_dict.items():
            model_mon_params.append((key, self.sess.run(entry)))

        print('Current Epoch: %d' % self.current_epoch)
        for label, param in zip(self.model.monitoring_labels, self.epoch_parameters):
            self.train_log_dict['valid_' + label].append(param)
            print(f"valid: {label}: {param:1.5f}")
        for label, param in zip(self.model.monitoring_labels, epoch_parameters_train):
            print(f"train: {label}: {param:1.5f}")
            self.train_log_dict['train_' + label].append(param)

        for val in model_mon_params:
            print("%s: %f" % val)
            self.train_log_dict[val[0]].append(val[1])

        print('Epoch training time: %.1f' % (time.time() - self._time))
        self._time = time.time()
        print('\n')
        self.current_epoch += 1

    def train_model(self):
        """
        train model
        """
        for epoch in range(self.max_epochs):
            self._train_epoch(epoch)

            self._save_best_state(self.epoch_parameters)

            if self.termination_criterion(self.epoch_parameters, epoch):
                break

        self._closing_ops()
