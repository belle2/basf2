#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Dennis Weyland 2017

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
    To sum over all permutations please use GlobalAveragePooling1D from keras.
    """

    def __init__(self, number_features, hidden_feature_shape=[30, 30, 30, 30], activation=tanh, **kwargs):
        """
        Init class.
        """

        #: Number of features. Number of different shared weights used for comparison for each relation.
        self.number_features = number_features
        #: Number of groups in input.
        self.number_groups = 0
        #: shape of hidden layers used for extracting relations
        self.hidden_feature_shape = hidden_feature_shape
        #: activation used for hidden layer in shared weights. For output sigmoid will always be used.
        self.activation = activations.get(activation)
        #: how many neurons has one comparable object
        self.group_len = 0
        #: saves weights for call
        self.variables = []
        #: number of relation combinations
        self.combinations = 0

        super(Relations, self).__init__(**kwargs)

    def build(self, input_shape):
        """
        Build all weights for Relations Layer
        :param input_shape: Input shape of tensor
        :return:  Nothing
        """
        # only accept 2D layers
        assert(len(input_shape) == 3)

        self.number_groups = input_shape[1]

        self.group_len = input_shape[2]

        self.combinations = np.int32(np.math.factorial(self.number_groups) / (2 * np.math.factorial(self.number_groups - 2)))

        dense_shape = [2 * self.group_len] + self.hidden_feature_shape + [self.number_features]

        for i in range(len(dense_shape[:-1])):
            weights = self.add_weight(name='relation_weights_{}'.format(i),
                                      shape=list(dense_shape[i:i + 2]), initializer='glorot_uniform', trainable=True)
            bias = self.add_weight(name='relation_weights_{}'.format(i),
                                   shape=(dense_shape[i + 1],), initializer='zeros', trainable=True)

            self.variables.append([weights, bias])

        super(Relations, self).build(input_shape)

    def call(self, inputs):
        """
        Compute Relational Layer
        :param inputs: input tensor
        :return: output tensor
        """
        input_groups = [inputs[:, i, :] for i in range(self.number_groups)]
        outputs = []
        for index, group1 in enumerate(input_groups[:-1]):
            for group2 in input_groups[index + 1:]:
                net = K.dot(K.concatenate([group1, group2]), self.variables[0][0])
                net = K.bias_add(net, self.variables[0][1])
                for variables in self.variables[1:]:
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
        # only 2D layers
        assert(len(input_shape) == 3)

        self.combinations = np.int32(np.math.factorial(self.number_groups) / (2 * np.math.factorial(self.number_groups - 2)))

        return (input_shape[0], self.combinations, self.number_features)

    def get_config(self):
        """
        Config required for saving parameters in keras model.
        """
        config = {
            'number_features': self.number_features,
            'hidden_feature_shape': self.hidden_feature_shape,
            'activation': activations.serialize(self.activation)
        }
        base_config = super(Relations, self).get_config()
        return dict(list(base_config.items()) + list(config.items()))


class EnhancedRelations(Layer):
    """
    This is a class which implements Relational Layer into Keras.
    See Class Relations for details.
    EnhanceRelations use an additional input for passing event information to every comparison:
    RN(O) = f_phi(sum_phi(g_theta(o_i,o_j,q)))
    q is fed in as second one dimensional input.
    """

    def __init__(self, number_features, hidden_feature_shape=[30, 30, 30, 30], activation=tanh, **kwargs):
        """
        Init class.
        """

        #: Number of features. Number of different shared weights used for comparison for each relation.
        self.number_features = number_features
        #: Number of groups in input.
        self.number_groups = 0
        #: shape of hidden layers used for extracting relations
        self.hidden_feature_shape = hidden_feature_shape
        #: activation used for hidden layer in shared weights. For output sigmoid will always be used.
        self.activation = activations.get(activation)
        #: how many neurons has one comparable object
        self.group_len = 0
        #: saves weights for call
        self.variables = []
        #: number of relation combinations
        self.combinations = 0
        #: size of second input vector
        self.question_len = 0

        super(EnhancedRelations, self).__init__(**kwargs)

    def build(self, input_shape):
        """
        Build all weights for Relations Layer
        :param input_shape: Input shape of tensor
        :return:  Nothing
        """
        # accept only 2 inputs
        assert(len(input_shape) == 2)
        # first input should be a 2D layers
        assert(len(input_shape[0]) == 3)
        # second input should be a 1D layers
        assert(len(input_shape[1]) == 2)

        self.number_groups = input_shape[0][1]

        self.group_len = input_shape[0][2]

        self.question_len = input_shape[1][1]

        self.combinations = np.int32(np.math.factorial(self.number_groups) / (2 * np.math.factorial(self.number_groups - 2)))

        dense_shape = [2 * self.group_len + self.question_len] + self.hidden_feature_shape + [self.number_features]

        for i in range(len(dense_shape[:-1])):
            weights = self.add_weight(name='relation_weights_{}'.format(i),
                                      shape=list(dense_shape[i:i + 2]), initializer='glorot_uniform', trainable=True)
            bias = self.add_weight(name='relation_weights_{}'.format(i),
                                   shape=(dense_shape[i + 1],), initializer='zeros', trainable=True)

            self.variables.append([weights, bias])

        super(EnhancedRelations, self).build(input_shape)

    def call(self, inputs):
        """
        Compute Relational Layer
        :param inputs: input tensor
        :return: output tensor
        """
        input_groups = [inputs[0][:, i, :] for i in range(self.number_groups)]
        questions = inputs[1]
        outputs = []
        for index, group1 in enumerate(input_groups[:-1]):
            for group2 in input_groups[index + 1:]:
                net = K.dot(K.concatenate([group1, group2, questions]), self.variables[0][0])
                net = K.bias_add(net, self.variables[0][1])
                for variables in self.variables[1:]:
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
        # accept only 2 inputs
        assert(len(input_shape) == 2)
        # first input should be a 2D layers
        assert(len(input_shape[0]) == 3)
        # second input should be a 1D layers
        assert(len(input_shape[1]) == 2)

        self.combinations = np.int32(np.math.factorial(self.number_groups) / (2 * np.math.factorial(self.number_groups - 2)))

        return (input_shape[0][0], self.combinations, self.number_features)

    def get_config(self):
        """
        Config required for saving parameters in keras model.
        """
        config = {
            'number_features': self.number_features,
            'hidden_feature_shape': self.hidden_feature_shape,
            'activation': activations.serialize(self.activation)
        }
        base_config = super(EnhancedRelations, self).get_config()
        return dict(list(base_config.items()) + list(config.items()))
