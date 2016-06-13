#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Jochen Gemmler 2016

from dft import binning
import numpy as np


def to_numpy(arr):
    n_arr = arr.view(np.float32).reshape(arr.shape + (-1,))
    return n_arr


def get_variable_names(arr):
    return list(arr.dtype.names)


def transform_variable(arr, variable_name):
    """ only equal statistics binning implemented
    :param arr:
    :param variable_name:
    :return: list with bin_limits, bin_values
    """
    _arr = to_numpy(arr[variable_name])
    bin_limits, bin_values = binning.get_modified_bin_limits(_arr)

    arr[variable_name] = binning.transform_array(_arr, bin_limits, bin_values).reshape(-1)
    return [bin_limits, bin_values]


def transform_array(arr, ignore=None):
    """
    :param arr: numpy structured array array to transform
    :param ignore: don't transform this variable
    :return: dictionary: variable_name: [bin_limits, bin_values] from transform_variable
    """
    variable_names = get_variable_names(arr)
    variable_names = [var for var in variable_names if var not in ignore]
    transform_dict = {}

    total_variables = len(variable_names)
    var_count = 0
    for variable in variable_names:
        m_trafo = transform_variable(arr, variable)
        transform_dict[variable] = m_trafo
        var_count += 1
        print('%i / %i variables transformed.' % (var_count, total_variables))

    return transform_dict


def transform_array_from_dict(arr, dict):
    """ transforms an numpy structured array from dict
    :param arr:
    :param dict:
    :return:
    """
    arr_variables = get_variable_names(arr)

    transform_variable_names = [var for var in dict if var in arr_variables]

    # print number of total transformed variables
    total_variables = len(arr_variables)
    var_count = 0
    for var in transform_variable_names:
        transform_variable(arr, var)
        var_count += 1
        print('%i / %i variables transformed.' % (var_count, total_variables))
