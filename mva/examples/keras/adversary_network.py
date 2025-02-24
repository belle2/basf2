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
from basf2_mva_extensions.preprocessing import fast_equal_frequency_binning
import basf2_mva_util

from keras.layers import Dense, Input
from keras.models import Model
from keras.optimizers import Adam
from keras.losses import binary_crossentropy, sparse_categorical_crossentropy
from keras.activations import sigmoid, tanh, softmax
from keras.callbacks import EarlyStopping, Callback

from sklearn.metrics import roc_auc_score

import numpy as np
import time
import warnings
warnings.filterwarnings('ignore', category=UserWarning)


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Building 3 keras models:
    1. Network without adversary, used for apply data.
    2. Frozen MLP with unfrozen Adverserial Network to train adverserial part of network.
    3. Unfrozen MLP with frozen adverserial to train MLP part of the network,
       combined with losses of the adverserial networks.
    """

    def adversary_loss(signal):
        """
        Loss for adversaries outputs
        :param signal: If signal or background distribution should be learned.
        :return: Loss function for the discriminator part of the Network.
        """
        back_constant = 0 if signal == 'signal' else 1
        sign = 1 if signal == 'signal' else -1

        def adv_loss(y, p):
            return sign * (y[:, 0] - back_constant) * sparse_categorical_crossentropy(y[:, 1:], p)
        return adv_loss

    # Define inputs for input_feature and spectator
    input_layer = Input(shape=(number_of_features,))

    # build first model which will produce the desired discriminator
    layer1 = Dense(units=number_of_features + 1, activation=tanh)(input_layer)
    layer2 = Dense(units=number_of_features + 1, activation=tanh)(layer1)
    layer3 = Dense(units=number_of_features + 1, activation=tanh)(layer2)
    output = Dense(units=1, activation=sigmoid)(layer3)

    # Model for applying Data. Loss function will not be used for training, if adversary is used.
    apply_model = Model(input_layer, output)
    apply_model.compile(optimizer=Adam(learning_rate=parameters['learning_rate']), loss=binary_crossentropy, metrics=['accuracy'])

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
        forward_model = Model(input_layer, [output] + adversaries)
        forward_model.compile(optimizer=Adam(learning_rate=parameters['learning_rate']),
                              loss=[binary_crossentropy] + adversary_losses_model, metrics=['accuracy'] * (len(adversaries) + 1),
                              loss_weights=[1.0] + [float(-parameters['lambda'])] * len(adversary_losses_model))
        forward_model.summary()

        # Model which train second, adversary part of the net
        adv_model = Model(input_layer, adversaries)
        # mark which layers we want to be trainable in the adversarial network
        for layer in adv_model.layers:
            layer.train_in_adversarial_mode = not layer.trainable

        adv_model.compile(optimizer=Adam(learning_rate=parameters['learning_rate']), loss=adversary_losses_model,
                          metrics=['accuracy'] * len(adversaries))
        adv_model.summary()

        state.forward_model = forward_model
        state.adv_model = adv_model
        state.K = parameters['adversary_steps']
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

    if not state.use_adv:
        state.model.fit(X, y, batch_size=400, epochs=1000, validation_data=(state.Xtest, state.ytest),
                        callbacks=[EarlyStopping(monitor='val_loss', patience=20, mode='min'), AUC_Callback()])
    else:
        class Adversary(Callback):
            """
            Callback to train Adversary
            """

            def on_batch_end(self, batch, logs=None):
                # freeze the layers of the forward network and unfreeze the adversarial layers
                for layer in state.adv_model.layers:
                    layer.trainable = layer.train_in_adversarial_mode

                state.adv_model.fit(X, target_array, verbose=0, batch_size=400, steps_per_epoch=state.K, epochs=1)

                # unfreeze the layers of the forward network and freeze the adversarial layers
                for layer in state.adv_model.layers:
                    layer.trainable = not layer.train_in_adversarial_mode

        state.forward_model.fit(X, [y] + target_array, batch_size=400, epochs=1000,
                                callbacks=[
            EarlyStopping(
                monitor='val_loss',
                patience=20,
                mode='min'),
            AUC_Callback(), Adversary()],
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
    test_file = find_file("mva/test_D0toKpipi.root", "examples")

    training_data = basf2_mva.vector(train_file)
    testing_data = basf2_mva.vector(test_file)

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
    specific_options.m_training_fraction = 0.8

    print(train_file)

    """
    Config for Adversary Networks:
    lambda: Trade off between classifier performance and noncorrelation between classifier output and spectators.
            Increase to reduce correlations(and also classifier performance)
    adversary_steps: How many batches the discriminator is trained after one batch of training the classifier.
            Less steps make the training faster but also unstable. Increase the parameter if something isn't working.
    number_bins: Number of Bins which are used to quantify the spectators. 10 should be sufficient.
    """

    specific_options.m_config = '{"adversary_steps": 5, "learning_rate": 0.01, "lambda": 5.0, "number_bins": 10}'
    basf2_mva.teacher(general_options, specific_options)

    method = basf2_mva_util.Method(general_options.m_identifier)
    inference_start = time.time()
    p, t = method.apply_expert(testing_data, general_options.m_treename)
    inference_time = time.time() - inference_start
    auc = basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)
    print("Adversary:", inference_time, auc)

    general_options.m_identifier = "keras_baseline"
    specific_options.m_config = '{"adversary_steps": 1, "learning_rate": 0.001, "lambda": 0.0, "number_bins": 10}'
    basf2_mva.teacher(general_options, specific_options)

    method = basf2_mva_util.Method(general_options.m_identifier)
    inference_start = time.time()
    p, t = method.apply_expert(testing_data, general_options.m_treename)
    inference_time = time.time() - inference_start
    auc = basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)
    print("Baseline:", inference_time, auc)

    # Now lets drop some of the features most correlated to the spectator variables.
    general_options.m_variables = basf2_mva.vector(*variables2)
    general_options.m_identifier = "keras_feature_drop"
    basf2_mva.teacher(general_options, specific_options)

    method = basf2_mva_util.Method(general_options.m_identifier)
    inference_start = time.time()
    p, t = method.apply_expert(testing_data, general_options.m_treename)
    inference_time = time.time() - inference_start
    auc = basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)
    print("Drop features:", inference_time, auc)

    # Uncomment the following lines to run basf2_mva_evaluation.py and compare all three methods
    # import os
    # os.system(
    #     f'basf2_mva_evaluate.py -id keras_adversary keras_baseline keras_feature_drop '\
    #     f'-train {train_file} -data {test_file} -c -out adversarial_output.pdf -l localdb/database.txt')
