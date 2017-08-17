#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Dennis Weyland 2017

# This example shows how to convert a model trained with keras inside a basf2 weightfile.

from basf2_mva_python_interface.contrib_keras import State

import tensorflow as tf
import tensorflow.contrib.keras as keras

from keras.models import load_model


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Just load keras model into state
    """
    return State(load_model(parameters['file_path']))


def partial_fit(state, X, S, y, w, epoch):
    """
    This should be empty, because our model is already fitted.
    """
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

    from keras.layers import Input, Dense
    from keras.models import Model
    from keras.optimizers import adam
    from keras.losses import binary_crossentropy
    from keras.activations import sigmoid, tanh

    # ##############Building Data samples ###########################
    # The training needs the same variables in the same orders as keras received them during training.
    # For this example we just create some random variables.

    variables = ['x' + str(i) for i in range(10)]

    data = np.random.normal(size=[1000, 11])
    data[:, -1] = np.int32(data[:, -1] > 0.5)

    # Build some simple model to convert into weightfile
    input = Input(shape=(10,))

    net = Dense(units=100, activation=tanh)(input)
    output = Dense(units=1, activation=sigmoid)(net)

    model = Model(input, output)
    model.compile(optimizer=adam(lr=0.01), loss=binary_crossentropy, metrics=['accuracy'])

    model.fit(data[:, :-1], data[:, -1], batch_size=10, epochs=10)

    # This path will delete itself with all data in it after end of program.
    with tempfile.TemporaryDirectory() as path:
        # Saving all variables in root files
        dic = {}
        for i, name in enumerate(variables):
            dic.update({name: data[:, i]})
        dic.update({'isSignal': data[:, -1]})

        df = pandas.DataFrame(dic, dtype=np.float32)
        to_root(df, os.path.join(path, 'data.root'), tree_key='tree')

        # Saving keras training model
        model.save(os.path.join(path, 'weights.h5'))

        # ##########################Do Conversion#################################

        general_options = basf2_mva.GeneralOptions()
        general_options.m_datafiles = basf2_mva.vector(os.path.join(path, 'data.root'))
        general_options.m_treename = "tree"
        general_options.m_variables = basf2_mva.vector(*variables)
        general_options.m_target_variable = "isSignal"

        specific_options = basf2_mva.PythonOptions()
        specific_options.m_framework = "contrib_keras"
        specific_options.m_steering_file = 'mva/examples/keras/keras_to_weightfile.py'

        general_options.m_identifier = 'converted_keras'
        specific_options.m_config = json.dumps({'file_path': os.path.join(path, 'weights.h5')})
        basf2_mva.teacher(general_options, specific_options)

        # ########################Apply weightfile####################################
        method = basf2_mva_util.Method(general_options.m_identifier)
        p, t = method.apply_expert(general_options.m_datafiles, general_options.m_treename)
        print('Overtraining AUC: ', basf2_mva_util.calculate_roc_auc(p, t))
