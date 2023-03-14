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
import uproot3 as uproot

from basf2_mva_python_interface.tensorflow import State


# This example is based on
# 'Machine learning algorithms for the Belle II experiment and their validation on Belle data' - Thomas Keck (2017), Section 3.3.2.4
# See also Algorithm 1 in https://arxiv.org/pdf/1611.01046.pdf

def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):

    gpus = tf.config.list_physical_devices('GPU')
    if gpus:
        for gpu in gpus:
            tf.config.experimental.set_memory_growth(gpu, True)

    # don't run the adversarial net if lambda is negative
    if parameters.get('lambda', 50) <= 0:
        number_of_spectators = 0

    def dense(x, W, b, activation_function):
        return activation_function(tf.matmul(x, W) + b)

    class adversarial_model(tf.Module):

        def __init__(self, **kwargs):
            super().__init__(**kwargs)

            self.optimizer = tf.optimizers.Adam(parameters.get('learning_rate', 0.01))
            self.epsilon = 1e-5
            # strength of adversarial constraint
            self.lam = parameters.get('lambda', 50)
            # number of adversarial training epochs per inference training epoch
            self.K = parameters.get('adversary_steps', 13)
            # number of epochs training the inference net before switching on the adversarial networks
            self.pre_train_epochs = parameters.get('pre_train_epochs', 0)
            self.advarsarial = number_of_spectators > 0

            def create_layer_variables(shape, name, activation_function=tf.tanh):
                weights = tf.Variable(
                      tf.random.truncated_normal(shape, stddev=1.0 / np.sqrt(float(shape[0]))),
                      name=f'{name}_weights')
                biases = tf.Variable(tf.zeros(shape=[shape[1]]), name=f'{name}_biases')
                return weights, biases, activation_function

            self.inference_layers = [
                                     create_layer_variables([number_of_features, number_of_features+1],
                                                            'inference_hidden', tf.sigmoid),
                                     create_layer_variables([number_of_features+1, 1],
                                                            'inference_sigmoid', tf.sigmoid),
                                    ]

            # create the layers for the adversarial net.
            # This has 2*n_spectators sets of layers, one each for signal and background for each spectator variable
            # the final 3 layers (means, widths, fractions) form part of the activation function and are not evaluated in sequence
            self.adversarial_layers = []
            for i in range(number_of_spectators):
                self.adversarial_layers.append([])
                for i_c, c in enumerate(['signal', 'background']):
                    self.adversarial_layers[i].append([
                        create_layer_variables([1, number_of_spectators+1], f'adversary_hidden_{i}_{c}',
                                               activation_function=tf.tanh),
                        create_layer_variables([number_of_spectators + 1, 4], f'adversary_means_{i}_{c}',
                                               activation_function=tf.identity),
                        create_layer_variables([number_of_spectators + 1, 4], f'adversary_widths_{i}_{c}',
                                               activation_function=tf.exp),
                        create_layer_variables([number_of_spectators + 1, 4], f'adversary_fractions_{i}_{c}',
                                               activation_function=tf.identity)
                    ])
            return

        @tf.function(input_signature=[tf.TensorSpec(shape=[None, number_of_features], dtype=tf.float32)])
        def __call__(self, x):
            for i in range(len(self.inference_layers)):
                x = dense(x, *self.inference_layers[i])
            return x

        @tf.function
        def inference_loss(self, predicted_y, target_y, w):
            diff_from_truth = tf.where(target_y == 1., predicted_y, 1. - predicted_y)
            cross_entropy = - tf.reduce_sum(w * tf.math.log(diff_from_truth + self.epsilon)) / tf.reduce_sum(w)
            return cross_entropy

        @tf.function
        def adversarial_loss(self, predicted_y, s, target_y, w):

            if number_of_spectators == 0:
                return tf.constant(0.)

            adversary_losses = []
            for i_spectator in range(number_of_spectators):
                s_single = tf.slice(s, [0, i_spectator], [-1, 1])  # get the single spectator variable column
                for i_c, c in enumerate(['signal', 'background']):

                    # loop through the hidden layers
                    x = tf.identity(predicted_y)
                    for i_layer in range(len(self.adversarial_layers[i_spectator][i_c])-3):
                        x = dense(x, *self.adversarial_layers[i_spectator][i_c][i_layer])

                    # calculate the activation function and loss
                    means = dense(x, *self.adversarial_layers[i_spectator][i_c][-3])
                    widths = dense(x, *self.adversarial_layers[i_spectator][i_c][-2])
                    fractions = dense(x, *self.adversarial_layers[i_spectator][i_c][-1])

                    activation = tf.reduce_sum(tf.nn.softmax(fractions) *
                                               tf.exp(-(means-s_single)*(means-s_single) / (2*widths)) /
                                               tf.sqrt(2 * np.pi * widths), axis=1)

                    if c == 'signal':
                        loss = -tf.reduce_sum(target_y*w*tf.math.log(activation + self.epsilon)) / tf.reduce_sum(target_y*w)
                    else:
                        loss = -tf.reduce_sum((1-target_y)*w*tf.math.log(activation + self.epsilon)) / tf.reduce_sum((1-target_y)*w)
                    adversary_losses.append(loss)

            return tf.math.add_n(adversary_losses)

        @tf.function
        def loss(self, x, s, y, w):
            predicted_y = self.__call__(x)
            inference_loss = self.inference_loss(predicted_y, y, w)
            adversary_loss = self.adversarial_loss(predicted_y, s, y, w)
            return inference_loss - self.lam * adversary_loss

    state = State(model=adversarial_model())

    state.epoch = 0
    state.avg_costs = []  # keeps track of the avg costs per batch over an epoch
    return state


def partial_fit(state, X, S, y, w, epoch, batch):
    """
    Pass batches of received data to tensorflow
    """
    with tf.GradientTape() as tape:
        if epoch < state.model.pre_train_epochs:
            cost = state.model.inference_loss(state.model(X), y, w)
            trainable_vars = [v for v in state.model.trainable_variables if 'inference' in v.name]

        elif epoch % state.model.K == 0 or not state.model.advarsarial:
            cost = state.model.loss(X, S, y, w)
            trainable_vars = [v for v in state.model.trainable_variables if 'inference' in v.name]

        else:
            cost = state.model.adversarial_loss(state.model(X), S, y, w)
            trainable_vars = [v for v in state.model.trainable_variables if 'adversary' in v.name]

        grads = tape.gradient(cost, trainable_vars)

    state.model.optimizer.apply_gradients(zip(grads, trainable_vars))

    if batch == 0 and epoch == 0:
        state.avg_costs = [cost]
    elif batch != state.nBatches-1:
        state.avg_costs.append(cost)
    else:
        # end of the epoch, print summary results, reset the avg_costs and update the counter
        print(f"Epoch: {epoch:04d} cost= {np.mean(state.avg_costs):.9f}")
        state.avg_costs = [cost]

    if epoch == 100000:
        return False
    return True


if __name__ == "__main__":
    from basf2 import conditions
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

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_spectators = basf2_mva.vector('daughterInvM(0, 1)', 'daughterInvM(0, 2)')
    general_options.m_target_variable = "isSignal"
    general_options.m_identifier = "tensorflow"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "tensorflow"
    specific_options.m_steering_file = 'mva/examples/orthogonal_discriminators/tensorflow_adversary.py'
    specific_options.m_normalize = True
    specific_options.m_nIterations = 1000
    specific_options.m_mini_batch_size = 400
    specific_options.m_config = '{"pre_train_epochs" : 50, "adversary_steps": 7, '\
        ' "learning_rate": 0.001, "lambda": 0.01}'
    basf2_mva.teacher(general_options, specific_options)

    method = basf2_mva_util.Method(general_options.m_identifier)
    test_data = ["test.root"]
    p, t = method.apply_expert(basf2_mva.vector(*test_data), general_options.m_treename)
    auc = basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)
    results = {}
    results['adversarial'] = {'p': p, 't': t, 'auc': auc}

    general_options.m_identifier = "tensorflow_baseline"
    specific_options.m_nIterations = 200
    specific_options.m_mini_batch_size = 400
    specific_options.m_config = '{"pre_train_epochs" : 0, "adversary_steps": 1, '\
        ' "learning_rate": 0.001, "lambda": -1.0}'
    basf2_mva.teacher(general_options, specific_options)

    method = basf2_mva_util.Method(general_options.m_identifier)
    test_data = ["test.root"]
    p, t = method.apply_expert(basf2_mva.vector(*test_data), general_options.m_treename)
    auc = basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)
    results['baseline'] = {'p': p, 't': t, 'auc': auc}

    general_options.m_variables = basf2_mva.vector(*variables2)
    general_options.m_identifier = "tensorflow_feature_drop"
    specific_options.m_nIterations = 200
    specific_options.m_mini_batch_size = 400
    specific_options.m_config = '{"pre_train_epochs" : 0, "adversary_steps": 1, '\
        ' "learning_rate": 0.001, "lambda": -1.0}'
    basf2_mva.teacher(general_options, specific_options)

    method = basf2_mva_util.Method(general_options.m_identifier)
    test_data = ["test.root"]
    p, t = method.apply_expert(basf2_mva.vector(*test_data), general_options.m_treename)
    auc = basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)
    results['featureDrop'] = {'p': p, 't': t, 'auc': auc}

    test_tree = uproot.open('test.root')['tree']
    invMassDaughter01 = test_tree.array('daughterInvM__bo0__cm__sp1__bc')
    invMassDaughter02 = test_tree.array('daughterInvM__bo0__cm__sp2__bc')
    isSignal = test_tree.array('isSignal').astype(np.int)

    def print_summary(name, data):

        def get_corr(p, var, isSignal, sig=True):
            if not sig:
                isSignal = (1 - isSignal)
            signal_mask = np.where(isSignal)
            p = p[signal_mask]
            var = var[signal_mask]
            return np.corrcoef(p, var)[0, 1]

        print(f'{name}:')
        print(f'auc: {data["auc"]:.3f}')
        print('Correlations between probability and spectator variables:')
        print(
            f'Sig: {get_corr(data["p"], invMassDaughter01, isSignal, True):.3f},'
            f' {get_corr(data["p"], invMassDaughter02, isSignal, True):.3f}')
        print(
            f'Bkg: {get_corr(data["p"], invMassDaughter01, isSignal, False):.3f},'
            f' {get_corr(data["p"], invMassDaughter02, isSignal, False):.3f}\n')

    print_summary('Baseline', results['baseline'])
    print_summary('Adversarial', results['adversarial'])
    print_summary('Feature Drop', results['featureDrop'])
