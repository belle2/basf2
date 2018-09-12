#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(Kshh) h skims
# Minakshi Nayak, 24/Jan/2017
#
######################################################
from basf2 import *
from modularAnalysis import *


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


def BsigToDhToKshhList():
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
