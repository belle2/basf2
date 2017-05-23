#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Jochen Gemmler 2016 - 2017

from dft import root_to_array as rta
from dft import root_to_numpy as rtn

import numpy as np


def get_train_sets_numpy(f_name, train_vars=None, ignore_vars=None, target_var='qrCombined', fraction=.92,
                         shuffle=True, start=None, stop=None, transform_dict=None):
    """
    :param f_name:
    :param train_vars:
    :param ignore_vars:
    :param target_var:
    :param start_perc: percentage of read-in file to start
    :param stop_perc: percentage of read-in file to stop
    :param start: event (row) in file to start
    :param stop: event (row) in file to stop
    :param v_fraction: determines the number of events used for training only,
    fraction x train, (1 - fraction)/2 x valid, (1 - fraction)/2 x test
    :param transform_dict: perform an already recorded transformation
    :return: pylearn2 dense design matrix and transformation dictionary for variables (with bins)
    """

    if ignore_vars is None:
        ignore_vars = ['__weight__']
    else:
        if '__weight__' not in ignore_vars:
            ignore_vars += ['__weight__']

    if train_vars is None:
        arr = rta.read_root_to_array(f_name, ignore=ignore_vars, start=0, stop=1)
        variables = arr.dtype.names
        train_vars = [var for var in variables if var not in ignore_vars]

    arr = rta.read_root_to_array(f_name, columns=train_vars, ignore=ignore_vars, start=start, stop=stop)

    if shuffle:
        np.random.shuffle(arr)

    valid_end = .5 * (1 + fraction)

    if target_var in train_vars:
        train_vars.remove(target_var)

    if transform_dict is None:
        esb_trafo = rtn.transform_array(arr, ignore=target_var)
    else:
        esb_trafo = transform_dict
        rtn.transform_array_from_dict(arr, transform_dict)

    total_entries = arr.shape[0]

    # consider shuffle after selection
    if shuffle:
        np.random.shuffle(arr)

    train_X = rtn.to_numpy(arr[:int(fraction * total_entries)][train_vars])
    train_y = rtn.to_numpy(arr[:int(fraction * total_entries)][target_var])

    valid_X = rtn.to_numpy(arr[int(fraction * total_entries):int(valid_end * total_entries)][train_vars])
    valid_y = rtn.to_numpy(arr[int(fraction * total_entries):int(valid_end * total_entries)][target_var])

    test_X = rtn.to_numpy(arr[int(valid_end * total_entries):][train_vars])
    test_y = rtn.to_numpy(arr[int(valid_end * total_entries):][target_var])

    print("Train vars:")
    for var in train_vars:
        print(var)

    print("Target var:")
    print(target_var)

    return train_X, train_y, valid_X, valid_y, test_X, test_y, esb_trafo


# function to generate testing files
def generate_testfile(f_name, train_vars=None, ignore_vars=None, target_var='qrCombined',
                      fraction=.92, shuffle=False, trafo=None, start=None, stop=None):
    """
    :param f_name:
    :param train_vars:
    :param ignore_vars:
    :param target_var:
    :param fraction: function will return (1-fraction) /2 x total events, eg if not shuffled, the files for test-set
    during training
    if fraction = -1 function will return the entire file
    :param shuffle:
    :return: variables and target as numpy array, names of the variables used for training
    """

    if ignore_vars is None:
        ignore_vars = ['__weight__']

    if train_vars is None:
        arr = rta.read_root_to_array(f_name, ignore=ignore_vars, start=0, stop=1)
        variables = arr.dtype.names
        print(variables)
        train_vars = [var for var in variables if var not in ignore_vars]
        if target_var in train_vars:
            train_vars.remove(target_var)

    arr = rta.read_root_to_array(f_name, ignore=ignore_vars, start=start, stop=stop)
    if trafo is not None:
        rtn.transform_array_from_dict(arr, trafo)

    total_entries = arr.shape[0]

    valid_end = .5 * (1 + fraction)

    if shuffle:
        np.random.shuffle(arr)

    a_test_x = rtn.to_numpy(arr[int(valid_end * total_entries):][train_vars])
    a_test_y = rtn.to_numpy(arr[int(valid_end * total_entries):][target_var])

    return a_test_x, a_test_y, train_vars
