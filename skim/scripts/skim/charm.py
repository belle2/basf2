#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for charm analyses.. """


__authors__ = [
    ""
]

from basf2 import *
from modularAnalysis import *


def D0ToHpJm():
    charmcuts = '1.80 < M < 1.93 and useCMSFrame(p)>2.2'
    D0_Channels = ['K-:loose pi+:loose',
                   'pi+:loose pi-:loose',
                   'K+:loose K-:loose',
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        vertexKFit('D0:' + str(chID), 0.001)
        D0List.append('D0:' + str(chID))

    Lists = D0List
    return Lists


def DstToD0PiD0ToHpJm():

    D0List = D0ToHpJm()

    Dstcuts = '0 < Q < 0.018'

    DstList = []
    for chID, channel in enumerate(D0List):
        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all ' + channel, Dstcuts, chID)
        vertexRave('D*+:' + str(chID), 0.001)
        DstList.append('D*+:' + str(chID))

    return DstList


def DstToD0PiD0ToHpJmPi0():
    Dstcuts = '0 < Q < 0.018'
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2.2'
    cutAndCopyList('pi0:myskim', 'pi0:skim', '0.11 < M < 0.15 and p > 0.28')

    DstList = []
    reconstructDecay('D0:sig -> K-:loose pi+:loose pi0:myskim', charmcuts, chID)
    vertexTree('D0:sig', 0.001)
    reconstructDecay('D*+:RS -> D0:RS pi+:all', Dstcuts)
    reconstructDecay('D*+:WS -> D0:WS pi+:all', Dstcuts)
    copyLists('D*+:sig', ['D*+:RS', 'D*+:WS'])
    vertexKFit('D*+:sig', 0.001)
    DstList.append('D*+:sig')

    return DstList


def D0ToHpHmPi0():
    cutAndCopyList('pi0:myskim', 'pi0:skim', '0.11 < M < 0.15 and p > 0.28')
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2.2'
    D0_Channels = ['pi+:loose pi-:loose pi0:myskim',
                   'K+:loose K-:loose pi0:myskim',
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        vertexTree('D0:' + str(chID), 0.001)
        D0List.append('D0:' + str(chID))

    Lists = D0List
    return Lists


def DstToD0PiD0ToHpHmPi0():
    Dstcuts = '0 < Q < 0.018'
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2.2'
    cutAndCopyList('pi0:myskim', 'pi0:skim', '0.11 < M < 0.15 and p > 0.28')
    D0_Channels = ['pi+:loose pi-:loose pi0:myskim',
                   'K+:loose K-:loose pi0:myskim',
                   ]

    DstList = []

    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        vertexTree('D0:' + str(chID), 0.001)
        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all D0:' + str(chID), Dstcuts, chID)
        vertexKFit('D*+:' + str(chID), 0.001)
        DstList.append('D*+:' + str(chID))

    return DstList


def DstToD0PiD0ToHpJmEta():
    reconstructDecay('eta:myskim -> gamma:loose gamma:loose', '0.49 < M < 0.55 and p > 0.28')
    Dstcuts = '0 < Q < 0.018'
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p) > 2.2'

    DstList = []
    reconstructDecay('D0:sig -> K-:loose pi+:loose eta:myskim', charmcuts)
    vertexTree('D0:sig', 0.001)
    reconstructDecay('D*+:RS -> D0:sig pi+:all', Dstcuts)
    reconstructDecay('D*-:WS -> D0:sig pi-:all', Dstcuts)
    vertexKFit('D*+:RS', conf_level=0.001)
    vertexKFit('D*+:WS', conf_level=0.001)
    DstList.append('D*+:RS')
    DstList.append('D*+:WS')

    return DstList


def DstToD0PiD0ToKsOmega():
    cutAndCopyList('pi0:mypi0', 'pi0:skim', '0.11 < M < 0.15 and p > 0.25 ')
    reconstructDecay('eta:3pi -> pi+:good pi-:good pi0:mypi0', '0.4 < M < 0.65')
    reconstructDecay('omega:3pi -> pi+:good pi-:good pi0:mypi0', '0.65 < M < 0.9')

    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2.2'
    reconstructDecay('D0:KsEta -> K_S0:merged eta:3pi', charmcuts)
    vertexTree('D0:KsEta', conf_level=0.001)
    reconstructDecay('D0:KsOmega -> K_S0:merged omega:3pi', charmcuts)
    vertexTree('D0:KsOmega', conf_level=0.001)
    copyLists('D0:sig', ['D0:KsEta', 'D0:KsOmega'])

    reconstructDecay('D*+:sig -> D0:sig pi+:slow', '0 < Q < 0.020')
    vertexKFit('D*+:sig', conf_level=0.001)
    DstList.append('D*+:sig')

    return DstList


def D0ToNeutrals():
    charmcuts = '1.78 < M < 1.94 and useCMSFrame(p)>2.2'
    D0_Channels = ['pi0:skim pi0:skim',
                   'K_S0:merged pi0:skim',
                   'K_S0:merged K_S0:merged',
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        vertexRave('D0:' + str(chID), 0.001)
        D0List.append('D0:' + str(chID))

    Lists = D0List
    return Lists


def DstToD0Neutrals():

    D0List = D0ToNeutrals()

    Dstcuts = '0 < Q < 0.02'

    DstList = []
    for chID, channel in enumerate(D0List):
        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all ' + channel, Dstcuts, chID)
        massVertexRave('D*+:' + str(chID), 0.001)
        DstList.append('D*+:' + str(chID))

    return DstList


def DstToD0PiD0ToHpHmKs():

    charmcuts = '1.80 < M < 1.93 and useCMSFrame(p)>2.2'
    Dstcuts = '0 < Q < 0.018'

    D0_Channels = ['pi-:loose pi+:loose K_S0:merged',
                   'K-:loose K+:loose K_S0:merged'
                   ]
    DstList = []

    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        vertexKFit('D0:' + str(chID), 0.001)

        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all D0:' + str(chID), Dstcuts, chID)
        vertexKFit('D*+:' + str(chID), 0.001)
        DstList.append('D*+:' + str(chID))

    return DstList


def CharmRareList():
    charmcuts = '1.78 < M < 1.94 and useCMSFrame(p)>2.2'
    D0_Channels = ['gamma:skim gamma:skim',
                   'e+:loose e-:loose',
                   'mu+:loose mu-:loose'
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:Rare' + str(chID) + ' -> ' + channel, charmcuts, chID)
        D0List.append('D0:Rare' + str(chID))

    Lists = D0List
    return Lists


def CharmSemileptonicList():
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
        reconstructDecay('D0:std' + str(chID) + ' -> ' + channel, Dcuts, chID)
        DList.append('D0:std' + str(chID))
    copyLists('D0:all', DList)

    DstarSLRecoilChannels = ['D0:all pi+:95eff',
                             ]

    antiD0List = []
    for chID, channel in enumerate(DstarSLRecoilChannels):
        reconstructRecoil('D*-:SL' + str(chID) + ' -> ' + channel, DstarSLcuts, chID)
        reconstructRecoilDaughter('anti-D0:SL' + str(chID) + ' -> D*-:SL' + str(chID) +
                                  ' pi-:95eff', antiD0SLcuts, chID)
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
        reconstructRecoilDaughter('anti-nu_e:SL' + str(chID) + ' -> ' + channel, '', chID)
        nueList.append('anti-nu_e:SL' + str(chID))

    numuList = []
    for chID, channel in enumerate(numuRecoilChannels):
        reconstructRecoilDaughter('anti-nu_mu:SL' + str(chID) + ' -> ' + channel, '', chID)
        numuList.append('anti-nu_mu:SL' + str(chID))

    allLists = nueList + numuList
    return allLists
