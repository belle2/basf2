#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Jochen Gemmler 2016

import theano as th
import numpy as np
import tempfile
import pickle
import os
import pandas

try:
    from pylearn2.datasets.dense_design_matrix import DenseDesignMatrix
    # also the network model depends on theano
    from dft import networkModel_default as nm
except(ImportError):
    print('WARNING: pylearn2 is not available, expert will still be usable')

from dft import binning
from dft import independent_training


class State(object):
    """
    State class for proper handling of parameters and data during function calls.
    """

    def __init__(self, theano_function=None, flatten_parameters=None, sig_back_pdfs=None, signal_fraction=None):
        """
        Constructor of the State class
        """
        #: theano function
        self.theano_function = theano_function
        #: parameters used for flattening
        self.flatten_parameters = flatten_parameters
        #: tuple, signal and background pdfs, used for transformation to probability
        self.sig_back_pdfs = sig_back_pdfs
        #: signal fraction override
        self.signal_fraction = signal_fraction

        # used during training
        #: bool, transform to probability
        self.transform_to_prob = None
        #: bool, convert pickled theano gpu function to cpu
        self.convert_to_cpu = None
        #: dictionary, parameters
        self.parameters = None
        #: temporary weightfile from tempfile
        self.temporary_weightfile = None

        # check if pdfs should be sampled from validation set
        #: this automatically activates the transform to probability
        self.sample_pdfs = None

        #: data vector X
        self.X = None
        #: target vector y
        self.y = None
        #: validation vector X
        self.Xvalid = None
        #: validation vector y
        self.Yvalid = None

    # TODO: make serializable (get state, set state) or better use builtin serialization
    # TODO: eg. xgboost, ...


def get_model(number_of_features, number_of_events, parameters):
    state = State()

    # intialize the dictionary
    if parameters is None:
        parameters = {}
    else:
        if not isinstance(parameters, dict):
            raise TypeError('parameters must be a dictionary')

    parameters['feature_number'] = int(number_of_features)

    state.temporary_weightfile = tempfile.NamedTemporaryFile(delete=False)
    state.temporary_weightfile.close()

    # set network output to a temporary file which will be delete in endfit
    parameters['save_path'] = state.temporary_weightfile.name

    state.transform_to_prob = parameters.pop('transform_to_prob', False)
    state.convert_to_cpu = parameters.pop('convert_to_cpu', False)

    state.parameters = parameters
    return state


def feature_importance(state):
    """
    Return a list containing the feature importances
    """
    return []


def load(obj):
    return State(*obj)


def apply(state, X):
    binning.transform_ndarray(X, state.flatten_parameters)  # 11,3ms (1000 events)
    # binning.transform_variable_vector(X.reshape(-1), state.flatten_parameters)  # 7ms (1000 events), 6.56ms (opt)
    # X = np.nan_to_num(X) # 2.7ms (1000 events), 2.0ms (opt)

    # return np.asarray([[.1]]) # 1.25ms (debug) .7ms (opt)
    if state.sig_back_pdfs is None:
        return state.theano_function(X)
    else:
        return binning.transform_array_to_probability_sf(
            state.theano_function(X),
            state.sig_back_pdfs,
            state.signal_fraction
        )


def begin_fit(state):
    state.X = []
    state.y = []
    state.Xvalid = []
    state.yvalid = []
    return state


def partial_fit(state, X, y, w, Xvalid, yvalid, wvalid, epoch):
    if len(np.unique(w)) != 1:
        raise NotImplementedError('Weighted files for pylearn2 training are not implemented.')
    state.X.append(X)
    state.y.append(y)
    state.Xvalid.append(Xvalid)
    state.yvalid.append(yvalid)
    return True


def end_fit(state):
    # transform all lists to a numpy array
    state.X = np.vstack(state.X)

    state.y = np.vstack(state.y).reshape(-1, 1)
    state.Xvalid = np.vstack(state.Xvalid)
    state.yvalid = np.vstack(state.yvalid).reshape(-1, 1)

    # get flatten parameters (only uses trainings set which should be ok)
    b_params = binning.get_ndarray_binning_parameters(state.X)

    # transform variables
    binning.transform_ndarray(state.X, b_params)
    binning.transform_ndarray(state.Xvalid, b_params)

    limit = 0

    # initialize network model
    ann = nm.init_train(DenseDesignMatrix(X=state.X, y=state.y),
                        DenseDesignMatrix(X=state.Xvalid[limit:], y=state.yvalid[limit:]), **state.parameters)

    # start training
    ann.main_loop()

    # delete temporary trained file
    os.unlink(state.temporary_weightfile.name)

    # save the trained model
    f_model = ann.model
    th_x = f_model.get_input_space().make_theano_batch()
    th_y = f_model.fprop(th_x)
    th_f = th.function([th_x], th_y)

    sig_back_pdfs = None

    if state.transform_to_prob:
        limit = int(len(state.yvalid) / 2)
        test_X = state.Xvalid[:limit]
        test_y = state.yvalid[:limit]
        y_hat = th_f(test_X)

        test_df = pandas.DataFrame.from_dict({'y': test_y.reshape(-1), 'y_hat': y_hat.reshape(-1)})
        signal_fraction = binning.get_signal_fraction(test_y)
        print('Signal fraction: %0.5f' % signal_fraction)
        # do not override state signal fraction (since this is  already an override)
        if state.signal_fraction is None:
            state.signal_fraction = signal_fraction

        sig_back_pdfs = binning.get_signal_background_pdf(test_df, bins=100)

    if state.convert_to_cpu:
        with open(state.temporary_weightfile.name, 'wb') as f:
            pickle.dump(f_model, f)
        independent_training.convert_gpu_to_cpu_model(state.temporary_weightfile.name)
        with open(state.temporary_weightfile.name, 'rb') as f:
            th_f = pickle.load(f)

    # pickle the theano function, flatten parameters, signal/background pdfs, signal_fraction

    return [th_f, b_params, sig_back_pdfs, state.signal_fraction]
