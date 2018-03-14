#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Dennis Weyland 2017

# This example serves as a basic example of implementing Relational networks into basf2 with tensorflow.
# As a toy example it will try to tell if 2 out of multiple lines are hitting each other in three dimensional space.
# Relevant Paper: https://arxiv.org/abs/1706.01427
# If you want to try out relational networks to your problem, feel free to import the two classes in your code.

from basf2_mva_python_interface.contrib_keras import State
import h5py

import tensorflow as tf
import tensorflow.contrib.keras as keras

from keras.layers import Input, Dense, Concatenate, Flatten, Dropout, GlobalAveragePooling1D
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

from basf2_mva_extensions.keras_relational import Relations


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Building keras model
    """

    input = Input(shape=(number_of_features,))
    net = input

    if parameters['use_relations']:
        net = Reshape((number_of_features // 6, 6))(net)
        net = Relations(number_features=parameters['number_features'])(net)
        # average over all permutations
        net = GlobalAveragePooling1D()(net)
    else:
        for i in range(6):
            net = Dense(units=2 * number_of_features, activation=tanh)(net)

    output = Dense(units=1, activation=sigmoid)(net)

    state = State(Model(input, output), custom_objects={'Relations': Relations})

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
            to_root(df, os.path.join(path, filename), key='variables')

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
                                                'number_features': 3})
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

        print('Relational Net AUC: ', basf2_mva_util.calculate_roc_auc(p1, t1))
        print('Feed Forward Net AUC: ', basf2_mva_util.calculate_roc_auc(p2, t2))
