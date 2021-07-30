#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import numpy as np

# Purity transformations


def get_bins(arr, bin_count=1024):
    """ Returns binning limits for equal statistic binning for an array
    :param arr: numpy array to get binning for
    :param bin_count: int number of bins
    :return: list with bin limits
    """

    # bin_count will produce +2 bins
    bin_count -= 2

    # remove nan -> will be set to zero later (left of lowest bin)
    _arr = arr[np.logical_not(np.isnan(arr))]
    arr_len = len(_arr)
    if bin_count is not None:
        if arr_len <= bin_count:
            raise ValueError('%d entries are not enough for equal statistics binning.' % len(_arr))

    _arr = np.sort(_arr)

    bin_idx_step = arr_len // bin_count

    # if array length not multiple of step size:
    remainder = arr_len % bin_count

    bin_limits = [_arr[0]]

    curr_idx = -1
    for bin_number in range(bin_count):

        curr_idx += bin_idx_step
        if bin_number < remainder:
            curr_idx += 1

        bin_limits.append(_arr[curr_idx])

    return bin_limits


def get_modified_bin_limits(arr, bin_count=1024):
    """ Feature binning: this case considers that multiple values can have the same value
    bins are increased respectively and set to the mean value of the new bin
    :param arr: numpy array to get binning for
    :param bin_count: int number of bins
    :return: list with bin limits
    """
    bins = get_bins(arr, bin_count)
    bin_limits, counts = np.unique(bins, return_counts=True)

    new_bin_limits = []
    bin_weights = []

    for i, count in enumerate(counts):
        new_bin_limits.append(bin_limits[i])
        bin_weights.append(1)
        if count > 1:
            new_bin_limits.append(np.nextafter(bin_limits[i], bin_limits[i] + 1))
            bin_weights.append(count - 1)

    bin_weights.append(1)

    # increase bin limits slightly (make sure that all occurring values are actually binned correctly)
    new_bin_limits[-1] = np.nextafter(new_bin_limits[-1], len(new_bin_limits) * new_bin_limits[-1])

    total_bins = sum(bin_weights)

    current_bin = 0

    step_len = 1 / total_bins
    bin_values = np.zeros(len(bin_weights))

    for i_idx, bin_weight in enumerate(bin_weights):
        bin_values[i_idx] = (current_bin + np.sum(range(bin_weight + 1)) / (bin_weight + 1)) * step_len
        current_bin += bin_weight

    # transform bin values from [0, 1] -> [-1, 1]
    bin_values = 2 * bin_values - 1

    return new_bin_limits, bin_values


def transform_value(value, new_bin_limits, bin_values):
    """ transforms a value according to given bins and bin values (mapping)
    :param value:
    :param new_bin_limits:
    :param bin_values:
    :return:
    """
    if np.isnan(value):
        return 0
    return bin_values[np.digitize(value, new_bin_limits)]


def transform_array(arr, new_bin_limits, bin_values):
    """ transforms an array according to given bins and bin values
    :param arr:
    :param new_bin_limits:
    :param bin_values:
    :return:
    """

    bin_idx = np.digitize(arr, new_bin_limits)
    nan_idx = np.where(np.isnan(arr))

    arr = bin_values[bin_idx]

    arr[nan_idx] = 0
    return arr


def get_transform_to_probability_map(df, bins=100):
    """ returns a transformation map to probability for a signal/background = 1 ratio
    :param df: pandas.DataFrame with truth: 'y', and network output: 'y_hat'
    :param bins: integer with number of bins
    :return: numpy array for bin mapping
    """

    a_bins = np.linspace(0, 1, bins + 1)

    # in case maximum value is equal to 1
    a_bins[-1] = 1.0000001

    # mapping tagger output to signal/ (signal + background) in the relevant bin

    grouped = df['y'].groupby(np.digitize(df['y_hat'], a_bins))

    # check if length equals set of values
    if not len(grouped) == bins:
        raise RuntimeError('Not enough values per bin. Choose less bins.')

    b_map = (grouped.sum() / grouped.count()).values

    return b_map


def transform_to_probability(value, b_map):
    """ transforms a given value to probability according to a bin map
    :param value:
    :param b_map:
    :return: float transformed value
    """

    if value < 0 or value > 1:
        raise ValueError(value)

    # shift -1 for array index purpose
    return b_map[int(value * (len(b_map) - 1))]


def transform_array_to_probability(arr, b_map):
    """ transforms a given arr to probability according to a bin map
    :param arr: numpy array to transform
    :param b_map:
    :return: numpy array: transformed array
    """

    if not np.all(np.isfinite(arr)):
        raise ValueError('Array not finite.')
    if not np.min(arr) >= 0 and not np.max(arr) <= 1:
        raise ValueError('Unexpected input values')

    map_entries = len(b_map)
    return b_map[(arr * (map_entries - 1)).astype(int)]


def get_signal_background_pdf(df, bins=100):
    """ get the signal and background pdfs of a dataframe to a given network output
    :param df:
    :param bins:
    :return: tuple of signal pdf and back ground
    """
    print("WARNING: this function (%s) is not tested yet" % get_signal_background_pdf.__name__)

    a_bins = np.linspace(0, 1, bins + 1)
    a_bins[-1] = 1 + np.nextafter(1, 1.1)

    df_sig = df[df['y'] == 1]
    df_back = df[df['y'] == 0]

    binned_sig = df_sig['y'].groupby(np.digitize(df_sig['y_hat'], a_bins))
    binned_back = df_back['y'].groupby(np.digitize(df_back['y_hat'], a_bins))

    sig_pdf = (binned_sig.count() / df_sig['y'].count()).values
    back_pdf = (binned_back.count() / df_back['y'].count()).values

    return sig_pdf, back_pdf


def trafo_to_prob_sf_func(p_signal, p_background, signal_fraction):
    """
    :param p_signal: signal_pdf value or array
    :param p_background: signal_pdf value or array
    :param signal_fraction:
    :return: (single value, np array) signal fraction dependent to probability transformation
    """

    return (p_signal * signal_fraction) / (p_signal * signal_fraction + p_background * (1 - signal_fraction))


def transform_to_probability_sf(value, sig_back_tuple, signal_fraction):
    """ returns a probability for a given signal fraction != .5
    :param value: classifier output
    :param sig_back_tuple: np.array, signal pdf, background pdf of the trained classifier
    :param signal_fraction: signal fraction of classifier events

    :return: float, probability for a given signal fraction
    """
    assert(signal_fraction > 0)

    p_signal = transform_to_probability(value, sig_back_tuple[0])
    p_background = transform_to_probability(value, sig_back_tuple[1])
    # print('Warning function %s is not tested yet' % transform_to_probability_sf.__name__)
    # function transform to probability actually just evluates the pdf a given point

    return trafo_to_prob_sf_func(p_signal, p_background, signal_fraction)


def transform_array_to_probability_sf(arr, sig_back_tuple, signal_fraction):
    """ transformation to probability. if smother output ("not peaky") is required, please implement spline
    interpolation
    :param arr: array to transform
    :param sig_back_tuple: np.array, signal pdf, background pdf of the trained classifier
    :param signal_fraction: signal fraction of classifier events
    :return:
    """
    assert(signal_fraction > 0)

    p_signal = transform_array_to_probability(arr, sig_back_tuple[0])
    p_back = transform_array_to_probability(arr, sig_back_tuple[1])
    # print('Warning function %s is not tested yet' % transform_array_to_probability_sf.__name__)
    return trafo_to_prob_sf_func(p_signal, p_back, signal_fraction)


def get_signal_fraction(arr, weights=None):
    """
    :param arr:
    :param weights:
    :return: signal fraction of a given array
    """
    # isinstance(arr, np.array)

    if weights is not None:
        return NotImplementedError

    if not np.all(np.isfinite(arr)):
        raise ValueError('Array not finite.')
    if not np.min(arr) >= 0 and not np.max(arr) <= 1:
        raise ValueError('Unexpected input values.')

    return np.sum(arr) / len(arr)


# new MVA interface adaptions
def get_ndarray_binning_parameters(ndarr, bin_count=1024):
    """
    :param ndarr: numpy.ndarray with variables to transform (may contain NaN values)
    :param bin_count: number of bins
    :return: list of tuples with scheme [new_bin_limits, bin_values]
    """

    binning_parameters = []
    # transform each column in an numpy ndarr
    for column in ndarr.T:
        binning_parameters.append(get_modified_bin_limits(column, bin_count))

    return binning_parameters


def transform_ndarray(ndarr, binning_parameters):
    """ flatten ndarray
    :param ndarr: numpy.ndarray with variables
    :param binning_parameters: list of tuples with scheme [new_bin_limits, bin_values]
    :return: None, inplace operation
    """

    assert(ndarr.dtype not in [np.int, np.int16, np.int32, np.int64])
    for i, param_tuple in enumerate(binning_parameters):
        ndarr[:, i] = transform_array(ndarr[:, i], *param_tuple)

    return None


def transform_variable_vector(arr, binning_parameters):
    """ transform only according to a recorded flatten distribution. this is necessary for single vector experts
    :param arr: numpy.array
    :param binning_parameters: list of tuples with scheme [new_bin_limits, bin_values]
    :return: None, inplace operation
    """

    assert(arr.dtype not in [np.int, np.int16, np.int32, np.int64])
    for i, param_tuple in enumerate(binning_parameters):
        arr[i] = transform_value(arr[i], *param_tuple)

    return None


def sanitize_labels(arr):
    """
    checks for a binary classification problem
    transforms the two class labels to {0,1}

    @param arr          numpy array,
    @:return            None, inplace, will not change dtype
    """
    # not binary
    assert len(np.unique(arr)) == 2, 'Not a binary classification!'

    # reject corner cases when classes would have special values
    if arr.min() > 0:
        arr[arr == arr.min()] = 0

    if arr.max() != 1:
        arr[arr == arr.max()] = 1

    # transform labels
    if arr.min() != 0:
        arr[arr == arr.min()] = 0
