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
import numpy as np

import tensorflow as tf
from tqdm import trange


class Layer(tf.Module):
    """
    definition of a layer obj
    """

    def __init__(self, name, tf_activation_str, dim_input, dim_output, p_bias, p_w,
                 random_seed=None):
        """
        :param name: name of the layer.
        :param tf_activation: string, name of an available tensorflow activations function
        :param dim_input: dimension of the input
        :param dim_output: dimension of the output
        :param p_bias: initial bias
        :param p_w: stddev of uniform distribution to initialize
        :param random_seed: random seed used in initialising the weights
        :return: None
        """
        #: layer parameters
        super().__init__(name=name)

        tf_activation_dict = {
            'tanh': tf.nn.tanh,
            'sigmoid': tf.nn.sigmoid,
            'relu': tf.nn.relu,
            'leaky_relu': tf.nn.leaky_relu,
        }

        if tf_activation_str not in tf_activation_dict:
            raise ValueError

        #: activation function
        self.tf_activation = tf_activation_dict[tf_activation_str]

        #: layer shape
        self.shape = [dim_input, dim_output]

        #: init parameters for uniform distribution
        self.w = self._init_weight(self.shape, p_w, random_seed)

        #: init parameters for bias
        self.b = self._init_bias(self.shape[1], p_bias)

        #: input
        self.input = None

        #: output
        self.output = None

    def _init_bias(self, width, init_val, name=None):
        """
        define bias variables
        """
        if name is None:
            name = self.name + '_b'
        initial = tf.constant(init_val, shape=[width], name=name)
        return tf.Variable(initial, name=name, trainable=True)

    def _init_weight(self, shape, stddev, operation_seed, name=None):
        """
        define weight variables
        """
        if name is None:
            name = self.name + '_w'
        initial = tf.random.truncated_normal(shape, stddev=stddev, seed=operation_seed, name=name)
        return tf.Variable(initial, name=name, trainable=True)

    @tf.function
    def __call__(self, x):
        """
        evaluate the layer
        """
        return self.tf_activation(tf.matmul(x, self.w) + self.b)

    def variable_to_summary(self, var, step, writer):
        """
        Passes information about each variable to the summary writer.
        """
        with writer.as_default():
            mean = tf.reduce_mean(var)
            stddev = tf.sqrt(tf.reduce_mean(tf.square(var - mean)))
            tf.summary.scalar(f'{var.name}_mean', mean, step=step)
            tf.summary.scalar(f'{var.name}_stddev', stddev, step=step)
            tf.summary.scalar(f'{var.name}_max', tf.reduce_max(var), step=step)
            tf.summary.scalar(f'{var.name}_min', tf.reduce_min(var), step=step)
            tf.summary.histogram(f'{var.name}_histogram', var, step=step)
            writer.flush()
        return

    def all_to_summary(self, step, writer):
        """
        Passes all layer variables to the tf.summary writer.
        """
        self.variable_to_summary(self.w, step=step, writer=writer)
        self.variable_to_summary(self.b, step=step, writer=writer)
        return


class MultilayerPerceptron(tf.Module):
    """
    multilayer perceptron class.
    """

    def __init__(self, layers, name='mlp'):
        """
        initialization
        """
        super().__init__(name=name)

        #: layer objs
        self.layers = layers

        #: weights
        self.w = None

        #: biases
        self.b = None

        #: collect all mlp parameters
        self.is_initialized = False
        self.initialize()

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

    def _collect_weights_and_biases(self):
        """
        collect tunable parameters
        """
        self.w = []
        self.b = []
        for layer in self.layers:
            self.w.append(layer.w)
            self.b.append(layer.b)

    def initialize(self):
        """
        initialize. Checks that the layer dimensions align.
        """
        if self.is_initialized:
            raise RuntimeError

        # check shape
        for _idx in range(len(self.layers) - 1):
            assert self.layers[_idx].shape[1] == self.layers[_idx + 1].shape[0]

        self._collect_weights_and_biases()  # TODO - remove?

        self.is_initialized = True
        return

    @tf.function
    def __call__(self, x):
        """
        Run the events through all the layers
        """
        for layer in self.layers:
            x = layer(x)
        return x

    def variables_to_writer(self, step, writer):
        """
        passes all the MLP variables to the tf.summary writer
        """
        for layer in self.layers:
            layer.all_to_summary(step, writer)
        return


class DefaultModel(tf.Module):
    """
    define the default model
    """

    def __init__(self, mlp,
                 mom_init=.9,
                 mom_max=.99,
                 mom_epochs=200,
                 lr_init=.05,
                 lr_min=1e-6,
                 lr_dec_rate=.976,
                 stop_epochs=10,
                 min_epochs=200,
                 max_epochs=1000,
                 wd_coeffs=None,
                 change_optimizer=None,
                 staircase=True,
                 smooth_cross_entropy=False):
        """
        initialization function
        :param mlp: network model.
        :param mom_init: initial momentum
        :param mom_max: maximum momentum
        :param mom_epochs: momentum epochs
        :param lr_init: initial learning rate
        :param lr_min: minimum learning rate
        :param lr_dec_rate: learning rate decay factor
        :param stop_epochs: number of epochs without improvement required for early termination
        :param min_epochs: minimum number of epochs for training
        :param max_epochs: maximum number of epochs for traning
        :param wd_coeffs: weight decay coefficients. If not None must have one per mlp layer.
        :param change_optimizer:
        :param staircaise:
        "param smooth_cross_entropy:
        """

        #: mlp net
        self.mlp = mlp

        if wd_coeffs is not None:
            assert len(wd_coeffs) == len(mlp.layers)

        #: weight decay coefficients
        self.wd_coeffs = wd_coeffs

        #: global step
        self.global_step = tf.Variable(0, trainable=False, name='global_step', dtype=tf.int64)

        # --optimizer params--
        #: initial momentum
        self.c_mom_init = tf.constant(mom_init, dtype=tf.float32)

        #: maximum momentum
        self.c_mom_max = tf.constant(mom_max, dtype=tf.float32)

        #: momentum epochs
        self.c_mom_epochs = tf.constant(mom_epochs, dtype=tf.float32)

        #: momentum decay rate
        self.c_mom_dec_rate = (self.c_mom_max - self.c_mom_init) / tf.cast(self.c_mom_epochs, tf.float32)

        #: initial learning rate
        self.c_lr_init = tf.constant(lr_init, dtype=tf.float32)

        #: minimum learning rate
        self.c_lr_min = tf.constant(lr_min, dtype=tf.float32)

        #: learning rate decay rate
        self.c_lr_dec_rate = tf.constant(lr_dec_rate, dtype=tf.float32)

        #: number of epochs without improvement for early termination
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

        # termination criterion
        #: min epochs
        self.min_epochs = min_epochs

        #: max epochs
        self.max_epochs = max_epochs

        #: termination criterion
        self.termination_criterion = None

        #: recent params
        self.recent_params = []

        #: the best value will be set a default start value, then updated with the termination criterion
        self.best_value = np.inf

        #: step countdown
        self.step_countdown = self.c_stop_epochs

        #: True for a small epsilon addition, false for a clipped network output
        self.smooth_cross_entropy = smooth_cross_entropy

        #: check if initialized
        self.is_initialized = False

        return

    # needs to be called since parameters depends on number of batches

    def initialize(self, data_set):
        """
        Finalises intitialization based of data_set specific information (number of batches per epoch)
        """
        if self.is_initialized:
            raise RuntimeError

        self.batches_per_epoch = data_set.batches

        # check layer dimensions align properly
        if not self.mlp.is_initialized:
            self.mlp.initialize()

        self._set_optimizer()
        self.termination_criterion = self._default_termination_criterion

        self.is_initialized = True
        return

    @tf.function
    def __call__(self, x):
        """
        Call the mlp
        """
        return self.mlp(x)

    def _default_termination_criterion(self,
                                       monitoring_param,
                                       epoch,
                                       prop_dec=1e-5):
        """
        early stopping criterion

        :param monitoring_param: the parameter to monitor for early termination
        :param epoch: the current epoch
        :param prop_dec:
        :return:
        """
        if epoch < self.min_epochs:
            return False

        if monitoring_param < self.best_value * (1. - prop_dec):
            self.step_countdown = self.c_stop_epochs
            self.best_value = monitoring_param
        else:
            self.step_countdown -= 1

        if self.step_countdown > 0:
            return False
        return True

    def _get_learning_rate(self):
        """
        Returns the learning rate at the current global step.
        """
        p = tf.cast(self.global_step, tf.float32) / tf.cast(self.batches_per_epoch, tf.float32)
        if self.c_staircase:
            p = tf.floor(p)
        return tf.maximum(tf.multiply(self.c_lr_init, tf.pow(self.c_lr_dec_rate, p)), self.c_lr_min)

    def _get_momentum(self):
        """
        returns the momentum at the current global step.
        """
        t_batches_per_epoch = tf.constant(self.batches_per_epoch, dtype=tf.float32)
        global_step = tf.cast(self.global_step, tf.float32)

        if self.c_staircase:
            t_limited_mom = tf.minimum(self.c_mom_init + self.c_mom_dec_rate *
                                       tf.floor(global_step / t_batches_per_epoch), self.c_mom_max)
        else:
            t_limited_mom = tf.minimum(self.c_mom_init + self.c_mom_dec_rate *
                                       (global_step / t_batches_per_epoch), self.c_mom_max)
        return t_limited_mom

    def _set_optimizer(self):
        """
        set optimizers
        """
        self.optimizers.append(tf.optimizers.SGD(learning_rate=self._get_learning_rate,
                                                 momentum=self._get_momentum))
        self.optimizers.append(tf.optimizers.SGD(learning_rate=self._get_learning_rate))
        return

    def get_optimizer(self, epoch=0):
        """
        get the optimizer. If multiple optimizers are booked gets the one appropriate for the epoch.

        :param epoch: current epoch.
        """

        if self.optimizer_change_epochs is None:
            return self.optimizers[0]

        if len(self.optimizer_change_epochs) > len(self.optimizers) + 1:
            raise RuntimeError

        # switch optimizer for given epoch
        for i in range(1, len(self.optimizer_change_epochs)):
            if self.optimizer_change_epochs[i - 1] <= epoch < self.optimizer_change_epochs[i]:
                return self.optimizers[i - 1]

    def loss(self, predict_y, true_y):
        """
        calculate the loss

        :param predict_y: predicted labels
        :param true_y: true labels
        """

        epsilon = 1e-10
        t_epsilon = tf.constant(epsilon)

        # sum over batch
        with tf.name_scope('cross_entropy'):
            if self.smooth_cross_entropy:
                cross_entropy = -tf.reduce_mean(tf.reduce_sum((true_y * tf.math.log(predict_y + t_epsilon) + (1 - true_y) *
                                                               tf.math.log(1 - predict_y + t_epsilon)), 1))
            else:
                cross_entropy = -tf.reduce_mean(tf.reduce_sum((true_y * tf.math.log(tf.clip_by_value(predict_y, epsilon, 1))) +
                                                              ((1 - true_y) * tf.math.log(tf.clip_by_value((1 - predict_y),
                                                                                                           epsilon, 1))), 1))
        loss = cross_entropy

        if self.wd_coeffs:
            wd_coeffs = self.wd_coeffs
            weights = self.mlp.w

            wd = [tf.constant(coeff) * tf.nn.l2_loss(w) for coeff, w in zip(wd_coeffs, weights)]
            loss += sum(wd)

        return loss, cross_entropy


class Trainer:
    """
    handling the training of the network model
    """

    def __init__(self,
                 model,
                 data_set,
                 log_dir=None,
                 save_name=None,
                 monitoring_size=10000):
        """
        class to train a predefined model
        :param model: DefaultModel obj
        :param data_set: TFData obj
        :param log_dir: str, directory name of tensorboard logging
        :param save_name: str, path and name for saving the weightfiles
        :param monitoring_size: int, number of events of training fraction used for monitoring
        """

        #: current time
        self._time = time.time()

        #: model
        self.model = model

        #: dataset
        self.data_set = data_set
        self.model.initialize(data_set)

        #: monitoring size
        self.monitoring_size = monitoring_size

        #: log dir
        self.log_dir = log_dir

        #: termination criterion
        self.termination_criterion = self.model.termination_criterion

        #: initialise current epoch
        self.current_epoch = 0

        #: initialise best epoch
        self.best_epoch = -np.inf

        if log_dir is not None:
            self._prepare_tensorboard(log_dir)

        if save_name is None:
            time_str = time.strftime("%Y%m%d-%H%M%S")
            save_name = os.path.join(os.getcwd(), '_'.join([time_str, 'model']))

        #: set the path and name for saving the weightfiles
        self.save_name = save_name

        self._prepare_monitoring()
        return

    def _prepare_monitoring(self):
        """
        checking dataset sizes for evaluation. These samples are used after each epoch to collect
        summary statistics and test early stopping criteria.
        """
        #: train_monitor
        self.train_monitor = -1
        #: test monitor
        self.valid_monitor = -1
        if self.data_set.train_events > self.monitoring_size:
            self.train_monitor = self.monitoring_size

        if self.data_set.valid_events > self.monitoring_size:
            self.valid_monitor = self.monitoring_size
        return

    def _prepare_tensorboard(self, log_dir):
        """
        prepare tensorboard
        """
        log_dir_train = os.path.join(log_dir, 'train')
        log_dir_valid = os.path.join(log_dir, 'valid')

        #: tf.summary.writer for training
        self.train_writer = tf.summary.create_file_writer(log_dir_train)

        #: tf.summary.writer for validation
        self.valid_writer = tf.summary.create_file_writer(log_dir_valid)
        return

    def _train_epoch(self, current_epoch):
        """
        train epoch
        """
        #: set optimizer for this epoch
        self.optimizer = self.model.get_optimizer(current_epoch)

        batch_iter = self.data_set.batch_iterator()

        t_range = trange(self.data_set.batches)
        t_range.set_description(f'Epoch {current_epoch:4d}')
        for i in t_range:

            batch = next(batch_iter)

            batch_x = batch[0]
            batch_y = batch[1]

            with tf.GradientTape() as tape:
                loss, _ = self.model.loss(self.model(batch_x), batch_y)
                grads = tape.gradient(loss, self.model.trainable_variables)

            self.optimizer.apply_gradients(zip(grads, self.model.trainable_variables))

            # write the learning rate and momentum to the tensorbord log
            if self.log_dir is not None:
                with self.train_writer.as_default():
                    tf.summary.scalar('learning_rate', self.model._get_learning_rate(), step=self.model.global_step)
                    tf.summary.scalar('momentum', self.model._get_momentum(), step=self.model.global_step)
                    self.train_writer.flush()

            self.model.global_step.assign_add(1)

        train_x = self.data_set.train_x[:self.train_monitor]
        train_y = self.data_set.train_y[:self.train_monitor]

        valid_x = self.data_set.valid_x[:self.valid_monitor]
        valid_y = self.data_set.valid_y[:self.valid_monitor]

        # run the training and validation samples to collect statistics
        train_loss, train_cross_entropy = self.model.loss(self.model(train_x), train_y)
        valid_loss, valid_cross_entropy = self.model.loss(self.model(valid_x), valid_y)

        # if we have a log_dir set write extra summary information
        if self.log_dir is not None:
            with self.train_writer.as_default():
                tf.summary.scalar('loss', train_loss, step=current_epoch)
                tf.summary.scalar('cross_entropy', train_cross_entropy, step=current_epoch)

                # this is now at the end of each epoch
                tf.summary.scalar('epoch_learning_rate', self.model._get_learning_rate(), step=current_epoch)
                tf.summary.scalar('epoch_momentum', self.model._get_momentum(), step=current_epoch)
                self.train_writer.flush()

            # write all the model parameters to the summary file too
            self.model.mlp.variables_to_writer(current_epoch, self.train_writer)

            with self.valid_writer.as_default():
                tf.summary.scalar('loss', valid_loss, step=current_epoch)
                tf.summary.scalar('cross_entropy', valid_cross_entropy, step=current_epoch)
                tf.summary.scalar('best_epoch', self.best_epoch, step=current_epoch)
                self.valid_writer.flush()

        # update time
        self._time = time.time()
        self.current_epoch += 1

        return valid_cross_entropy

    def _save_best_state(self, cross_entropy):
        """
        save model as a checkpoint only if a global minimum is reached on validation sample
        :return:
        """
        # current state - do we need this?
        checkpoint = tf.train.Checkpoint(self.model)
        checkpoint.save(self.save_name.replace('model', 'model_current'))

        # check for a not set best value
        if self.model.best_value == np.inf:
            return

        if cross_entropy < self.model.best_value:
            self.best_epoch = self.current_epoch
            checkpoint.save(self.save_name)
        return

    def _closing_ops(self):
        """
        closing operations
        """
        if self.log_dir is not None:
            self.train_writer.close()
            self.valid_writer.close()
        return

    def train_model(self):
        """
        train model
        """
        for epoch in range(self.model.max_epochs):
            valid_cross_entropy = self._train_epoch(epoch)

            self._save_best_state(valid_cross_entropy)

            if self.termination_criterion(valid_cross_entropy, epoch):
                break

        self._closing_ops()
        return
