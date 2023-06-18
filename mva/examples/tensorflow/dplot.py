#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import find_file
import numpy as np
import tensorflow as tf
import basf2_mva

from basf2_mva_python_interface.tensorflow import State


class Prior:
    """
    Calculates prior from signal and background pdfs of the fit variable
    """

    def __init__(self, z, y):
        """
        Constructor of a new prior distribution
            @param z fit variable
            @param y target variable
        """
        #: signal cdf, pdf and binning
        self.signal_cdf, self.signal_pdf, self.signal_bins = calculate_cdf_and_pdf(z[y == 1])
        #: background cdf, pdf and binning
        self.bckgrd_cdf, self.bckgrd_pdf, self.bckgrd_bins = calculate_cdf_and_pdf(z[y == 0])
        # Avoid numerical instabilities
        self.bckgrd_pdf[0] = self.bckgrd_pdf[-1] = 1

    def get_signal_pdf(self, X):
        """
        Calculate signal pdf for given fit variable value
            @param X nd-array containing fit variable values
        """
        return self.signal_pdf[np.digitize(X, bins=self.signal_bins)]

    def get_bckgrd_pdf(self, X):
        """
        Calculate background pdf for given fit variable value
            @param X nd-array containing fit variable values
        """
        return self.bckgrd_pdf[np.digitize(X, bins=self.bckgrd_bins)]

    def get_signal_cdf(self, X):
        """
        Calculate signal cdf for given fit variable value
            @param X nd-array containing fit variable values
        """
        return self.signal_cdf[np.digitize(X, bins=self.signal_bins)]

    def get_bckgrd_cdf(self, X):
        """
        Calculate background cdf for given fit variable value
            @param X nd-array containing fit variable values
        """
        return self.bckgrd_cdf[np.digitize(X, bins=self.bckgrd_bins)]

    def get_prior(self, X):
        """
        Calculate prior signal probability for given fit variable value
            @param X nd-array containing fit variable values
        """
        prior = self.get_signal_pdf(X) / (self.get_signal_pdf(X) + self.get_bckgrd_pdf(X))
        prior = np.where(np.isfinite(prior), prior, 0.5)
        return prior

    def get_boost_weights(self, X):
        """
        Calculate boost weights used in dplot boost training step
            @param X nd-array containing fit variable values
        """
        signal_weight = self.get_signal_cdf(X) / self.get_bckgrd_pdf(X)
        signal_weight = np.where(np.isfinite(signal_weight), signal_weight, 0)
        # NOT self.get_bckgrd_cdf() here, signal and background are handled asymmetrical!
        bckgrd_weight = (1.0 - self.get_signal_cdf(X)) / self.get_bckgrd_pdf(X)
        bckgrd_weight = np.where(np.isfinite(bckgrd_weight), bckgrd_weight, 0)
        return np.r_[signal_weight, bckgrd_weight]

    def get_uncorrelation_weights(self, X, boost_prediction):
        """
        Calculate uncorrelation weights used in dplot classifier training step
            @param X nd-array containing fit variable values
            @param boost_prediction output of the boost classifier
        """
        reg_boost_prediction = boost_prediction * 0.99 + 0.005
        weights = (self.get_signal_cdf(X) / reg_boost_prediction +
                   (1.0 - self.get_signal_cdf(X)) / (1.0 - reg_boost_prediction)) / 2
        return weights


def calculate_cdf_and_pdf(X):
    """
    Calculates cdf and pdf of given sample and adds under/overflow bins
        @param X 1-d np.array
    """
    pdf, bins = np.histogram(X, bins=200, density=True)
    cdf = np.cumsum(pdf * (bins - np.roll(bins, 1))[1:])
    return np.hstack([0.0, cdf, 1.0]), np.hstack([0.0, pdf, 0.0]), bins


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):

    @tf.function
    def dense(x, W, b, activation_function):
        return activation_function(tf.matmul(x, W) + b)

    class my_model(tf.Module):

        def __init__(self, **kwargs):
            super().__init__(**kwargs)

            self.boost_optimizer = tf.optimizers.Adam(0.01)
            self.inference_optimizer = tf.optimizers.Adam(0.01)

            def create_layer_variables(shape, name, activation_function):
                weights = tf.Variable(
                    tf.random.truncated_normal(shape, stddev=1.0 / np.sqrt(float(shape[0]))),
                    name=f'{name}_weights')
                biases = tf.Variable(tf.zeros(shape=[shape[1]]), name=f'{name}_biases')
                return weights, biases, activation_function

            self.boost_layer_vars = []
            self.boost_layer_vars.append(create_layer_variables([number_of_features, 20], 'boost_input', tf.nn.sigmoid))
            for i in range(3):
                self.boost_layer_vars.append(create_layer_variables([20, 20], f'boost_hidden{i}', tf.nn.sigmoid))
            self.boost_layer_vars.append(create_layer_variables([20, 1], 'boost_output', tf.nn.sigmoid))

            self.inference_layer_vars = []
            self.inference_layer_vars.append(create_layer_variables([number_of_features, 20], 'inference_input', tf.nn.sigmoid))
            for i in range(3):
                self.inference_layer_vars.append(create_layer_variables([20, 20], f'inference_hidden{i}', tf.nn.sigmoid))
            self.inference_layer_vars.append(create_layer_variables([20, 1], 'inference_output', tf.nn.sigmoid))

            self.n_boost_layers = len(self.boost_layer_vars)
            self.n_inference_layers = len(self.inference_layer_vars)

        @tf.function(input_signature=[tf.TensorSpec(shape=[None, number_of_features], dtype=tf.float32)])
        def __call__(self, x):
            for i in range(self.n_inference_layers):
                x = dense(x, *self.inference_layer_vars[i])
            return x

        @tf.function(input_signature=[tf.TensorSpec(shape=[None, number_of_features], dtype=tf.float32)])
        def boost(self, x):
            for i in range(self.n_boost_layers):
                x = dense(x, *self.boost_layer_vars[i])
            return x

        @tf.function
        def loss(self, predicted_y, target_y, w):
            epsilon = 1e-5
            diff_from_truth = tf.where(target_y == 1., predicted_y, 1. - predicted_y)
            cross_entropy = - tf.reduce_sum(w * tf.math.log(diff_from_truth + epsilon)) / tf.reduce_sum(w)
            return cross_entropy

    state = State(model=my_model())
    return state


def partial_fit(state, X, S, y, w, epoch, batch):
    """
    Pass received data to tensorflow session
    """
    prior = Prior(S[:, 0], y[:, 0])
    N = 100
    batch_size = 100

    # make sure we have the epochs and batches set correctly.
    assert epoch < 2, "There should only be two iterations, one for the boost training,"\
        " one for the dplot training. Check the value of m_nIterations."
    assert batch == 0, "All data should be passed to partial_fit on each call."\
        " The mini batches are handled internally. Check that m_mini_batch_size=0."

    indices = np.arange(len(X))
    for i in range(N):
        np.random.shuffle(indices)
        for pos in range(0, len(indices), batch_size):
            if pos + batch_size >= len(indices):
                break
            index = indices[pos: pos + batch_size]
            z_batch = S[index, 0]
            x_batch = X[index]

            if epoch == 0:
                x_batch = np.r_[x_batch, x_batch]
                w_batch = prior.get_boost_weights(z_batch) * np.r_[w[index, 0], w[index, 0]]
                y_batch = np.r_[np.ones(batch_size), np.zeros(batch_size)]
                y_batch = np.reshape(y_batch, (-1, 1))
                optimizer = state.model.boost_optimizer
                name = 'boost'
            else:
                p_batch = state.model.boost(x_batch).numpy()
                w_batch = prior.get_uncorrelation_weights(z_batch, p_batch.flatten()) * w[index, 0]
                y_batch = y[index]
                optimizer = state.model.inference_optimizer
                name = 'inference'

            w_batch = np.reshape(w_batch, (-1, 1)).astype(np.float32)

            with tf.GradientTape() as tape:
                if epoch == 0:
                    y_predict_batch = state.model.boost(x_batch)
                else:
                    y_predict_batch = state.model(x_batch)

                avg_cost = state.model.loss(y_predict_batch, y_batch, w_batch)
                trainable_variables = [v for v in state.model.trainable_variables if name in v.name]
                grads = tape.gradient(avg_cost, trainable_variables)

            optimizer.apply_gradients(zip(grads, trainable_variables))

        print("Internal Epoch:", '%04d' % (i), "cost=", f"{avg_cost:.9f}")
    return True


if __name__ == "__main__":

    train_file = find_file("mva/train_D0toKpipi.root", "examples")
    training_data = basf2_mva.vector(train_file)

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    general_options.m_identifier = "TensorflowDPlot"
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
    specific_options.m_steering_file = 'mva/examples/tensorflow/dplot.py'
    specific_options.m_nIterations = 2  # Feed data twice (first time for boost training, second time for dplot training)
    specific_options.m_mini_batch_size = 0  # Pass all events each 'batch'
    basf2_mva.teacher(general_options, specific_options)
