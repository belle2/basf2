#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for charm analyses.. """


__authors__ = [
    ""
]

from basf2 import *
from modularAnalysis import *


def D0ToHpJm(CharmPath):
    charmcuts = '1.80 < M < 1.93 and useCMSFrame(p)>2.2'
    D0_Channels = ['K-:loose pi+:loose',
                   'pi+:loose pi-:loose',
                   'K+:loose K-:loose',
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID, path=CharmPath)
        vertexKFit('D0:' + str(chID), 0.001, path=CharmPath)
        D0List.append('D0:' + str(chID))

    Lists = D0List
    return Lists


def DstToD0PiD0ToHpJm(CharmPath):

    D0List = D0ToHpJm(CharmPath)

    Dstcuts = '0 < Q < 0.018'

    DstList = []
    for chID, channel in enumerate(D0List):
        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all ' + channel, Dstcuts, chID, path=CharmPath)
        vertexRave('D*+:' + str(chID), 0.001, path=CharmPath)
        DstList.append('D*+:' + str(chID))

    return DstList


def DstToD0PiD0ToHpJmPi0(CharmPath):
    Dstcuts = '0 < Q < 0.018'
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2.2'
    cutAndCopyList('pi0:myskim', 'pi0:skim', '0.11 < M < 0.15 and p > 0.28', path=CharmPath)

    DstList = []
    reconstructDecay('D0:sig -> K-:loose pi+:loose pi0:myskim', charmcuts, chID, path=CharmPath)
    vertexTree('D0:sig', 0.001, path=CharmPath)
    reconstructDecay('D*+:RS -> D0:RS pi+:all', Dstcuts, path=CharmPath)
    reconstructDecay('D*+:WS -> D0:WS pi+:all', Dstcuts, path=CharmPath)
    copyLists('D*+:sig', ['D*+:RS', 'D*+:WS'], path=CharmPath)
    vertexKFit('D*+:sig', 0.001, path=CharmPath)
    DstList.append('D*+:sig')

    return DstList


def DstToD0PiD0ToHpHmPi0(CharmPath):
    Dstcuts = '0 < Q < 0.018'
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2.2'
    cutAndCopyList('pi0:myskim', 'pi0:skim', '0.11 < M < 0.15 and p > 0.28', path=CharmPath)
    D0_Channels = ['pi+:loose pi-:loose pi0:myskim',
                   'K+:loose K-:loose pi0:myskim',
                   ]

    DstList = []

    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID, path=CharmPath)
        vertexTree('D0:' + str(chID), 0.001, path=CharmPath)
        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all D0:' + str(chID), Dstcuts, chID, path=CharmPath)
        vertexKFit('D*+:' + str(chID), 0.001, path=CharmPath)
        DstList.append('D*+:' + str(chID))

    return DstList


def DstToD0PiD0ToHpJmEta(CharmPath):
    reconstructDecay('eta:myskim -> gamma:loose gamma:loose', '0.49 < M < 0.55 and p > 0.28', path=CharmPath)
    Dstcuts = '0 < Q < 0.018'
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p) > 2.2'

    DstList = []
    reconstructDecay('D0:sig -> K-:loose pi+:loose eta:myskim', charmcuts, path=CharmPath)
    vertexTree('D0:sig', 0.001, path=CharmPath)
    reconstructDecay('D*+:RS -> D0:sig pi+:all', Dstcuts, path=CharmPath)
    reconstructDecay('D*-:WS -> D0:sig pi-:all', Dstcuts, path=CharmPath)
    vertexKFit('D*+:RS', conf_level=0.001, path=CharmPath)
    vertexKFit('D*+:WS', conf_level=0.001, path=CharmPath)
    DstList.append('D*+:RS')
    DstList.append('D*+:WS')

    return DstList


def DstToD0PiD0ToKsOmega(CharmPath):
    cutAndCopyList('pi0:mypi0', 'pi0:skim', '0.11 < M < 0.15 and p > 0.25 ', path=CharmPath)
    reconstructDecay('eta:3pi -> pi+:good pi-:good pi0:mypi0', '0.4 < M < 0.65', path=CharmPath)
    reconstructDecay('omega:3pi -> pi+:good pi-:good pi0:mypi0', '0.65 < M < 0.9', path=CharmPath)

    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2.2'
    reconstructDecay('D0:KsEta -> K_S0:merged eta:3pi', charmcuts, path=CharmPath)
    vertexTree('D0:KsEta', conf_level=0.001, path=CharmPath)
    reconstructDecay('D0:KsOmega -> K_S0:merged omega:3pi', charmcuts, path=CharmPath)
    vertexTree('D0:KsOmega', conf_level=0.001, path=CharmPath)
    copyLists('D0:sig', ['D0:KsEta', 'D0:KsOmega'], path=CharmPath)

    reconstructDecay('D*+:sig -> D0:sig pi+:slow', '0 < Q < 0.018', path=CharmPath)
    vertexKFit('D*+:sig', conf_level=0.001, path=CharmPath)
    DstList.append('D*+:sig')

    return DstList


def D0ToNeutrals(CharmPath):
    charmcuts = '1.78 < M < 1.94 and useCMSFrame(p)>2.2'
    D0_Channels = ['pi0:skim pi0:skim',
                   'K_S0:merged pi0:skim',
                   'K_S0:merged K_S0:merged',
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID, path=CharmPath)
        vertexRave('D0:' + str(chID), 0.001, path=CharmPath)
        D0List.append('D0:' + str(chID))

    Lists = D0List
    return Lists


def DstToD0Neutrals(CharmPath):

    D0List = D0ToNeutrals()

    Dstcuts = '0 < Q < 0.02'

    DstList = []
    for chID, channel in enumerate(D0List):
        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all ' + channel, Dstcuts, chID, path=CharmPath)
        massVertexRave('D*+:' + str(chID), 0.001, path=CharmPath)
        DstList.append('D*+:' + str(chID))

    return DstList


def DstToD0PiD0ToHpHmKs(CharmPath):

    charmcuts = '1.80 < M < 1.93 and useCMSFrame(p)>2.2'
    Dstcuts = '0 < Q < 0.018'

    D0_Channels = ['pi-:loose pi+:loose K_S0:merged',
                   'K-:loose K+:loose K_S0:merged'
                   ]
    DstList = []

    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID, path=CharmPath)
        vertexKFit('D0:' + str(chID), 0.001, path=CharmPath)

        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all D0:' + str(chID), Dstcuts, chID, path=CharmPath)
        vertexKFit('D*+:' + str(chID), 0.001, path=CharmPath)
        DstList.append('D*+:' + str(chID))

    return DstList


def CharmRareList(CharmPath):
    charmcuts = '1.78 < M < 1.94 and useCMSFrame(p)>2.2'
    D0_Channels = ['gamma:skim gamma:skim',
                   'e+:loose e-:loose',
                   'mu+:loose mu-:loose'
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:Rare' + str(chID) + ' -> ' + channel, charmcuts, chID, path=CharmPath)
        D0List.append('D0:Rare' + str(chID))

    Lists = D0List
    return Lists


def CharmSemileptonicList(CharmPath):
    Dcuts = '1.82 < M < 1.92'
    DstarSLcuts = '1.9 < M < 2.1'
    antiD0SLcuts = 'massDifference(0)<0.15'

    D_Channels = ['K-:95eff pi+:95eff',
                  'K-:95eff pi+:95eff pi0:skim',
                  'K-:95eff pi+:95eff pi+:95eff pi-:95eff',
                  'K-:95eff pi+:95eff pi+:95eff pi-:95eff pi0:skim',
                  ]

    DList = []
    for chID, channel in enumerate(D_Channels):
        reconstructDecay('D0:std' + str(chID) + ' -> ' + channel, Dcuts, chID, path=CharmPath)
        DList.append('D0:std' + str(chID))
    copyLists('D0:all', DList)

    DstarSLRecoilChannels = ['D0:all pi+:95eff',
                             ]

    antiD0List = []
    for chID, channel in enumerate(DstarSLRecoilChannels):
        reconstructRecoil('D*-:SL' + str(chID) + ' -> ' + channel, DstarSLcuts, chID, path=CharmPath)
        reconstructRecoilDaughter('anti-D0:SL' + str(chID) + ' -> D*-:SL' + str(chID) +
                                  ' pi-:95eff', antiD0SLcuts, chID, path=CharmPath)
        antiD0List.append('anti-D0:SL' + str(chID))

    nueRecoilChannels = []
    for channel in antiD0List:
        nueRecoilChannels.append(channel + ' K+:95eff e-:std')
        nueRecoilChannels.append(channel + ' pi+:95eff e-:std')

    numuRecoilChannels = []
    for channel in antiD0List:
        numuRecoilChannels.append(channel + ' K+:95eff mu-:std')
        numuRecoilChannels.append(channel + ' pi+:95eff mu-:std')

    nueList = []
    for chID, channel in enumerate(nueRecoilChannels):
        reconstructRecoilDaughter('anti-nu_e:SL' + str(chID) + ' -> ' + channel, '', chID, path=CharmPath)
        nueList.append('anti-nu_e:SL' + str(chID))

    numuList = []
    for chID, channel in enumerate(numuRecoilChannels):
        reconstructRecoilDaughter('anti-nu_mu:SL' + str(chID) + ' -> ' + channel, '', chID, path=CharmPath)
        numuList.append('anti-nu_mu:SL' + str(chID))

    allLists = nueList + numuList
    return allLists
