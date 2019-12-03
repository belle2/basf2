#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Skim list building functions for :math:`B\\to X_c + h` analyses
"""

__authors__ = [
    "Niharika Rout"
]

from basf2 import *
from modularAnalysis import *
import sys
import glob
import os.path
import basf2


def BsigToDhToKpiList(path):
    """
    Skim list definitions for all charged  B to charm 3 body decays.

    **Skim Author**: Niharika Rout

    **Skim Name**: BtoDh_Kpi

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14140101

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:
    1. B- -> D0 (K- pi+) pi-
    2. B- -> D0 (K- pi+) K-


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    1. 1.6 < M_D0 < 2.0
    2. 5.2 < Mbc
    3. abs(deltaE) < 0.5

Note:
    This skim uses loadDKpi() where D0 channels are defined.


    """

    __author__ = "Niharika Rout"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.5'

    BsigChannels = ['D0:Kpi pi-:all',
                    'D0:Kpi K-:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        reconstructDecay('B-:BtoD0h' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        BsigList.append('B-:BtoD0h' + str(chID))

    Lists = BsigList
    return Lists


def loadDKpi(path):

    Dcuts = '1.6 < M < 2.0'
    D0Channels = [
        'K-:all pi+:all'
    ]
    D0List = []
    for chID, channel in enumerate(D0Channels):
        reconstructDecay('D0:list' + str(chID) + ' -> ' + channel, Dcuts, chID, path=path)
        D0List.append('D0:list' + str(chID))
        copyLists('D0:Kpi', D0List, path=path)
    return D0List


def BsigToDhToKpipipiList(path):
    """
    **Skim Author**: N. Rout

    **Skim Name**: BtoDh_Kpipipi

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14140102

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:
    1. B- -> D0 (K- pi+ pi- pi+) pi-
    2. B- -> D0 (K- pi+ pi- pi+) K-


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    1. 1.6 < M_D0 < 2.0
    2. 5.2 < Mbc
    3. abs(deltaE) < 0.5

    Note:
    This skim uses loadDkpipipi() where D0 channels are defined.

    """
    __author__ = "Niharika Rout"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.5'

    BsigChannels = ['D0:Kpipipi pi-:all',
                    'D0:Kpipipi K-:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        reconstructDecay('B-:BtoDhkpipipi' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        BsigList.append('B-:BtoDhkpipipi' + str(chID))

    Lists = BsigList
    return Lists


def loadDkpipipi(path):
    Dcuts = '1.6 < M < 2.0'

    D0_Channels = [
        'K-:all pi+:all pi-:all pi+:all'
    ]
    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:Kpipipi' + str(chID) + ' -> ' + channel, Dcuts, chID, path=path)
        D0List.append('D0:Kpipipi' + str(chID))
        copyLists('D0:Kpipipi', D0List, path=path)
    return D0List
