#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

import numpy as np
import tensorflow as tf
import basf2_mva
import pandas

from basf2_mva_python_interface.tensorflow import State


class Prior(object):
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
        # NOT self.get_bckgrd_cdf() here, signal and background are handlet asymmetrical!
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

    tf.reset_default_graph()
    x = tf.placeholder(tf.float32, [None, number_of_features], name='x')
    y = tf.placeholder(tf.float32, [None, 1], name='y')
    w = tf.placeholder(tf.float32, [None, 1], name='w')

    def layer(x, shape, name, unit=tf.sigmoid):
        with tf.name_scope(name) as scope:
            weights = tf.Variable(tf.truncated_normal(shape, stddev=1.0 / np.sqrt(float(shape[0]))), name='weights')
            biases = tf.Variable(tf.constant(0.0, shape=[shape[1]]), name='biases')
            layer = unit(tf.matmul(x, weights) + biases)
        return layer

    # Boost network
    boost_hidden1 = layer(x, [number_of_features, 20], 'boost_hidden1')
    boost_hidden2 = layer(boost_hidden1, [20, 20], 'boost_hidden2')
    boost_hidden3 = layer(boost_hidden2, [20, 20], 'boost_hidden3')
    boost_hidden4 = layer(boost_hidden3, [20, 20], 'boost_hidden4')
    boost_activation = layer(boost_hidden4, [20, 1], 'boost_sigmoid', unit=tf.sigmoid)

    epsilon = 1e-5
    boost_loss = -tf.reduce_sum(y * w * tf.log(boost_activation + epsilon) +
                                (1.0 - y) * w * tf.log(1 - boost_activation + epsilon)) / tf.reduce_sum(w)

    boost_optimizer = tf.train.AdamOptimizer(learning_rate=0.01)
    boost_minimize = boost_optimizer.minimize(boost_loss)

    # Inference network
    inference_hidden1 = layer(x, [number_of_features, 20], 'inference_hidden1')
    inference_hidden2 = layer(inference_hidden1, [20, 20], 'inference_hidden2')
    inference_hidden3 = layer(inference_hidden2, [20, 20], 'inference_hidden3')
    inference_hidden4 = layer(inference_hidden3, [20, 20], 'inference_hidden4')
    inference_activation = layer(inference_hidden4, [20, 1], 'inference_sigmoid', unit=tf.sigmoid)

    epsilon = 1e-5
    inference_loss = -tf.reduce_sum(y * w * tf.log(inference_activation + epsilon) +
                                    (1.0 - y) * w * tf.log(1 - inference_activation + epsilon)) / tf.reduce_sum(w)

    inference_optimizer = tf.train.AdamOptimizer(learning_rate=0.01)
    inference_minimize = inference_optimizer.minimize(inference_loss)

    init = tf.global_variables_initializer()

    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    session = tf.Session(config=config)
    session.run(init)

    state = State(x, y, inference_activation, inference_loss, inference_minimize, session)
    state.boost_cost = boost_loss
    state.boost_optimizer = boost_minimize
    state.boost_activation = inference_activation
    state.w = w
    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Pass received data to tensorflow session
    """
    prior = Prior(S[:, 0], y[:, 0])
    N = 100
    batch_size = 100

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
                optimizer = state.boost_optimizer
                cost = state.boost_cost
            else:
                p_batch = state.session.run(state.boost_activation, feed_dict={state.x: x_batch})
                w_batch = prior.get_uncorrelation_weights(z_batch, p_batch.flatten()) * w[index, 0]
                y_batch = y[index]
                optimizer = state.optimizer
                cost = state.cost

            w_batch = np.reshape(w_batch, (-1, 1))
            feed_dict = {state.x: x_batch, state.y: y_batch, state.w: w_batch}
            state.session.run(optimizer, feed_dict=feed_dict)
        avg_cost = state.session.run(cost, feed_dict=feed_dict)
        print("Epoch:", '%04d' % (i), "cost=", "{:.9f}".format(avg_cost))
    return True


if __name__ == "__main__":

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
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
                 'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)', 'daughterInvariantMass(1, 2)']
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_spectators = basf2_mva.vector('M')
    general_options.m_target_variable = "isSignal"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "tensorflow"
    specific_options.m_steering_file = 'mva/examples/tensorflow_dplot.py'
    specific_options.m_nIterations = 2  # Feed data twice (first time for boost training, second time for dplot training)
    specific_options.m_mini_batch_size = 0
    basf2_mva.teacher(general_options, specific_options)
