#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# H. Atmacan,  Nov/09/2016
#
######################################################

from basf2 import *
from modularAnalysis import *


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


def BsigToDhTohhList():
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
