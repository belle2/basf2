#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Dennis Weyland 2017

import tensorflow as tf
import tensorflow.contrib.keras as keras

from keras.layers.merge import Average
from keras.layers.core import Reshape
from keras.layers import activations
from keras.activations import sigmoid, tanh
from keras.engine.topology import Layer
from keras import backend as K
import numpy as np


class Relations(Layer):
    """
    This is a class which implements Relational Layer into Keras.
    Relational Layer compares every combination of two feature groups with shared weights.
    Use this class as every other Layer in Keras.
    Relevant Paper: https://arxiv.org/abs/1706.01427
    RN(O) = f_phi(sum_phi(g_theta(o_i,o_j)))
    For flexibility reason only the part g(o_i,o_j) is modelled
    f_phi corresponds to a MLP net
    To sum over the output see RevelationsAverage
    """

    def __init__(self, number_groups, number_features, hidden_feature_shape=[30, 30, 30, 30], activation=tanh, **kwargs):
        """
        Init class.
        """
        #: Number of groups in input. This layer will divide the input shape by number of groups for comparison.
        self.number_groups = number_groups
        #: Number of features. Number of different shared weights used for comparison for each relation.
        self.number_features = number_features
        #: shape of hidden layers used for extracting relations
        self.hidden_feature_shape = hidden_feature_shape
        #: activation used for hidden layer in shared weights. For output sigmoid will always be used.
        self.activation = activations.get(activation)
        #: how many neurons has one comparable object
        self.group_len = 0
        #: saves weights for call
        self.variables = []
        #: number of relation combinations
        self.combinations = np.int32(np.math.factorial(self.number_groups) / (2 * np.math.factorial(self.number_groups - 2)))

        super(Relations, self).__init__(**kwargs)

    def build(self, input_shape):
        """
        Build all weights for Relations Layer
        :param input_shape: Input shape of tensor
        :return:  Nothing
        """
        # only accept flat layers
        assert(len(input_shape) == 2)
        # every neuron which goes into the raltional layer has to be used
        assert(input_shape[1] % self.number_groups == 0)

        self.group_len = input_shape[1] // self.number_groups

        dense_shape = [2 * self.group_len] + self.hidden_feature_shape + [1]

        for feature_number in range(self.number_features):
            weight_array = []
            for i in range(len(dense_shape[:-1])):
                weights = self.add_weight(name='relation_weights_{}_{}'.format(feature_number, i),
                                          shape=list(dense_shape[i:i + 2]), initializer='glorot_uniform', trainable=True)
                bias = self.add_weight(name='relation_weights_{}_{}'.format(feature_number, i),
                                       shape=(dense_shape[i + 1],), initializer='zeros', trainable=True)

                weight_array.append([weights, bias])

            self.variables.append(weight_array)

        super(Relations, self).build(input_shape)

    def call(self, inputs):
        """
        Compute Relational Layer
        :param inputs: input tensor
        :return: output tensor
        """
        input_groups = [inputs[:, i * self.group_len:(i + 1) * self.group_len] for i in range(self.number_groups)]
        outputs = []
        for feature in range(self.number_features):
            for index, group1 in enumerate(input_groups[:-1]):
                for group2 in input_groups[index + 1:]:
                    net = K.dot(K.concatenate([group1, group2]), self.variables[feature][0][0])
                    net = K.bias_add(net, self.variables[feature][0][1])
                    for variables in self.variables[feature][1:]:
                        net = self.activation(net)
                        net = K.dot(net, variables[0])
                        net = K.bias_add(net, variables[1])
                    outputs.append(sigmoid(net))

        flat_result = K.concatenate(outputs)
        return Reshape((self.combinations, self.number_features,))(flat_result)

    def compute_output_shape(self, input_shape):
        """
        Compute Output shape
        :return: Output shape
        """
        # only accept flat layers
        assert(len(input_shape) == 2)
        # every neuron which goes into the raltional layer has to be used
        assert(input_shape[1] % self.number_groups == 0)

        return (input_shape[0], self.combinations, self.number_features)

    def get_config(self):
        """
        Config required for saving parameters in keras model.
        """
        config = {
            'number_groups': self.number_groups,
            'number_features': self.number_features,
            'hidden_feature_shape': self.hidden_feature_shape,
            'activation': activations.serialize(self.activation)
        }
        base_config = super(Relations, self).get_config()
        return dict(list(base_config.items()) + list(config.items()))


class RelationsAverage(Layer):
    """
    Follow up class for Relations.
    Relevant paper:
    Relevant Paper: https://arxiv.org/abs/1706.01427
    RN(O) = f_phi(sum_phi(g_theta(o_i,o_j)))
    Here only the sum_phi is implemented.
    """

    def __init__(self, **kwargs):
        """
        Init class.
        """
        #: Number of Relations
        self.number_relations = 0
        #: Number of features coming from Relation
        self.number_features = 0

        super(RelationsAverage, self).__init__(**kwargs)

    def build(self, input_shape):
        """
        Build all weights for Relations Layer
        :param input_shape: Input shape of tensor
        :return:  Nothing
        """
        # only accept 2D layers
        assert(len(input_shape) == 3)

        self.number_relations = input_shape[1]
        self.number_features = input_shape[2]

        super(RelationsAverage, self).build(input_shape)

    def call(self, inputs):
        """
        Compute Relational Layer
        :param inputs: input tensor
        :return: output tensor
        """
        feature_array = []
        for feature_number in range(self.number_features):
            feature_array.append(inputs[:, :, feature_number])

        return Average()(feature_array)

    def compute_output_shape(self, input_shape):
        """
        Compute Output shape
        :return: Output shape
        """
        # only accept 2D layers
        assert(len(input_shape) == 3)

        return (input_shape[0], input_shape[1])

    def get_config(self):
        """
        Config required for saving parameters in keras model.
        """
        config = {}
        base_config = super(RelationsAverage, self).get_config()
        return dict(list(base_config.items()) + list(config.items()))
