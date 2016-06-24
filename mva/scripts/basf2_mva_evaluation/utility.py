#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math


def purity(nSig, nBg):
    if nSig == 0:
        return 0.0
    if nSig + nBg == 0:
        return 0.0
    return nSig / float(nSig + nBg)


def efficiency(nSig, nTrueSig):
    if nSig == 0:
        return 0.0
    if nTrueSig == 0:
        return float('inf')
    return nSig / float(nTrueSig)


def efficiencyError(nSig, nTrueSig):
    """
    for an efficiency eps = nSig/nTrueSig, this function calculates the
    standard deviation according to http://arxiv.org/abs/physics/0701199 .
    """
    if nTrueSig == 0:
        return float('inf')

    k = float(nSig)
    n = float(nTrueSig)
    variance = (k + 1) * (k + 2) / ((n + 2) * (n + 3)) - (k + 1) ** 2 / ((n + 2) ** 2)
    if variance <= 0:
        return 0.0
    return math.sqrt(variance)


def purityError(nSig, nBg):
    nTot = nSig + nBg
    if nTot == 0:
        return 0.0
    return efficiencyError(nSig, nTot)
