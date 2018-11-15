#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Skim list building functions for :math:`B\\to X_c + h` analyses
"""

__authors__ = [
    "Minakshi Nayak",
    "Hulya Atmacan",
    "Niharika Rout"
]

from basf2 import *
from modularAnalysis import *


def BsigToDhTohhList():
    """
    Skim list definitions for all charged  B to charm 2 body decays.

    **Skim Author**: H. Atmacan

    **Skim Name**: BtoDh_hh

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14140100

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:
    1. B- -> D0 (K- pi+) pi-
    2. B- -> D0 (K+ pi-) pi-
    3. B- -> D0 (pi- pi+) pi-
    4. B- -> D0 (K- K+) pi-
    5. B- -> D0 (K- pi+) K-
    6. B- -> D0 (K+ pi-) K-
    7. B- -> D0 (pi- pi+) K-
    8. B- -> D0 (K- K+) K-


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    1. 1.5 < M_D0 < 2.2
    2. 5.2 < Mbc
    3. abs(deltaE) < 0.3

    Note:
    This skim uses loadD0bar() where D0 channels are defined.


    """

    __author__ = "H. Atmacan"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'

    BsigChannels = ['anti-D0:hh pi+:all',
                    'anti-D0:hh K+:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        reconstructDecay('B+:BtoDh' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BsigList.append('B+:BtoDh' + str(chID))

    Lists = BsigList
    return Lists


def loadD0bar():

    Dcuts = '1.5 < M < 2.2'
    D0barChannels = [
        'K+:all pi-:all',
        'K-:all pi+:all',
        'pi+:all pi-:all',
        'K+:all K-:all'
    ]
    D0barList = []
    for chID, channel in enumerate(D0barChannels):
        reconstructDecay('anti-D0:D0Bar' + str(chID) + ' -> ' + channel, Dcuts, chID)
        D0barList.append('anti-D0:D0Bar' + str(chID))
        copyLists('anti-D0:hh', D0barList)
    return D0barList


def BsigToDhToKshhList():
    """
    **Skim Author**: M. Nayak

    **Skim Name**: BtoDh_Kshh

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14140200

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:
    1. B- -> D0 (Ks K- pi+) pi-
    2. B- -> D0 (Ks K+ pi-) pi-
    3. B- -> D0 (Ks pi- pi+) pi-
    4. B- -> D0 (Ks K- K+) pi-
    5. B- -> D0 (Ks K- pi+) K-
    6. B- -> D0 (Ks K+ pi-) K-
    7. B- -> D0 (Ks pi- pi+) K-
    8. B- -> D0 (Ks K- K+) K-


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    1. 1.5 < M_D0 < 2.2
    2. 5.2 < Mbc
    3. abs(deltaE) < 0.3

    Note:
    This skim uses loadDkshh() where D0 channels are defined.

    """
    __author__ = "M. Nayak"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'

    BsigChannels = ['D0:Kshh pi-:all',
                    'D0:Kshh K-:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        reconstructDecay('B-:BtoDhkshh' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BsigList.append('B-:BtoDhkshh' + str(chID))

    Lists = BsigList
    return Lists


def loadDkshh():
    Dcuts = '1.5 < M < 2.2'

    D0_Channels = [
        'K_S0:all pi+:all pi-:all',
        'K_S0:all K+:all K-:all'
    ]
    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:Kshh' + str(chID) + ' -> ' + channel, Dcuts, chID)
        D0List.append('D0:Kshh' + str(chID))
        copyLists('D0:Kshh', D0List)
    return D0List


def BsigToDhToKspi0List():
    """
    **Skim Author**: M. Nayak

    **Skim Name**: BtoDh_Kspi0

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14140300

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:
    1. B- -> D0 (Ks pi0) pi-
    2. B- -> D0 (Ks pi0) K-


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    1. 1.5 < M_D0 < 2.2
    2. 5.2 < Mbc
    3. abs(deltaE) < 0.3

    Note:
    This skim uses loadDkspi0() where D0 channels are defined.

    """
    __author__ = "M. Nayak"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'

    BsigChannels = ['D0:Kspi0 pi-:all',
                    'D0:Kspi0 K-:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        reconstructDecay('B-:BtoDhkspi0' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BsigList.append('B-:BtoDhkspi0' + str(chID))

    Lists = BsigList
    return Lists


def loadDkspi0():
    Dcuts = '1.5 < M < 2.2'

    D0_Channels = [
        'K_S0:all pi0:skim'
    ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:Kspi0' + str(chID) + ' -> ' + channel, Dcuts, chID)
        D0List.append('D0:Kspi0' + str(chID))
        copyLists('D0:Kspi0', D0List)
    return D0List


def BsigToDhToKspipipi0List():
    """
    **Skim Author**: N. Rout

    **Skim Name**: BtoDh_Kspipipi0

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14140400

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:
    1. B- -> D0 (Ks pi+ pi- pi0) pi-
    2. B- -> D0 (Ks pi+ pi- pi0) K-


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    1. 1.8 < M_D0 < 1.9
    2. 5.25 < Mbc
    3. abs(deltaE) < 0.2

    Note:
    This skim uses loadDkspipipi0() where D0 channels are defined.

    """
    __author__ = "N. Rout"

    Bcuts = 'Mbc > 5.25 and abs(deltaE) < 0.2'

    BsigChannels = ['D0:Kspipipi0 pi-:all',
                    'D0:Kspipipi0 K-:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        reconstructDecay('B-:BtoDhkspipipi0' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BsigList.append('B-:BtoDhkspipipi0' + str(chID))

    Lists = BsigList
    return Lists


def loadDkspipipi0():
    Dcuts = '1.8 < M < 1.9'

    D0_Channels = [
        'K_S0:all pi+:all pi-:all pi0:skim'
    ]
    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:Kspipipi0' + str(chID) + ' -> ' + channel, Dcuts, chID)
        D0List.append('D0:Kspipipi0' + str(chID))
        copyLists('D0:Kspipipi0', D0List)
    return D0List


def DoubleCharmList():
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    B0_Channels = ['D0:all D0:all',
                   'D0:all D*0:all',
                   'D*0:all D0:all',
                   'D*0:all D*0:all']

    B0List = []
    for chID, channel in enumerate(B0_Channels):
        reconstructDecay('B0:DC' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0List.append('B0:DC' + str(chID))

    Lists = B0List
    return Lists
