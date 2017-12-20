#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This describes the keras model for the training in B2A712.
# To understand the code, the keras documentation is a good starting point.
# Also have a look at the contrib keras interface in:
# mva/scripts/basf2_mva_python_interface/contrib_keras.py
#
# Additional examples with keras can be found in the folder:
# mva/examples/keras
#
# Also have a look at https://confluence.desy.de/display/BI/Continuum+Suppression+Framework
# The techniques are described in more detail in http://ekp-invenio.physik.uni-karlsruhe.de/record/48934
#
# Contributors: D. Weyland (November 2017), P. Goldenzweig (October 2016)
#
################################################################################

import tensorflow as tf
import tensorflow.contrib.keras as keras

from keras.layers import Input, Dense, Concatenate, Dropout, Lambda, GlobalAveragePooling1D, Reshape
from keras.models import Model, load_model
from keras.optimizers import adam
from keras.losses import binary_crossentropy, sparse_categorical_crossentropy
from keras.activations import sigmoid, tanh, softmax
from keras.callbacks import Callback, EarlyStopping
from sklearn.metrics import roc_auc_score
import numpy as np
import os

from basf2_mva_python_interface.contrib_keras import State
from basf2_mva_extensions.keras_relational import Relations, EnhancedRelations
from basf2_mva_extensions.preprocessing import fast_equal_frequency_binning


def slice(input, begin, end):
    """
    Simple function for slicing feature in tensors.
    """
    return input[:, begin:end]


class batch_generator():
    """
    Class to create batches for training the Adversary Network.
    See mva/examples/keras/adversary_network.py for details.
    """

    def __init__(self, X, Y, Z):
        """
        Init the class
        :param X: Input Features
        :param Y: Label Data
        :param Z: Spectaters/Qunatity to be uncorrelated to
        """
        #: Input Features
        self.X = X
        #: Label Data
        self.Y = Y
        #: Spectators
        self.Z = Z
        #: Number of events
        self.len = len(Y)
        #: Index array, which will be shuffled
        self.index_array = np.arange(self.len)
        np.random.shuffle(self.index_array)
        #: Pointer for index array
        self.pointer = 0

    def next_batch(self, batch_size):
        """
        Getting next batch of training data
        """
        if self.pointer + batch_size >= self.len:
            np.random.shuffle(self.index_array)
            self.pointer = 0

        batch_index = self.index_array[self.pointer:self.pointer + batch_size]
        self.pointer += batch_size

        return self.X[batch_index], self.Y[batch_index], self.Z[batch_index]


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Build the keras model for training.
    """
    def adversary_loss(l, signal):
        """
        Loss for adversaries outputs
        :param l: Trade off factor for training model 2. FOr model 3 choose -1
        :param signal: If signal or background distribution should be learned.
        :return: Loss function for the discriminator part of the Network.
        """
        back_constant = 0 if signal else 1

        def adv_loss(y, p):
            return -l * (y[:, 0] - back_constant) * sparse_categorical_crossentropy(y[:, 1:], p)
        return adv_loss

    param = {'use_relation_layers': False, 'lambda': 0, 'number_bins': 10, 'adversary_steps': 5}

    if isinstance(parameters, dict):
        param.update(parameters)

    # Restrain training to only one GPU if your machine has multiple GPUs
    # os.environ["CUDA_VISIBLE_DEVICES"] = '0'
    # Uncomment if you are using GPU and don't want to occupy all GPU resources.
    # gpu_options = tf.GPUOptions(allow_growth=True)
    # s = tf.InteractiveSession(config=tf.ConfigProto(gpu_options=gpu_options))

    # Build classifier
    input = Input((number_of_features,))

    # The slicing in relation layers is only accurate if your are using the variables from
    # choose_input_features(True, True, 1).
    # For an example of Relation Layers see: mva/examples/keras/relational_network.py
    if param['use_relation_layers']:
        low_level_input = Lambda(slice, arguments={'begin': 0, 'end': 560})(input)
        high_level_input = Lambda(slice, arguments={'begin': 560, 'end': 590})(input)
        relations_tracks = Lambda(slice, arguments={'begin': 0, 'end': 340})(low_level_input)
        relations_tracks = Reshape((20, 17))(relations_tracks)
        relations_clusters = Lambda(slice, arguments={'begin': 340, 'end': 560})(low_level_input)
        relations_clusters = Reshape((20, 11))(relations_clusters)

        relations1 = EnhancedRelations(number_features=20, hidden_feature_shape=[80, 80, 80])([relations_tracks, high_level_input])
        relations2 = EnhancedRelations(number_features=20, hidden_feature_shape=[
                                       80, 80, 80])([relations_clusters, high_level_input])

        relations_output1 = GlobalAveragePooling1D()(relations1)
        relations_output2 = GlobalAveragePooling1D()(relations2)

        net = Concatenate()([relations_output1, relations_output2])

        net = Dense(units=100, activation=tanh)(net)
        net = Dropout(0.5)(net)
        net = Dense(units=100, activation=tanh)(net)
        net = Dropout(0.5)(net)

    else:
        net = Dense(units=50, activation=tanh)(input)
        net = Dense(units=50, activation=tanh)(net)
        net = Dense(units=50, activation=tanh)(net)

    output = Dense(units=1, activation=sigmoid)(net)

    # Model for applying Data. Loss function will not be used for training, if adversary is used.
    apply_model = Model(input, output)
    apply_model.compile(optimizer=adam(), loss=binary_crossentropy, metrics=['accuracy'])

    state = State(apply_model, use_adv=param['lambda'] > 0 and number_of_spectators > 0, preprocessor_state=None,
                  custom_objects={'EnhancedRelations': EnhancedRelations})

    # The following is only relevant when using Adversaries
    # See mva/examples/keras/adversary_network.py  for details
    if state.use_adv:
        adversaries, adversary_losses_model1, adversary_losses_model2 = [], [], []
        for mode in ['signal', 'background']:
            for i in range(number_of_spectators):
                adversary1 = Dense(units=2 * param['number_bins'], activation=tanh, trainable=False)(output)
                adversary2 = Dense(units=2 * param['number_bins'], activation=tanh, trainable=False)(adversary1)
                adversaries.append(Dense(units=param['number_bins'], activation=softmax, trainable=False)(adversary2))

                adversary_losses_model1.append(adversary_loss(param['lambda'], mode == 'signal'))
                adversary_losses_model2.append(adversary_loss(-1, mode == 'signal'))

        # Model which trains first part of the net
        model1 = Model(input, [output] + adversaries)
        model1.compile(optimizer=adam(),
                       loss=[binary_crossentropy] + adversary_losses_model1, metrics=['accuracy'])
        model1.summary()

        # Model which train second, adversary part of the net
        model2 = Model(input, adversaries)
        # freeze everything except adversary layers
        for layer in model2.layers:
            layer.trainable = not layer.trainable

        model2.compile(optimizer=adam(), loss=adversary_losses_model2,
                       metrics=['accuracy'])
        model2.summary()

        state.forward_model, state.adv_model = model1, model2
        state.K = parameters['adversary_steps']
        state.number_bins = param['number_bins']

    return state


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """
    Save Validation Data for monitoring Training
    """
    state.Xtest = Xtest
    state.Stest = Stest
    state.ytest = ytest

    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Fit the model.
    For every training step of MLP. Adverserial Network (if used) will be trained K times.
    """
    # Apply equal frequency binning for input data
    # See mva/examples/keras/preprocessing.py for details
    preprocessor = fast_equal_frequency_binning()
    preprocessor.fit(X, number_of_bins=500)
    X = preprocessor.apply(X)
    state.Xtest = preprocessor.apply(state.Xtest)
    # save preprocessor state in the State class
    state.preprocessor_state = preprocessor.export_state()

    def build_adversary_target(p_y, p_s):
        """
        Concat isSignal and spectator bins, because both are target information for the adversary.
        """
        return [np.concatenate((p_y, i), axis=1) for i in np.split(p_s, len(p_s[0]), axis=1)] * 2

    if state.use_adv:
        # Get bin numbers of S with equal frequency binning
        S_preprocessor = fast_equal_frequency_binning()
        S_preprocessor.fit(S, number_of_bins=state.number_bins)
        S = S_preprocessor.apply(S) * state.number_bins
        state.Stest = S_preprocessor.apply(state.Stest) * state.number_bins
        # Build target for adversary loss function
        target_array = build_adversary_target(y, S)
        target_val_array = build_adversary_target(state.ytest, state.Stest)
        # Build Batch Generator for adversary Callback
        state.batch_gen = batch_generator(X, y, S)

    class AUC_Callback(keras.callbacks.Callback):
        """
        Callback to print AUC after every epoch.
        """

        def on_train_begin(self, logs={}):
            self.val_aucs = []

        def on_epoch_end(self, epoch, logs={}):
            val_y_pred = state.model.predict(state.Xtest).flatten()
            val_auc = roc_auc_score(state.ytest, val_y_pred)
            print('\nTest AUC: {}\n'.format(val_auc))
            self.val_aucs.append(val_auc)
            return

    class Adversary(keras.callbacks.Callback):
        """
        Callback to train Adversary
        """

        def on_batch_end(self, batch, logs={}):
            v_X, v_y, v_S = state.batch_gen.next_batch(500 * state.K)
            target_adversary = build_adversary_target(v_y, v_S)
            state.adv_model.fit(v_X, target_adversary, verbose=0, batch_size=500)

    if not state.use_adv:
        state.model.fit(X, y, batch_size=500, epochs=100000, validation_data=(state.Xtest, state.ytest),
                        callbacks=[EarlyStopping(monitor='val_loss', patience=10, mode='min'), AUC_Callback()])
    else:
        state.forward_model.fit(X, [y] + target_array, batch_size=500, epochs=100000,
                                callbacks=[EarlyStopping(monitor='val_loss', patience=10, mode='min'), AUC_Callback(), Adversary()],
                                validation_data=(state.Xtest, [state.ytest] + target_val_array))
    return False


def apply(state, X):
    """
    Apply estimator to passed data.
    Has to be overwritten, because also the expert has to apply preprocessing.
    """
    # The preprocessor state is automatically loaded in the load function
    preprocessor = fast_equal_frequency_binning(state.preprocessor_state)
    # Apply preprocessor
    X = preprocessor.apply(X)

    r = state.model.predict(X).flatten()
    return np.require(r, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])
