#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Dennis Weyland 2017

# This example shows how to remove bias on one or several spectator variables.
# Relevant paper: https://arxiv.org/abs/1611.01046
# use basf2_mva_evaluation.py with train.root and test.root at the end to see the impact on the spectator variables.

import basf2
import basf2_mva
from basf2_mva_python_interface.contrib_keras import State
import h5py

import tensorflow as tf
import tensorflow.contrib.keras as keras

from keras.layers import Input, Dense, Concatenate, Lambda
from keras.models import Model, load_model
from keras.optimizers import adam
from keras.losses import binary_crossentropy
from keras.activations import sigmoid, tanh, softmax
from keras import backend as K
from keras.callbacks import Callback, EarlyStopping
from keras.utils import plot_model

import numpy as np


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Building 3 keras models:
    1. Network without adversary, used for apply data.
    2. Freezed MLP with unfreezed Adverserial Network to train adverserial part of network.
    3. Unfreezed MLP with freezed adverserial to train MLP part of the network,
       combined with losses of the adverserial networks.
    """

    def gaussian(input):
        z, means, widths, fractions = input
        return K.sum(fractions * K.exp(-(means - z) * (means - z) / (2 * widths)) /
                     K.sqrt(2 * np.pi * widths), axis=1, keepdims=True)

    def adversary_loss(signal, l):
        epsilon = 1e-5

        def adv_loss(y, p):
            var_y = y if signal else (1 - y)
            return -(-l) * K.sum(var_y * tf.log(p + epsilon)) / K.sum(var_y)
        return adv_loss

    def slice(input, start, end):
        return input[:, start:end]

    # Define inputs for input_feature and spectator
    input1 = Input(shape=(number_of_features,))
    input2 = Input(shape=(number_of_spectators,))

    # build first model which will produce the desired discriminator
    layer1 = Dense(units=number_of_features + 1, activation=tanh)(input1)
    layer2 = Dense(units=number_of_features + 1, activation=tanh)(layer1)
    layer3 = Dense(units=number_of_features + 1, activation=tanh)(layer2)
    output1 = Dense(units=1, activation=sigmoid)(layer3)

    # Model for applying Data. Loss function will not used for training, if adversary is used.
    apply_model = Model(input1, output1)
    apply_model.compile(optimizer=adam(lr=parameters['learning_rate']), loss=binary_crossentropy, metrics=['accuracy'])

    state = State(apply_model, use_adv=parameters['lambda'] != 0)

    # build second model on top of the first one which will try to predict spectators
    signal_adversaries, back_adversaries = [], []
    if state.use_adv:
        for i in range(number_of_spectators):
            z_single = Lambda(slice, arguments={'start': i, 'end': i + 1})(input2)
            for mode in ['signal', 'background']:
                adversary1 = Dense(units=number_of_features + 1, activation=tanh, trainable=False)(output1)
                adversary2 = Dense(units=number_of_features + 1, activation=tanh, trainable=False)(adversary1)
                adversary_means = Dense(units=4, trainable=False)(adversary2)
                adversary_widths = Dense(units=4, activation=K.exp, trainable=False)(adversary2)
                adversary_fractions = Dense(units=4, activation=softmax, trainable=False)(adversary2)
                adversary_activation = Lambda(gaussian)([z_single, adversary_means, adversary_widths, adversary_fractions])
                if mode == 'signal':
                    signal_adversaries.append(adversary_activation)
                else:
                    back_adversaries.append(adversary_activation)

        # Building loss functions for every model and case
        signal_losses = [adversary_loss(True, parameters['lambda'])] * len(signal_adversaries)
        back_losses = [adversary_loss(False, parameters['lambda'])] * len(back_adversaries)
        adv_signal_losses = [adversary_loss(True, -1.)] * len(signal_adversaries)
        adv_back_losses = [adversary_loss(False, -1.)] * len(back_adversaries)

        # Model which trains first part of the net
        model1 = Model([input1, input2], [output1] + signal_adversaries + back_adversaries)
        model1.compile(optimizer=adam(lr=parameters['learning_rate']), loss=[binary_crossentropy] + signal_losses + back_losses,
                       metrics=['accuracy'])
        model1.summary()

        # Model which train second, adversary part of the net
        model2 = Model([input1, input2], signal_adversaries + back_adversaries)
        # freeze everything except adversary layers
        for layer in model2.layers:
            layer.trainable = not layer.trainable

        model2.compile(optimizer=adam(lr=parameters['learning_rate']), loss=adv_signal_losses + adv_back_losses,
                       metrics=['accuracy'])
        model2.summary()

        state.forward_model, state.adv_model = model1, model2
        state.K = parameters['adversary_steps']
        state.number_loss_functions = len(signal_adversaries) + len(back_adversaries)

        # draw model as a picture
        plot_model(model1, to_file='model.png', show_shapes=True)

    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Fit the model.
    For every training step of MLP. Adverserial Network will be trained K times.
    """
    if not state.use_adv:
        state.model.train_on_batch(X, y)
    elif epoch % state.K == 0:
        state.forward_model.train_on_batch([X, S], [y] * (state.number_loss_functions + 1))
        avg_cost = state.forward_model.test_on_batch([X, S], [y] * (state.number_loss_functions + 1))[0]
        if epoch % 20 == 0:
            print('Epoch {}: Cost: {}'.format(epoch, avg_cost))
    else:
        state.adv_model.train_on_batch([X, S], [y] * state.number_loss_functions)
    return True


if __name__ == "__main__":
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
                 'daughter(2, daughter(0, minC2HDist))', 'daughter(2, daughter(1, minC2HDist))',
                 'M']

    variables2 = ['p', 'pt', 'pz', 'phi',
                  'chiProb', 'dr', 'dz', 'dphi',
                  'daughter(2, chiProb)',
                  'daughter(0, kaonID)', 'daughter(0, pionID)', 'daughter(1, kaonID)', 'daughter(1, pionID)',
                  'daughter(2, daughter(0, E))', 'daughter(2, daughter(1, E))',
                  'daughter(2, daughter(0, clusterTiming))', 'daughter(2, daughter(1, clusterTiming))',
                  'daughter(2, daughter(0, clusterE9E25))', 'daughter(2, daughter(1, clusterE9E25))',
                  'daughter(2, daughter(0, minC2HDist))', 'daughter(2, daughter(1, minC2HDist))']

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_spectators = basf2_mva.vector('daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)')
    general_options.m_target_variable = "isSignal"
    general_options.m_identifier = "keras"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "contrib_keras"
    specific_options.m_steering_file = 'mva/examples/keras/adversary_network.py'
    specific_options.m_normalize = True
    specific_options.m_nIterations = 1000
    specific_options.m_mini_batch_size = 400
    specific_options.m_config = '{"adversary_steps": 13, "learning_rate": 0.001, "lambda": 0.1}'
    basf2_mva.teacher(general_options, specific_options)

    general_options.m_identifier = "keras_baseline"
    specific_options.m_nIterations = 100
    specific_options.m_config = '{"adversary_steps": 1, "learning_rate": 0.001, "lambda": 0.0}'
    basf2_mva.teacher(general_options, specific_options)

    general_options.m_variables = basf2_mva.vector(*variables2)
    general_options.m_identifier = "keras_feature_drop"
    basf2_mva.teacher(general_options, specific_options)
