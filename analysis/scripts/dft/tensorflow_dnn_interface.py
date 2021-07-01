#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import os
import json
import tempfile
import numpy as np
import tensorflow as tf
import pandas

# was still important for some shared libraries at some point

from basf2_mva_python_interface.tensorflow import State

from dft import binning

from dft import tensorflow_dnn_model as tfm
from dft.TfData import TfDataBasf2, TfDataBasf2Stub


def get_tensorflow_model(number_of_features, parameters):
    """
    generates the tensorflow model
    :param number_of_features: int, number of features is handled separately
    :param parameters:
    :return:
    """

    layers = parameters.get('layers', None)
    wd_coeffs = parameters.get('wd_coeffs', [])

    lr_dec_rate = parameters.get('lr_dec_rate', 1 / (1 + 2e-7)**1.2e5)
    lr_init = parameters.get('lr_init', .05)
    mom_init = parameters.get('mom_init', .9)
    min_epochs = parameters.get('min_epochs', 300)
    max_epochs = parameters.get('max_epochs', 400)
    stop_epochs = parameters.get('stop_epochs', 10)

    if layers is None:
        layers = [['h0', 'tanh', number_of_features, 300, .0001, 1.0 / np.sqrt(300)],
                  ['h1', 'tanh', 300, 300, .0001, 1.0 / np.sqrt(300)],
                  ['h2', 'tanh', 300, 300, .0001, 1.0 / np.sqrt(300)],
                  ['h3', 'tanh', 300, 300, .0001, 1.0 / np.sqrt(300)],
                  ['h4', 'tanh', 300, 300, .0001, 1.0 / np.sqrt(300)],
                  ['h5', 'tanh', 300, 300, .0001, 1.0 / np.sqrt(300)],
                  ['h6', 'tanh', 300, 300, .0001, 1.0 / np.sqrt(300)],
                  ['h7', 'tanh', 300, 300, .0001, 1.0 / np.sqrt(300)],
                  ['y', 'sigmoid', 300, 1, .0001, 0.002 * 1.0 / np.sqrt(300)]]
    else:
        layers[0][2] = number_of_features

    # None disables usage of wd_coeffs
    if wd_coeffs is not None and not wd_coeffs:
        wd_coeffs = [2e-5 for _ in layers]

    mlp = tfm.MultilayerPerceptron.from_list(layers)
    model = tfm.DefaultModel(mlp, lr_dec_rate=lr_dec_rate, lr_init=lr_init, mom_init=mom_init, wd_coeffs=wd_coeffs,
                             min_epochs=min_epochs, max_epochs=max_epochs, stop_epochs=stop_epochs)

    return model


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    specifies the and configures the tensorflow model
    :param number_of_features:
    :param number_of_spectators:
    :param number_of_events:
    :param training_fraction:
    :param parameters: as dictionary encoded json object
    :return: State obj
    """

    # get all parameters, if they are not available, use default values
    if parameters is None:
        parameters = {}
    else:
        if not isinstance(parameters, dict):
            raise TypeError('parameters must be a dictionary')

    cuda_mask = parameters.get('cuda_visible_devices', '3')
    tensorboard_dir = parameters.get('tensorboard_dir', None)

    batch_size = parameters.get('batch_size', 100)
    seed = parameters.get('seed', None)

    # postprocessing parameters, from dictionary
    transform_to_probability = parameters.get('transform_to_probability', False)

    # initialize session
    tf.reset_default_graph()
    gpu_options = tf.GPUOptions(allow_growth=True)

    # set random state
    if seed:
        print('Seed: ', seed)
        tf.set_random_seed(seed)

    # mask cuda devices
    os.environ['CUDA_VISIBLE_DEVICES'] = cuda_mask
    session = tf.Session(config=tf.ConfigProto(gpu_options=gpu_options))

    # initialize model
    x = tf.placeholder(tf.float32, [None, number_of_features])
    y = tf.placeholder(tf.float32, [None, 1])

    # using a stub data set since there is no data available at this state
    stub_data_set = TfDataBasf2Stub(batch_size, number_of_features, number_of_events, training_fraction)

    # set saving file name, unfortunately this is already required in partial_fit
    save_dir = tempfile.TemporaryDirectory()
    save_name = os.path.join(save_dir.name, 'mymodel')

    model = get_tensorflow_model(number_of_features, parameters)
    training = tfm.Trainer(model, stub_data_set, session, tensorboard_dir, save_name, input_placeholders=[x, y])

    state = State(x, y, session=session)

    # training object is required in partial fit
    state.training = training
    state.batch_size = batch_size
    state.save_dir = save_dir

    state.transform_to_probability = transform_to_probability

    # save parameters
    saved_parameters = parameters.copy()
    saved_parameters['number_of_features'] = number_of_features
    state.parameters = json.dumps(saved_parameters)
    state.seed = seed
    return state


def apply(state, X):
    """
    modified apply function
    """

    binning.transform_ndarray(X, state.binning_parameters)
    chunk_size = 1000000
    if len(X) > chunk_size:
        results = list()
        for i in range(0, len(X), chunk_size):
            results.append(state.session.run(state.activation, feed_dict={state.x: X[i: i + chunk_size]}))
        r = np.concatenate(results).flatten()
    else:
        r = state.session.run(state.activation, feed_dict={state.x: X}).flatten()
    if state.transform_to_probability:
        binning.transform_array_to_sf(r, state.sig_back_tuple, signal_fraction=.5)

    return np.require(r, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


def load(obj):
    """
    Load Tensorflow estimator into state
    """
    # tensorflow operations
    tf.reset_default_graph()
    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    session = tf.Session(config=config)

    parameters = json.loads(obj[0])

    number_of_features = parameters.pop('number_of_features')

    x = tf.placeholder(tf.float32, [None, number_of_features])
    y = tf.placeholder(tf.float32, [None, 1])

    class DataStub:
        """
        simple stub obj
        """
        feature_number = number_of_features
        batches = 1

    model = get_tensorflow_model(number_of_features, parameters)
    model.initialize(DataStub(), [x, y])
    saver = tf.train.Saver()

    # tensorflow is a moving target, file loading and saving of mid-level api changes rapidly. so we use the legacy here
    with tempfile.TemporaryDirectory() as path:
        with open(os.path.join(path, obj[1] + '.data-00000-of-00001'), 'w+b') as file1, open(
                os.path.join(path, obj[1] + '.index'), 'w+b') as file2:
            file1.write(bytes(obj[2]))
            file2.write(bytes(obj[3]))
        tf.train.update_checkpoint_state(path, obj[1])
        saver.restore(session, os.path.join(path, obj[1]))

    # load and initialize required objects
    state = State(x, y, session=session)
    state.activation = model.mlp.output

    # preprocessing parameters
    state.binning_parameters = obj[4]

    # postprocessing transform to probability, if pdf was sampled during training
    state.transform_to_probability = obj[5]
    state.sig_back_tuple = obj[6]

    seed = obj[7]
    print('Deep FlavorTagger loading... Training seed: ', seed)

    return state


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """
    use test sets for monitoring
    """
    # TODO: split this set to define an independent test set for transformations to probability

    state.Xvalid = Xtest[:len(Xtest) // 2]
    state.yvalid = ytest[:len(ytest) // 2]

    state.Xtest = Xtest[len(Xtest) // 2:]
    state.ytest = ytest[len(ytest) // 2:]

    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    returns fractions of training and testing dataset, also uses weights
    :param X: unprocessed training dataset
    :param Xtest: unprocessed validation dataset
    :return: bool, True == continue, False == stop iterations
    """

    # training is performed in a single epoch
    if epoch > 0:
        raise RuntimeError

    # preprocessing
    state.binning_parameters = binning.get_ndarray_binning_parameters(X)

    binning.transform_ndarray(X, state.binning_parameters)
    binning.transform_ndarray(state.Xvalid, state.binning_parameters)

    if np.all(np.isnan(X)):
        raise ValueError('NaN values in Dataset. Preprocessing transformations failed.')

    # replace stub dataset
    data_set = TfDataBasf2(X, y, state.Xvalid, state.yvalid, state.batch_size, seed=state.seed)

    state.training.data_set = data_set

    # start training
    state.training.train_model()

    return False


def end_fit(state):
    """
    save the trained model
    :param state:
    :return:
    """
    filename = state.training.save_name
    with open(filename + '.data-00000-of-00001', 'rb') as file1, open(filename + '.index', 'rb') as file2:
        data1 = file1.read()
        data2 = file2.read()
    binning_parameters = state.binning_parameters

    # transform to probability has to be saved since state object has to return untransformed network output
    transform_to_probability = state.transform_to_probability
    state.transform_to_probability = False

    # sample pdfs of trained model on test_dataset, return test df
    state.get_from_collection()
    y_hat = state(*state.Xtest)
    test_df = pandas.DataFrame.from_dict({'y': state.ytest.reshape(-1), 'y_hat': y_hat.reshape(-1)})
    (sig_pdf, back_pdf) = binning.get_signal_background_pdf(test_df)
    seed = state.seed
    parameters = state.parameters
    del state
    return [parameters, os.path.basename(filename), data1, data2, binning_parameters, transform_to_probability,
            (sig_pdf, back_pdf), seed]
