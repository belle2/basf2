#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#######################################################
#
# B -> D(Kspipipi0) h skims
# Minakshi Nayak, 24/Jan/2017
#
######################################################
from basf2 import *
from modularAnalysis import *


def loadDkspipipi0():
    Dcuts = '1.5 < M < 2.2'

    D0_Channels = [
        'K_S0:all pi+:all pi-:all pi0:skim'
    ]
    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:Kspipipi0' + str(chID) + ' -> ' + channel, Dcuts, chID)
        D0List.append('D0:Kspipipi0' + str(chID))
        copyLists('D0:Kspipipi0', D0List)
    return D0List


def BsigToDhToKspipipi0List():
    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'

    BsigChannels = ['D0:Kspipipi0 pi-:all',
                    'D0:Kspipipi0 K-:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        reconstructDecay('B-:BtoDhkspipipi0' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BsigList.append('B-:BtoDhkspipipi0' + str(chID))

    Lists = BsigList
    return Lists
