#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Dennis Weyland 2017

from basf2_mva_python_interface.contrib_keras import State
import h5py

import tensorflow as tf
import tensorflow.contrib.keras as keras

from keras.layers import Input, Dense, Concatenate, Flatten, Dropout
from keras.layers.merge import Average
from keras.layers.core import Reshape
from keras.layers import activations
from keras.models import Model, load_model
from keras.optimizers import adam
from keras.losses import binary_crossentropy, mean_squared_error
from keras.activations import sigmoid, tanh
from keras.engine.topology import Layer
from keras import backend as K
from keras.callbacks import Callback, EarlyStopping
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


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Building keras model
    """

    input = Input(shape=(number_of_features,))
    net = input

    if parameters['use_relations']:
        net = Relations(number_groups=parameters['number_groups'], number_features=parameters['number_features'])(net)
        net = RelationsAverage()(net)
    else:
        for i in range(6):
            net = Dense(units=2 * number_of_features, activation=tanh)(net)

    output = Dense(units=1, activation=sigmoid)(net)

    state = State(Model(input, output), custom_objects={'Relations': Relations, 'RelationsAverage': RelationsAverage})

    state.model.compile(optimizer=adam(lr=0.001), loss=binary_crossentropy, metrics=['accuracy'])
    state.model.summary()

    return state


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """
    Returns just the state object
    """
    state.Xtest = Xtest
    state.ytest = ytest

    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Do the fit
    """
    class TestCallback(Callback):
        """
        Print small summary.
        Class has to inherit from abstract Callback class
        """

        def on_epoch_end(self, epoch, logs={}):
            """
            Print summary at the end of epoch.
            For other possibilities look at the abstract Callback class.
            """
            loss, acc = state.model.evaluate(state.Xtest, state.ytest, verbose=0, batch_size=1000)
            loss2, acc2 = state.model.evaluate(X[:10000], y[:10000], verbose=0, batch_size=1000)
            print('\nTesting loss: {}, acc: {}'.format(loss, acc))
            print('Training loss: {}, acc: {}'.format(loss2, acc2))

    state.model.fit(X, y, batch_size=100, epochs=100, validation_data=(state.Xtest, state.ytest),
                    callbacks=[TestCallback(), EarlyStopping(monitor='val_loss')])
    return False


if __name__ == "__main__":
    import os
    import pandas
    from root_pandas import to_root
    import tempfile
    import json
    import numpy as np

    import basf2
    import basf2_mva
    import basf2_mva_util

    # ##############Building Data samples ###########################
    # This is  a dataset for testing relational nets.
    # It consists of number_total_lines lines in 3 dimensional space.
    # Each line has 6 variables.
    # In apprx. half of the cases, two lines are hitting each other.
    # This is considered a signal event.
    # Training results differs from the number of total lines.

    variables = []
    # try using 10 and 20 lines and see what happens
    number_total_lines = 5
    # Names for the training data set
    for i in range(number_total_lines):
        variables += ['px_' + str(i), 'py_' + str(i), 'pz_' + str(i), 'dx_' + str(i), 'dy_' + str(i),
                      'dz_' + str(i)]
    # Number of events in training and test root file.
    number_of_events = 1000000

    def build_signal_event():
        """Building two lines which are hitting each other"""
        p_vec1, p_vec2 = np.random.normal(size=3), np.random.normal(size=3)
        v_cross = np.random.normal(size=3)
        epsilon1, epsilon2 = (np.random.rand() * 2 - 1) / 10, (np.random.rand() * 2 - 1) / 10
        v_vec1 = v_cross + (p_vec1 * epsilon1)
        v_vec2 = v_cross + (p_vec2 * epsilon2)
        return np.concatenate([p_vec1, v_vec1]), np.concatenate([p_vec2, v_vec2])

    # This path will delete itself with all data in it after end of program.
    with tempfile.TemporaryDirectory() as path:
        for filename in ['train.root', 'test.root']:
            print('Building ' + filename)
            # Use random numbers to build all training and spectator variables.
            data = np.random.normal(size=[number_of_events, number_total_lines * 6])
            target = np.zeros([number_of_events], dtype=bool)

            # Overwrite for half of the variables some lines so that they are hitting each other.
            # Write them also at the end for the spectators.
            for index, sample in enumerate(data):
                if np.random.rand() > 0.5:
                    target[index] = True
                    i1, i2 = int(np.random.rand() * number_total_lines), int(np.random.rand() * (number_total_lines - 1))
                    i2 = (i1 + i2) % number_total_lines
                    track1, track2 = build_signal_event()
                    data[index, i1 * 6:(i1 + 1) * 6] = track1
                    data[index, i2 * 6:(i2 + 1) * 6] = track2

            # Saving all variables in root files
            dic = {}
            for i, name in enumerate(variables):
                dic.update({name: data[:, i]})
            dic.update({'isSignal': target})

            df = pandas.DataFrame(dic, dtype=np.float32)
            to_root(df, os.path.join(path, filename), tree_key='variables')

        # ##########################Do Training#################################
        # Do a comparison of different Nets for this task.

        general_options = basf2_mva.GeneralOptions()
        general_options.m_datafiles = basf2_mva.vector(os.path.join(path, 'train.root'))
        general_options.m_treename = "variables"
        general_options.m_variables = basf2_mva.vector(*variables)
        general_options.m_target_variable = "isSignal"

        specific_options = basf2_mva.PythonOptions()
        specific_options.m_framework = "contrib_keras"
        specific_options.m_steering_file = 'mva/examples/keras/relational_network.py'
        specific_options.m_training_fraction = 0.999

        # Train relational Net
        print('Train relational net ')
        general_options.m_identifier = os.path.join(path, 'relation.xml')
        specific_options.m_config = json.dumps({'use_relations': True,
                                                'number_groups': number_total_lines, 'number_features': 3})
        basf2_mva.teacher(general_options, specific_options)

        # Train normal feed forward Net:
        print('Train feed forward net')
        general_options.m_identifier = os.path.join(path, 'feed_forward.xml')
        specific_options.m_config = json.dumps({'use_relations': False})
        basf2_mva.teacher(general_options, specific_options)

        # ########################Compare Results####################################
        method1 = basf2_mva_util.Method(os.path.join(path, 'relation.xml'))
        method2 = basf2_mva_util.Method(os.path.join(path, 'feed_forward.xml'))

        test_data = basf2_mva.vector(os.path.join(path, 'test.root'))

        print('Apply relational net')
        p1, t1 = method1.apply_expert(test_data, general_options.m_treename)
        print('Apply feed forward net')
        p2, t2 = method2.apply_expert(test_data, general_options.m_treename)

        print('Reltional Net AUC: ', basf2_mva_util.calculate_roc_auc(p1, t1))
        print('Feed Forward Net AUC: ', basf2_mva_util.calculate_roc_auc(p2, t2))
