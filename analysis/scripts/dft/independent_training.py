#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Jochen Gemmler 2016

from __future__ import division, print_function, generators

import pickle
import os
import subprocess
import re

import theano as th

import numpy as np
import pandas

import shutil
import fileinput
import sys

from dft import networkModel_default
from dft import root_dataset as rd
from dft import binning
from dft.DeepFlavorTagger import makeROOTCompatible

import json


def write_out_json(network_args, train_params, data_path, weight_path, variable_names, network_model=None):
    """ create a json file for the trained model for easily access parameters

    :param default_args:
    :param train_params
    :param network_args:
    :param data_path:
    :param weight_path:
    :param network_model:
    :return:
    """
    if network_model is None:
        network_model = networkModel_default

    params = network_model.get_default_params()

    # params not to monitor
    params_no_monitor = network_model.get_not_json_params()

    # meta params for general arguments
    meta_params = dict()
    meta_params['model_name'] = network_model.__name__
    meta_params['data_file'] = data_path
    meta_params['weight_path'] = weight_path
    meta_params['variable_names'] = variable_names

    for param, val in network_args.items():
        if param not in params_no_monitor:
            params[param] = val

    save_path = weight_path.replace('weights.pkl', 'hparams.json')

    params = {'meta params': meta_params, 'train params': train_params, 'model params': params}

    json.dump(params, open(save_path, 'w'))


def get_mlp_training(dataset_train, dataset_valid, network_model=None, **kwargs):
    """ Prepare default network training.
    :param dataset_train: training dataset
    :param dataset_valid: validation dataset
    :param network_model: python module containing init_train which returns a pylearn.train.Train instance
    :param kwargs: arguements for mlp
    :return: pylearn.train.Train instance
    """

    if network_model is None:
        print('Using Model %s ' % networkModel_default)
        network_model = networkModel_default
    else:
        print('Using Model %s' % network_model)

    print('Modifications')
    for arg in kwargs:
        print(arg, kwargs[arg])

    train = network_model.init_train(dataset_train, dataset_valid, **kwargs)

    return train


def get_ignore_vars_std(ignore_variable='pValue', tracks=5):
    """ generates root compatible names for a given variable with respect of the python flavor tagging module
    :param ignore_variable: name of the variable to be ignored
    :param tracks: number of charged tracks used (per charge)
    :return: list with root compatible variable names to ignore when loading the root file into memory
    """
    ignore_list = []
    base_name1 = 'getVariableByRank__bopi__pl__clpos_charged__cm__spp__cm__sp'
    base_name2 = 'getVariableByRank__bopi__pl__clneg_charged__cm__spp__cm__sp'
    mid_name = '__cm__sp'
    end_name = '__bc'

    ignore_variable = makeROOTCompatible(ignore_variable)

    for i in range(1, tracks + 1):
        name1 = base_name1 + ignore_variable + mid_name + str(i) + end_name
        name2 = base_name2 + ignore_variable + mid_name + str(i) + end_name
        ignore_list.append(name1)
        ignore_list.append(name2)

    return ignore_list


def train_mlp(working_dir, prefix='standard', fraction=.92, target_var='qrCombined', ignore_vars=None,
              tracks=10, network_model=None, shuffle=False, start=None, stop=None, output_prefix=None,
              overwrite_trained=False, **mlp_args):
    """

    :param working_dir: working directory
    :param prefix: classifier prefix of the training file
    :param fraction: separate train, valid, test (test for transform to probability);
    train : (train+valid+test) = fraction
    valid : test = 1
    :param target_var:
    :param ignore_vars: variable manager variable name (not the acutually generated name in the root file)
    eg. pValue will have 10 corresponding names
    :param tracks: even number of maximum charged tracks
    :param network_model: python function that contains init_train which returns pylearn2.train
    :param start: defines on which event reading in is started
    :param stop: defines on which event reading in is stopped
    :param ouput_prefix: use this prefix if trained with different network settings, ignore_vars, ...
    will create specific .pkl files and copy the .xml configuration file
    :param mlp_args: arguments passed to the mlp
    :return: None
    """

    # train specific params for json documentation
    train_params = dict()
    train_params['fraction'] = fraction
    train_params['target_var'] = target_var
    train_params['shuffle'] = shuffle
    train_params['ignore_vars'] = ignore_vars
    train_params['tracks'] = tracks
    train_params['start'] = start
    train_params['stop'] = stop

    if start is not None or stop is not None:
        raise NotImplementedError

    # allows output_prefix of generated files to be different
    if output_prefix is None:
        output_prefix = prefix

    mlp_output = os.path.join(working_dir, output_prefix + '_weights.pkl')
    if os.path.isfile(mlp_output) and not overwrite_trained:
        print('MLP with prefix %s is already available and will not be overwritten.'
              'Training  will be skipped.' % output_prefix)
        return None

    # copy .xml file since expert will demand it
    if output_prefix != prefix:
        shutil.copyfile(os.path.join(working_dir, prefix + '.xml'), os.path.join(working_dir, output_prefix + '.xml'))

    # ignore specific variables in training and adapt the .xml file
    # this requires a different output prefix
    ignore_vars_modified = None
    if ignore_vars is not None:
        if prefix == output_prefix:
            print("If variables are ignored for training, the output prefix has to be set.")
            raise ValueError
        if tracks % 2 != 0:
            print("Tracks have to be an even number")
            raise ValueError

        # TODO: check  if variables with corresponding tacks are actually available in xml file
        # TODO: adapt new corresponding xml file

        ignore_vars_modified = ['__weight__']
        for var in ignore_vars:
            ignore_vars_modified += get_ignore_vars_std(var, tracks // 2)

    variable_file = os.path.join(working_dir, prefix + '_training_data.root')

    print('Loading data to memory...')
    d_train, d_valid, a_test_x, a_test_y, esb_trafo = rd.get_train_sets(variable_file, ignore_vars=ignore_vars_modified,
                                                                        target_var=target_var, fraction=fraction,
                                                                        shuffle=shuffle)

    # define features
    features = a_test_x.shape[1]

    train_trafo_name = os.path.join(working_dir, output_prefix + '_trafo.pkl')

    with open(train_trafo_name, 'wb') as f:
        pickle.dump(esb_trafo, f)

    theano_function_output = os.path.join(working_dir, output_prefix + '_theano.pkl')

    bin_trafo = os.path.join(working_dir, output_prefix + '_bin_trafo.pkl')

    # ### test_set, train_set monitoring
    # mlp_args['dataset_train_monitor'] = d_train_monitor
    # mlp_args['dataset_test_monitor'] = d_test

    mlp_args['feature_number'] = features
    mlp_args['save_path'] = mlp_output

    # returns the signal fraction on the eval test set.
    train_params['signal_fraction'] = binning.get_signal_fraction(a_test_y)

    # write out json file for the training
    write_out_json(mlp_args, train_params, variable_file, mlp_output, list(esb_trafo.copy().keys()), network_model)
    # initialize the training
    mlp_training = get_mlp_training(d_train, d_valid, network_model, **mlp_args)

    # start the train loop
    mlp_training.main_loop()

    # create transformation to probability
    f_model = mlp_training.model

    # calculate PDFs and signal fraction

    th_x = f_model.get_input_space().make_theano_batch()
    th_y = f_model.fprop(th_x)
    th_f = th.function([th_x], th_y)

    with open(theano_function_output, 'wb') as f:
        pickle.dump(th_f, f)

    # also convert model and theano function to cpu
    convert_gpu_to_cpu_model(mlp_output)

    a_test_y_hat = th_f(a_test_x)

    df = pandas.DataFrame.from_dict({'y': a_test_y.reshape(-1), 'y_hat': a_test_y_hat.reshape(-1)})

    # get the classifier pdf
    b_map = binning.get_transform_to_probability_map(df)

    with open(bin_trafo, 'wb') as f:
        pickle.dump(b_map, f)


def get_test_data(weight_dir, test_dir=None, weight_prefix='standard', test_prefix=None, shuffle=False, fraction=-1,
                  ignore_vars=None, transform_to_prob=False, tracks=10, target_var='qrCombined',
                  mode='gpu'):
    """

    :param weight_dir: directory with weight files
    :param test_dir: directory with root set to test
    :param weight_prefix: prefix of weights
    :param test_prefix: prefix of test set
    :param shuffle: shuffle the test data
    :param fraction: train/test+validate fraction, if -1, complete set will be used
    :param ignore_vars: exact variable name to ignore
    :param transform_to_prob:
    :param tracks:
    :param mode: use cpu/ gpu weight files
    :return:
    """
    if test_dir is None:
        test_dir = weight_dir

    if test_prefix is None:
        test_prefix = weight_prefix

    if mode is 'gpu':
        theano_function_output = os.path.join(weight_dir, weight_prefix + '_theano.pkl')
    elif mode is 'cpu':
        theano_function_output = os.path.join(weight_dir, weight_prefix + '_theano_cpu.pkl')
    else:
        raise NotImplementedError('mode %s' % mode)

    with open(theano_function_output, 'rb') as f:
        th_f = pickle.load(f)

    ignore_vars_modified = ['__weight__']
    if ignore_vars is not None:
        for var in ignore_vars:
            ignore_vars_modified += get_ignore_vars_std(var, tracks // 2)

    variable_file = os.path.join(test_dir, test_prefix + '_training_data.root')
    a_test_x, a_test_y, train_vars = rd.generate_untransformed_testfile(variable_file,
                                                                        target_var=target_var,
                                                                        shuffle=shuffle,
                                                                        fraction=fraction,
                                                                        ignore_vars=ignore_vars_modified)

    train_trafo_name = os.path.join(weight_dir, weight_prefix + '_trafo.pkl')

    with open(train_trafo_name, 'rb') as f:
        d_trafo = pickle.load(f)

    m_transformations = [d_trafo[name] for name in train_vars]

    b_test_x = np.empty(shape=a_test_x.shape, dtype=a_test_x.dtype)

    for column_idx, trafo in enumerate(m_transformations):
        b_test_x[:, column_idx] = binning.transform_array(a_test_x[:, column_idx], *trafo)

    a_test_y_hat = th_f(b_test_x.astype(np.float32))

    if transform_to_prob:
        bin_trafo = os.path.join(weight_dir, weight_prefix + '_bin_trafo.pkl')
        with open(bin_trafo, 'rb') as f:
            b_map = pickle.load(f)
        a_test_y_hat = binning.transfrom_array_to_probability(a_test_y_hat, b_map)

    return a_test_y, a_test_y_hat


def convert_gpu_to_cpu_model(gpu_model_path, overwrite=False):
    if overwrite:
        cpu_model_path = gpu_model_path
        cpu_theano_function_path = gpu_model_path
    else:
        cpu_model_path = gpu_model_path.replace('.pkl', '_cpu.pkl')
        cpu_theano_function_path = cpu_model_path.replace('weights_cpu.pkl', 'theano_cpu.pkl')

    # changeging theano flags has to be done in a different process, since device option cannot be modified via
    # theano.config.device after initializiation

    theano_flags = os.environ['THEANO_FLAGS']
    print(theano_flags)
    theano_flags = re.sub("gpu\d", "cpu", theano_flags)
    print(theano_flags)
    # theano_flags = theano_flags.replace('device=gpu', 'device=cpu')

    os.environ['THEANO_FLAGS'] = theano_flags

    try:
        base_path = os.path.join(os.environ['BELLE2_LOCAL_DIR'], 'analysis/scripts/dft')
    except(KeyError):
        print('WARNING: No basf2 installation found.')
        return None

    full_path = os.path.join(base_path, 'gpu_pkl_to_cpu_pkl.py')
    if not os.path.isfile(full_path):
        print('WARNING:No conversion function available.')
        return None

    subprocess.call('python3 %s %s %s' % (full_path, gpu_model_path, cpu_model_path), shell=True)

    full_path = os.path.join(base_path, 'model_to_theano_function.py')
    subprocess.call('python3 %s %s %s' % (full_path, cpu_model_path, cpu_theano_function_path), shell=True)


def convert_old_xml_file(xml_path_and_name):
    print('WARNING: convert_old_xml_file is only a temporary function')
    replace_dict = {'Pl2TrafoFile': 'theano_transformationFile',
                    'Pl2ProbTrafoFile': 'theano_probabilityTransformationFile',
                    'theanoFunctionFile': 'theano_functionFile'}

    changes = 0
    with fileinput.FileInput(xml_path_and_name, inplace=True) as f:
        for line in f:
            for key, value in replace_dict.items():
                if key in line:
                    changes += line.count(key)
                    line = line.replace(key, value)
            sys.stdout.write(line)

    if changes > 0:
        print('.xml file was replaces %i changes were made' % changes)
