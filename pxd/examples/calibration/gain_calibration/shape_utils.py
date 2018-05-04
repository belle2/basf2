"""
Some helper functions for dealing with string representations of clusters

:author: benjamin.schwenker@phys.uni-goettinge.de
"""

import math
import re
import numpy as np


def get_label_type(label):
    """Returns label type"""
    return re.split('D', label)[0]


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


def get_short_digital_label(shape, thetaU, thetaV):
    """Short (digital) label contains only entry&exit pixels hit by track"""
    label = "S"
    digits = re.split('D', shape)
    entry, exit = get_entry_exit_index(shape, thetaU, thetaV)

    # Add entry pixel
    data = re.split('\.', digits[entry + 1])
    label += "D" + data[0] + '.' + data[1]

    if not entry == exit:
        # Add exit pixel
        data = re.split('\.', digits[exit + 1])
        label += "D" + data[0] + '.' + data[1]

    return label


def get_entry_exit_index(shape, thetaU=1, thetaV=1):
    "Get indices for entry and exit pixel of track depending on slopes into sensor"
    size = get_size(shape)
    vmax = get_vmax(shape)
    if thetaV >= 0:
        if thetaU >= 0:
            return 0, size - 1
        else:
            return get_indices(shape, 0)[-1], get_indices(shape, vmax)[0]
    else:
        if thetaU >= 0:
            return get_indices(shape, vmax)[0], get_indices(shape, 0)[-1]
        else:
            return get_indices(shape, vmax)[-1], get_indices(shape, 0)[0]


def get_signals(shape):
    """Get array of signals from shape"""
    size = get_size(shape)
    signals = np.zeros((size,), dtype=np.int)
    digits = re.split('D', shape)
    for i in range(size):
        digit = digits[i + 1]
        signals[i] = int(re.split('\.', digit)[2])
    return signals


def pick_signals(shape, indexset):
    """Get array of picked signals from shape. Picked signals are given in indexset."""
    dim = len(indexset)
    signals = np.zeros((dim,), dtype=np.int)
    digits = re.split('D', shape)
    for i, index in enumerate(indexset):
        digit = digits[index + 1]
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


def get_umax(shape):
    """Get maximum ucell of shape"""
    return np.max(get_ucells(shape))


def get_umin(shape):
    """Get minimum ucell of shape"""
    return np.min(get_ucells(shape))


def get_usize(shape):
    """Get size of shape along u axis"""
    return 1 + np.max(get_ucells(shape)) - np.min(get_ucells(shape))


def get_vmax(shape):
    """Get maximum vcell of shape"""
    return np.max(get_vcells(shape))


def get_vmin(shape):
    """Get minimum vcell of shape"""
    return np.min(get_vcells(shape))


def get_vsize(shape):
    """Get size of shape along v axis"""
    return 1 + np.max(get_vcells(shape)) - np.min(get_vcells(shape))


def get_indices(shape, v):
    """Get list of digit indices for vcell=v"""
    indexlist = []
    for i, vcell in enumerate(get_vcells(shape)):
        if vcell == v:
            indexlist.append(i)
    return indexlist


def getPitchV(pixelkind=0):
    """Pixel pitch in mm"""
    if pixelkind == 0:
        return 0.055
    elif pixelkind == 1:
        return 0.060
    elif pixelkind == 2:
        return 0.070
    elif pixelkind == 3:
        return 0.085


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


def getPitchU(pixelkind=0):
    """Pixel pitch in mm"""
    return 0.05
