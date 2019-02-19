#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for charm analyses.. """


__authors__ = [
    ""
]

from basf2 import *
from modularAnalysis import *


def D0ToHpJm(path):
    charmcuts = '1.80 < M < 1.93 and useCMSFrame(p)>2.2'
    D0_Channels = ['K-:loose pi+:loose',
                   'pi+:loose pi-:loose',
                   'K+:loose K-:loose',
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:HpJm' + str(chID) + ' -> ' + channel, charmcuts, chID, path=path)
        vertexKFit('D0:HpJm' + str(chID), 0.001, path=path)
        D0List.append('D0:HpJm' + str(chID))

    Lists = D0List
    return Lists


def DstToD0PiD0ToHpJm(path):

    D0List = D0ToHpJm(path)

    Dstcuts = '0 < Q < 0.018'

    DstList = []
    for chID, channel in enumerate(D0List):
        reconstructDecay('D*+:HpJm' + str(chID) + ' -> pi+:all ' + channel, Dstcuts, chID, path=path)
        vertexRave('D*+:HpJm' + str(chID), 0.001, path=path)
        DstList.append('D*+:HpJm' + str(chID))

    return DstList


def DstToD0PiD0ToHpJmPi0(path):
    Dstcuts = '0 < Q < 0.018'
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2.2'
    cutAndCopyList('pi0:myskim', 'pi0:skim', '0.11 < M < 0.15 and p > 0.28', path=path)

    DstList = []
    reconstructDecay('D0:HpJmPi0 -> K-:loose pi+:loose pi0:myskim', charmcuts, path=path)
    vertexTree('D0:HpJmPi0', 0.001, path=path)
    reconstructDecay('D*+:HpJmPi0RS -> D0:HpJmPi0 pi+:all', Dstcuts, path=path)
    reconstructDecay('D*-:HpJmPi0WS -> D0:HpJmPi0 pi-:all', Dstcuts, path=path)
    copyLists('D*+:HpJmPi0', ['D*+:HpJmPi0RS', 'D*+:HpJmPi0WS'], path=path)
    vertexKFit('D*+:HpJmPi0', 0.001, path=path)
    DstList.append('D*+:HpJmPi0')

    return DstList


def DstToD0PiD0ToHpHmPi0(path):
    Dstcuts = '0 < Q < 0.018'
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2.2'
    cutAndCopyList('pi0:myskim', 'pi0:skim', '0.11 < M < 0.15 and p > 0.28', path=path)
    D0_Channels = ['pi+:loose pi-:loose pi0:myskim',
                   'K+:loose K-:loose pi0:myskim',
                   ]

    DstList = []

    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:HpHmPi0' + str(chID) + ' -> ' + channel, charmcuts, chID, path=path)
        vertexTree('D0:HpHmPi0' + str(chID), 0.001, path=path)
        reconstructDecay('D*+:HpHmPi0' + str(chID) + ' -> pi+:all D0:HpHmPi0' + str(chID), Dstcuts, chID, path=path)
        vertexKFit('D*+:HpHmPi0' + str(chID), 0.001, path=path)
        DstList.append('D*+:HpHmPi0' + str(chID))

    return DstList


def DstToD0PiD0ToHpJmEta(path):
    reconstructDecay('eta:myskim -> gamma:loose gamma:loose', '0.49 < M < 0.55 and p > 0.28', path=path)
    Dstcuts = '0 < Q < 0.018'
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p) > 2.2'

    DstList = []
    reconstructDecay('D0:HpJmEta -> K-:loose pi+:loose eta:myskim', charmcuts, path=path)
    vertexTree('D0:HpJmEta', 0.001, path=path)
    reconstructDecay('D*+:HpJmEtaRS -> D0:HpJmEta pi+:all', Dstcuts, path=path)
    reconstructDecay('D*-:HpJmEtaWS -> D0:HpJmEta pi-:all', Dstcuts, path=path)
    vertexKFit('D*+:HpJmEtaRS', conf_level=0.001, path=path)
    vertexKFit('D*+:HpJmEtaWS', conf_level=0.001, path=path)
    DstList.append('D*+:HpJmEtaRS')
    DstList.append('D*+:HpJmEtaWS')

    return DstList


def DstToD0PiD0ToKsOmega(path):
    cutAndCopyList('pi0:mypi0', 'pi0:skim', '0.11 < M < 0.15 and p > 0.25 ', path=path)
    reconstructDecay('eta:3pi -> pi+:loose pi-:loose pi0:mypi0', '0.4 < M < 0.65', path=path)
    reconstructDecay('omega:3pi -> pi+:loose pi-:loose pi0:mypi0', '0.65 < M < 0.9', path=path)

    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2.2'
    reconstructDecay('D0:Eta -> K_S0:merged eta:3pi', charmcuts, path=path)
    vertexTree('D0:Eta', conf_level=0.001, path=path)
    reconstructDecay('D0:Omega -> K_S0:merged omega:3pi', charmcuts, path=path)
    vertexTree('D0:Omega', conf_level=0.001, path=path)
    copyLists('D0:KsOmega', ['D0:Eta', 'D0:Omega'], path=path)

    DstList = []
    reconstructDecay('D*+:KsOmega -> D0:KsOmega pi+:all', '0 < Q < 0.018', path=path)
    vertexKFit('D*+:KsOmega', conf_level=0.001, path=path)
    DstList.append('D*+:KsOmega')

    return DstList


def D0ToNeutrals(path):
    charmcuts = '1.78 < M < 1.94 and useCMSFrame(p)>2.2'
    D0_Channels = ['pi0:skim pi0:skim',
                   'K_S0:merged pi0:skim',
                   'K_S0:merged K_S0:merged',
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:2Nbdy' + str(chID) + ' -> ' + channel, charmcuts, chID, path=path)
        vertexRave('D0:2Nbdy' + str(chID), 0.001, path=path)
        D0List.append('D0:2Nbdy' + str(chID))

    Lists = D0List
    return Lists


def DstToD0Neutrals(path):

    D0List = D0ToNeutrals(path)

    Dstcuts = '0 < Q < 0.02'

    DstList = []
    for chID, channel in enumerate(D0List):
        reconstructDecay('D*+:2Nbdy' + str(chID) + ' -> pi+:all ' + channel, Dstcuts, chID, path=path)
        massVertexRave('D*+:2Nbdy' + str(chID), 0.001, path=path)
        DstList.append('D*+:2Nbdy' + str(chID))

    return DstList


def DstToD0PiD0ToHpHmKs(path):

    charmcuts = '1.80 < M < 1.93 and useCMSFrame(p)>2.2'
    Dstcuts = '0 < Q < 0.018'

    D0_Channels = ['pi-:loose pi+:loose K_S0:merged',
                   'K-:loose K+:loose K_S0:merged'
                   ]
    DstList = []

    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:HpHmKs' + str(chID) + ' -> ' + channel, charmcuts, chID, path=path)
        vertexKFit('D0:HpHmKs' + str(chID), 0.001, path=path)

        reconstructDecay('D*+:HpHmKs' + str(chID) + ' -> pi+:all D0:HpHmKs' + str(chID), Dstcuts, chID, path=path)
        vertexKFit('D*+:HpHmKs' + str(chID), 0.001, path=path)
        DstList.append('D*+:HpHmKs' + str(chID))

    return DstList


def CharmRareList(path):
    charmcuts = '1.78 < M < 1.94 and useCMSFrame(p)>2.2'
    D0_Channels = ['gamma:skim gamma:skim',
                   'e+:loose e-:loose',
                   'mu+:loose mu-:loose'
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:Rare' + str(chID) + ' -> ' + channel, charmcuts, chID, path=path)
        D0List.append('D0:Rare' + str(chID))

    Lists = D0List
    return Lists
