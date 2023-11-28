#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This example shows how to remove bias on one or several spectator variables.
# Relevant paper: https://arxiv.org/abs/1611.01046
# use basf2_mva_evaluation.py with train.root and test.root at the end to see the impact on the spectator variables.

import basf2_mva
from basf2_mva_python_interface.keras import State

from tensorflow.keras.layers import Dense, Input
from tensorflow.keras.models import Model
from tensorflow.keras.optimizers import Adam
from tensorflow.keras.losses import binary_crossentropy, sparse_categorical_crossentropy
from tensorflow.keras.activations import sigmoid, tanh, softmax
from tensorflow.keras.callbacks import EarlyStopping, Callback
from tensorflow.keras.utils import plot_model

import numpy as np
from basf2_mva_extensions.preprocessing import fast_equal_frequency_binning

from sklearn.metrics import roc_auc_score

import warnings
warnings.filterwarnings('ignore', category=UserWarning)


class batch_generator():
    """
    Class to create batches for training the Adversary Network.
    Once the steps_per_epoch argument is available for the fit function in keras, this class will become obsolete.
    """

    def __init__(self, X, Y, Z):
        """
        Init the class
        :param X: Input Features
        :param Y: Label Data
        :param Z: Spectators/Quantity to be uncorrelated to
        """
        #: Input Features
        self.X = X
        #: Label data
        self.Y = Y
        #: Spectatirs/Quantity to be uncorrelated to
        self.Z = Z
        #: Length of the data
        self.len = len(Y)
        #: Index array containing indices from 0 to len
        self.index_array = np.arange(self.len)
        #: Pointer to the current start of the batch
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
    Building 3 keras models:
    1. Network without adversary, used for apply data.
    2. Freezed MLP with unfreezed Adverserial Network to train adverserial part of network.
    3. Unfreezed MLP with freezed adverserial to train MLP part of the network,
       combined with losses of the adverserial networks.
    """

    def adversary_loss(signal):
        """
        Loss for adversaries outputs
        :param signal: If signal or background distribution should be learned.
        :return: Loss function for the discriminator part of the Network.
        """
        back_constant = 0 if signal else 1

        def adv_loss(y, p):
            return (y[:, 0] - back_constant) * sparse_categorical_crossentropy(y[:, 1:], p)
        return adv_loss

    # Define inputs for input_feature and spectator
    input = Input(shape=(number_of_features,))

    # build first model which will produce the desired discriminator
    layer1 = Dense(units=number_of_features + 1, activation=tanh)(input)
    layer2 = Dense(units=number_of_features + 1, activation=tanh)(layer1)
    layer3 = Dense(units=number_of_features + 1, activation=tanh)(layer2)
    output = Dense(units=1, activation=sigmoid)(layer3)

    # Model for applying Data. Loss function will not be used for training, if adversary is used.
    apply_model = Model(input, output)
    apply_model.compile(optimizer=Adam(lr=parameters['learning_rate']), loss=binary_crossentropy, metrics=['accuracy'])

    state = State(apply_model, use_adv=parameters['lambda'] > 0 and number_of_spectators > 0)
    state.number_bins = parameters['number_bins']

    # build second model on top of the first one which will try to predict spectators
    adversaries, adversary_losses_model = [], []
    if state.use_adv:
        for mode in ['signal', 'background']:
            for i in range(number_of_spectators):
                adversary1 = Dense(units=2 * parameters['number_bins'], activation=tanh, trainable=False)(output)
                adversary2 = Dense(units=2 * parameters['number_bins'], activation=tanh, trainable=False)(adversary1)
                adversaries.append(Dense(units=parameters['number_bins'], activation=softmax, trainable=False)(adversary2))

                adversary_losses_model.append(adversary_loss(mode == 'signal'))

        # Model which trains first part of the net
        model1 = Model(input, [output] + adversaries)
        model1.compile(optimizer=Adam(lr=parameters['learning_rate']),
                       loss=[binary_crossentropy] + adversary_losses_model, metrics=['accuracy'],
                       loss_weights=[1] + [-parameters['lambda']] * len(adversary_losses_model))
        model1.summary()

        # Model which train second, adversary part of the net
        model2 = Model(input, adversaries)
        # freeze everything except adversary layers
        for layer in model2.layers:
            layer.trainable = not layer.trainable

        model2.compile(optimizer=Adam(lr=parameters['learning_rate']), loss=adversary_losses_model,
                       metrics=['accuracy'])
        model2.summary()

        state.forward_model, state.adv_model = model1, model2
        state.K = parameters['adversary_steps']

        # draw model as a picture
        plot_model(model1, to_file='model.png', show_shapes=True)

    return state


def begin_fit(state, Xtest, Stest, ytest, wtest, nBatches):
    """
    Save Validation Data for monitoring Training
    """
    state.Xtest = Xtest
    state.Stest = Stest
    state.ytest = ytest

    return state


def partial_fit(state, X, S, y, w, epoch, batch):
    """
    Fit the model.
    For every training step of MLP. Adverserial Network will be trained K times.
    """

    def build_adversary_target(p_y, p_s):
        """
        Concat isSignal and spectator bins, because both are target information for the adversary.
        """
        return [np.concatenate((p_y, i), axis=1) for i in np.split(p_s, len(p_s[0]), axis=1)] * 2

    if state.use_adv:
        # Get bin numbers of S with equal frequency binning
        preprocessor = fast_equal_frequency_binning()
        preprocessor.fit(S, number_of_bins=state.number_bins)
        S = preprocessor.apply(S) * state.number_bins
        state.Stest = preprocessor.apply(state.Stest) * state.number_bins
        # Build target for adversary loss function
        target_array = build_adversary_target(y, S)
        target_val_array = build_adversary_target(state.ytest, state.Stest)
        # Build Batch Generator for adversary Callback
        state.batch_gen = batch_generator(X, y, S)

    class AUC_Callback(Callback):
        """
        Callback to print AUC after every epoch.
        """

        def on_train_begin(self, logs=None):
            self.val_aucs = []

        def on_epoch_end(self, epoch, logs=None):
            val_y_pred = state.model.predict(state.Xtest).flatten()
            val_auc = roc_auc_score(state.ytest, val_y_pred)
            print(f'\nTest AUC: {val_auc}\n')
            self.val_aucs.append(val_auc)
            return

    class Adversary(Callback):
        """
        Callback to train Adversary
        """

        def on_batch_end(self, batch, logs=None):
            v_X, v_y, v_S = state.batch_gen.next_batch(400 * state.K)
            target_adversary = build_adversary_target(v_y, v_S)
            state.adv_model.fit(v_X, target_adversary, verbose=0, batch_size=400)

    if not state.use_adv:
        state.model.fit(X, y, batch_size=400, epochs=1000, validation_data=(state.Xtest, state.ytest),
                        callbacks=[EarlyStopping(monitor='val_loss', patience=2, mode='min'), AUC_Callback()])
    else:
        state.forward_model.fit(X, [y] + target_array, batch_size=400, epochs=1000,
                                callbacks=[EarlyStopping(monitor='val_loss', patience=2, mode='min'), AUC_Callback(), Adversary()],
                                validation_data=(state.Xtest, [state.ytest] + target_val_array))
    return False


if __name__ == "__main__":
    from basf2 import conditions, find_file
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    variables = ['p', 'pt', 'pz', 'phi',
                 'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)', 'daughter(0, phi)',
                 'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)', 'daughter(1, phi)',
                 'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)', 'daughter(2, phi)',
                 'chiProb', 'dr', 'dz', 'dphi',
                 'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, dz)', 'daughter(1, dz)',
                 'daughter(0, dphi)', 'daughter(1, dphi)',
                 'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
                 'daughter(0, kaonID)', 'daughter(0, pionID)', 'daughter(1, kaonID)', 'daughter(1, pionID)',
                 'daughterAngle(0, 1)', 'daughterAngle(0, 2)', 'daughterAngle(1, 2)',
                 'daughter(2, daughter(0, E))', 'daughter(2, daughter(1, E))',
                 'daughter(2, daughter(0, clusterTiming))', 'daughter(2, daughter(1, clusterTiming))',
                 'daughter(2, daughter(0, clusterE9E25))', 'daughter(2, daughter(1, clusterE9E25))',
                 'daughter(2, daughter(0, minC2TDist))', 'daughter(2, daughter(1, minC2TDist))',
                 'M']

    variables2 = ['p', 'pt', 'pz', 'phi',
                  'chiProb', 'dr', 'dz', 'dphi',
                  'daughter(2, chiProb)',
                  'daughter(0, kaonID)', 'daughter(0, pionID)', 'daughter(1, kaonID)', 'daughter(1, pionID)',
                  'daughter(2, daughter(0, E))', 'daughter(2, daughter(1, E))',
                  'daughter(2, daughter(0, clusterTiming))', 'daughter(2, daughter(1, clusterTiming))',
                  'daughter(2, daughter(0, clusterE9E25))', 'daughter(2, daughter(1, clusterE9E25))',
                  'daughter(2, daughter(0, minC2TDist))', 'daughter(2, daughter(1, minC2TDist))']

    train_file = find_file("mva/train_D0toKpipi.root", "examples")
    training_data = basf2_mva.vector(train_file)

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_spectators = basf2_mva.vector('daughterInvM(0, 1)', 'daughterInvM(0, 2)')
    general_options.m_target_variable = "isSignal"
    general_options.m_identifier = "keras_adversary"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "keras"
    specific_options.m_steering_file = 'mva/examples/keras/adversary_network.py'
    specific_options.m_normalize = True
    specific_options.m_training_fraction = 0.9

    """
    Config for Adversary Networks:
    lambda: Trade off between classifier performance and noncorrelation between classifier output and spectators.
            Increase to reduce correlations(and also classifier performance)
    adversary_steps: How many batches the discriminator is trained after one batch of training the classifier.
            Less steps make the training faster but also unstable. Increase the parameter if something isn't working.
    number_bins: Number of Bins which are used to quantify the spectators. 10 should be sufficient.
    """
    specific_options.m_config = '{"adversary_steps": 5, "learning_rate": 0.001, "lambda": 20.0, "number_bins": 10}'
    basf2_mva.teacher(general_options, specific_options)

    general_options.m_identifier = "keras_baseline"
    specific_options.m_config = '{"adversary_steps": 1, "learning_rate": 0.001, "lambda": 0.0, "number_bins": 10}'
    basf2_mva.teacher(general_options, specific_options)

    general_options.m_variables = basf2_mva.vector(*variables2)
    general_options.m_identifier = "keras_feature_drop"
    basf2_mva.teacher(general_options, specific_options)
