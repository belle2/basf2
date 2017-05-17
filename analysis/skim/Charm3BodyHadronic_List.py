#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# P. Urquijo, 15/Jan/2015
# G. Casarosa, 7/Oct/2016
#
######################################################

from basf2 import *
from modularAnalysis import *


def D0ToHpJmPi0():
    charmcuts = '1.78 < M < 1.94'
    D0_Channels = ['K-:loose pi+:loose pi0:all',
                   'pi+:loose pi-:loose pi0:all',
                   'K+:loose K-:loose pi0:all',
                   ]

    D0_FitChannels = ['^K-:loose ^pi+:loose pi0:all',
                      '^pi+:loose ^pi-:loose pi0:all',
                      '^K+:loose ^K-:loose pi0:all',
                      ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        massVertexRave('D0:' + str(chID), 0.001, 'D0:' + str(chID) + ' -> ' + D0_FitChannels[chID])
        D0List.append('D0:' + str(chID))

    Lists = D0List
    return Lists


def DstToD0PiD0ToHpJmPi0():

    Dstcuts = '0 < Q < 0.04'
    precharmcuts = '1.7 < M < 2'
    postcharmcuts = '1.78 < M < 1.94'
    D0_Channels = ['K-:loose pi+:loose pi0:all',
                   'pi+:loose pi-:loose pi0:all',
                   'K+:loose K-:loose pi0:all',
                   ]

    D0_FitChannels = ['^K-:loose ^pi+:loose pi0:all',
                      '^pi+:loose ^pi-:loose pi0:all',
                      '^K+:loose ^K-:loose pi0:all',
                      ]

    DstList = []

    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:pre' + str(chID) + ' -> ' + channel, precharmcuts, chID)
        vertexRave('D0:pre' + str(chID), 0.001, 'D0:' + str(chID) + ' -> ' + D0_FitChannels[chID])
        cutAndCopyList('D0:' + str(chID), 'D0:pre' + str(chID), postcharmcuts, True)
        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all D0:' + str(chID), Dstcuts, chID)
        DstList.append('D*+:' + str(chID))

        return DstList


def DstToD0PiD0ToHpHmKs():

    precharmcuts = '1.7 < M < 2'
    postcharmcuts = '1.78 < M < 1.94'
    Dstcuts = '0 < Q < 0.04'

    D0_Channels = ['pi-:loose pi+:loose K_S0:all',
                   'K-:loose K+:loose K_S0:all',
                   ]

    DstList = []

    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:pre' + str(chID) + ' -> ' + channel, precharmcuts, chID)
        vertexRave('D0:pre' + str(chID), 0.001)
        cutAndCopyList('D0:' + str(chID), 'D0:pre' + str(chID), postcharmcuts, True)

        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all D0:' + str(chID), Dstcuts, chID)
        vertexRave('D*+:' + str(chID), 0.001)
        DstList.append('D*+:' + str(chID))

    return DstList
