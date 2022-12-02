#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This example shows how to convert a model trained with keras inside a basf2 weightfile.

from basf2_mva_python_interface.keras import State

from tensorflow.keras.models import load_model


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Just load keras model into state
    """
    return State(load_model(parameters['file_path']))


def partial_fit(state, X, S, y, w, epoch, batch):
    """
    This should be empty, because our model is already fitted.
    """
    return False


if __name__ == "__main__":
    import os
    import pandas
    import uproot
    import tempfile
    import json
    import numpy as np

    import basf2_mva
    import basf2_mva_util

    from tensorflow.keras.layers import Input, Dense
    from tensorflow.keras.models import Model
    from tensorflow.keras.optimizers import Adam
    from tensorflow.keras.losses import binary_crossentropy
    from tensorflow.keras.activations import sigmoid, tanh
    from basf2 import conditions
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    # ##############Building Data samples ###########################
    # The training needs the same variables in the same orders as keras received them during training.
    # For this example we just create some random variables.

    variables = ['x' + str(i) for i in range(10)]

    data = np.random.normal(size=[1000, 11])
    data[:, -1] = np.int32(data[:, -1] > 0.5)

    # Build some simple model as an example to convert into weightfile
    input = Input(shape=(10,))

    net = Dense(units=100, activation=tanh)(input)
    output = Dense(units=1, activation=sigmoid)(net)

    model = Model(input, output)
    model.compile(optimizer=Adam(lr=0.01), loss=binary_crossentropy, metrics=['accuracy'])

    model.fit(data[:, :-1], data[:, -1], batch_size=10, epochs=10)

    # This path will delete itself with all data in it after end of program.
    with tempfile.TemporaryDirectory() as path:
        # Saving all variables in root files
        dic = {}
        for i, name in enumerate(variables):
            dic.update({name: data[:, i]})
        dic.update({'isSignal': data[:, -1]})

        df = pandas.DataFrame(dic, dtype=np.float64)
        with uproot.recreate(os.path.join(path, 'data.root')) as outfile:
            outfile['tree'] = df

        # Saving keras training model
        model.save(os.path.join(path, 'example_existing_model'))

        # ##########################Do Conversion#################################

        general_options = basf2_mva.GeneralOptions()
        general_options.m_datafiles = basf2_mva.vector(os.path.join(path, 'data.root'))
        general_options.m_treename = "tree"
        general_options.m_variables = basf2_mva.vector(*variables)
        general_options.m_target_variable = "isSignal"

        specific_options = basf2_mva.PythonOptions()
        specific_options.m_framework = "keras"
        specific_options.m_steering_file = 'mva/examples/keras/import_existing_keras_model.py'

        general_options.m_identifier = 'converted_keras'
        specific_options.m_config = json.dumps({'file_path': os.path.join(path, 'example_existing_model')})
        basf2_mva.teacher(general_options, specific_options)

        # ########################Apply weightfile####################################
        method = basf2_mva_util.Method(general_options.m_identifier)
        p, t = method.apply_expert(general_options.m_datafiles, general_options.m_treename)
        print('Overtraining AUC: ', basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t))
