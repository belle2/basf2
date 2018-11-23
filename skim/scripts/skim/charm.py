#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for charm analyses.. """


__authors__ = [
    ""
]

from basf2 import *
from modularAnalysis import *


def D0ToHpJm():
    charmcuts = '1.79 < M < 1.93 and useCMSFrame(p)>2'
    D0_Channels = ['K-:loose pi+:loose',
                   'pi+:loose pi-:loose',
                   'K+:loose K-:loose',
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        vertexRave('D0:' + str(chID), 0.001)
        D0List.append('D0:' + str(chID))

    Lists = D0List
    return Lists


def DstToD0PiD0ToHpJm():

    D0List = D0ToHpJm()

    Dstcuts = '0 < Q < 0.022'

    DstList = []
    for chID, channel in enumerate(D0List):
        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all ' + channel, Dstcuts, chID)
        vertexRave('D*+:' + str(chID), 0.001)
        DstList.append('D*+:' + str(chID))

    return DstList


def D0ToHpJmPi0():
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2'
    D0_Channels = ['K-:loose pi+:loose pi0:skim',
                   'pi+:loose pi-:loose pi0:skim',
                   'K+:loose K-:loose pi0:skim',
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        vertexRave('D0:' + str(chID), 0.001)
        D0List.append('D0:' + str(chID))

    Lists = D0List
    return Lists


def DstToD0PiD0ToHpJmPi0():

    Dstcuts = '0 < Q < 0.02'
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2.0'
    D0_Channels = ['K-:loose pi+:loose pi0:skim',
                   'pi+:loose pi-:loose pi0:skim',
                   'K+:loose K-:loose pi0:skim',
                   ]

    DstList = []

    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        vertexRave('D0:' + str(chID), 0.001)
        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all D0:' + str(chID), Dstcuts, chID)
        vertexRave('D*+:' + str(chID), 0.001)
        DstList.append('D*+:' + str(chID))

    return DstList


def D0ToNeutrals():
    charmcuts = '1.78 < M < 1.94 and useCMSFrame(p)>2'
    D0_Channels = ['pi0:skim pi0:skim',
                   'K_S0:all pi0:skim',
                   'K_S0:all K_S0:all',
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

    Dstcuts = '0 < Q < 0.04'

    DstList = []
    for chID, channel in enumerate(D0List):
        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all ' + channel, Dstcuts, chID)
        massVertexRave('D*+:' + str(chID), 0.001)
        DstList.append('D*+:' + str(chID))

    return DstList


def DstToD0PiD0ToHpHmKs():

    charmcuts = '1.78 < M < 1.94 and useCMSFrame(p)>2'
    Dstcuts = '0 < Q < 0.04'

    D0_Channels = ['pi-:loose pi+:loose K_S0:all',
                   'K-:loose K+:loose K_S0:all'
                   ]
    DstList = []

    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        vertexRave('D0:' + str(chID), 0.001)

        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all D0:' + str(chID), Dstcuts, chID)
        vertexRave('D*+:' + str(chID), 0.001)
        DstList.append('D*+:' + str(chID))

    return DstList


def CharmRareList():
    charmcuts = '1.78 < M < 1.94 and useCMSFrame(p)>2'
    D0_Channels = ['gamma:skim gamma:skim',
                   'e+:loose e-:loose',
                   'mu+:loose mu-:loose pi0:skim'
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
