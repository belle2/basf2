"""
Some helper functions for dealing with string representations of clusters

:author: benjamin.schwenker@phys.uni-goettinge.de
"""

import math
import re
import numpy as np


def get_size(shape):
    """Number of digits in shape"""
    return len(re.split('D', shape)) - 1


def get_shape(cluster):
    """Returns shape from cluster"""
    ucells = get_ucells(cluster)
    vcells = get_vcells(cluster)
    signals = get_signals(cluster)
    minu = np.min(ucells)
    minv = np.min(vcells)
    size = get_size(cluster)
    shape = str(size)
    for i in range(size):
        shape += "D" + str(vcells[i] - minv) + '.' + str(ucells[i] - minu) + '.' + str(signals[i])
    return shape


def get_digital_label(shape):
    """Digital label is the ordered sequence of pixels hit by track"""
    label = "F"
    for digit in re.split('D', shape):
        data = re.split('\.', digit)
        if len(data) == 1:
            pass
        else:
            label += "D" + data[0] + '.' + data[1]
    return label


def get_signals(shape):
    """Get array of signals from shape"""
    size = get_size(shape)
    signals = np.zeros((size,), dtype=np.int)
    digits = re.split('D', shape)
    for i in range(size):
        digit = digits[i + 1]
        signals[i] = int(re.split('\.', digit)[2])
    return signals


def get_vcells(shape):
    """Get array of vcells from shape"""
    size = get_size(shape)
    vcells = np.zeros((size,), dtype=np.int)
    digits = re.split('D', shape)
    for i in range(size):
        digit = digits[i + 1]
        vcells[i] = int(re.split('\.', digit)[0])
    return vcells


def get_ucells(shape):
    """Get array of ucells from shape"""
    size = get_size(shape)
    ucells = np.zeros((size,), dtype=np.int)
    digits = re.split('D', shape)
    for i in range(size):
        digit = digits[i + 1]
        ucells[i] = int(re.split('\.', digit)[1])
    return ucells


def get_usize(shape):
    """Get size of shape along u axis"""
    return 1 + np.max(get_ucells(shape)) - np.min(get_ucells(shape))


def get_vsize(shape):
    """Get size of shape along v axis"""
    return 1 + np.max(get_vcells(shape)) - np.min(get_vcells(shape))


def getPixelKind(VPitch):
    if abs(VPitch - 0.0055) < 0.0001:
        return 0
    elif abs(VPitch - 0.0060) < 0.0001:
        return 1
    elif abs(VPitch - 0.0070) < 0.0001:
        return 2
    elif abs(VPitch - 0.0085) < 0.0001:
        return 3
    else:
        return -1
