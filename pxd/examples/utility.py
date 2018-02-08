#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###################################################
# Some helper functions to deal with cluster shapes
###################################################

from basf2 import *
from ROOT import Belle2


def computeEta(cluster, thetaU, thetaV):
    head, tail = get_head_tail_index(cluster, thetaU, thetaV)
    if not tail == head:
        eta = getCharge(cluster, tail) / (getCharge(cluster, tail) + getCharge(cluster, head))
    else:
        eta = cluster.getCharge()
    return eta


def getDigit(cluster, index):
    digit = cluster.getRelationsTo("PXDDigits").object(index)
    return digit.getVCellID(), digit.getUCellID(), digit.getCharge()


def getCharge(cluster, index):
    digit = cluster.getRelationsTo("PXDDigits").object(index)
    return int(digit.getCharge())


def get_head_tail_index(cluster, thetaU, thetaV):
    size = cluster.getSize()
    vmax = cluster.getVSize() - 1
    vmin = cluster.getVStart()

    if thetaV >= 0:
        if thetaU >= 0:
            return size - 1, 0
        else:
            return get_indices_at_v(cluster, vmax)[0], get_indices_at_v(cluster, 0)[-1]
    else:
        if thetaU >= 0:
            return get_indices_at_v(cluster, 0)[-1], get_indices_at_v(cluster, vmax)[0]
        else:
            return get_indices_at_v(cluster, 0)[0], get_indices_at_v(cluster, vmax)[-1]


def get_indices_at_v(cluster, v):
    indexlist = []
    digits = cluster.getRelationsTo("PXDDigits")
    vmin = cluster.getVStart()
    for i, digit in enumerate(digits):
        vcell = digit.getVCellID() - vmin
        if vcell == v:
            indexlist.append(i)
    return indexlist


def get_short_shape_name(cluster, thetaU, thetaV):
    head, tail = get_head_tail_index(cluster, thetaU, thetaV)
    vmin = cluster.getVStart()
    umin = cluster.getUStart()
    name = "S"

    digit = getDigit(cluster, tail)
    name += "D" + str(digit[0] - vmin) + '.' + str(digit[1] - umin)

    if not head == tail:
        digit = getDigit(cluster, head)
        name += "D" + str(digit[0] - vmin) + '.' + str(digit[1] - umin)

    return name


def get_full_shape_name(cluster, thetaU, thetaV):
    vmin = cluster.getVStart()
    umin = cluster.getUStart()
    size = cluster.getSize()
    name = "F"

    for index in range(size):
        digit = getDigit(cluster, index)
        name += "D" + str(digit[0] - vmin) + '.' + str(digit[1] - umin)
    return name


def check_cluster(cluster):
    sensor_info = Belle2.VXD.GeoCache.get(cluster.getSensorID())
    reject = False
    kinds = set()

    digits = cluster.getRelationsTo("PXDDigits")
    for digit in digits:
        kind = getPixelKind(sensor_info.getVPitch(sensor_info.getVCellPosition(digit.getVCellID())))
        kinds.add(kind)

        # Cluster at sensor edge
        if digit.getVCellID() <= 0 or digit.getVCellID() >= 767:
            reject = True
        # Cluster at sensor edge
        if digit.getUCellID() <= 0 or digit.getUCellID() >= 249:
            reject = True

    # Check cluster has not digits with different pixel kind
    if not len(kinds) == 1:
        reject = True

    return reject, kinds.pop()


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
