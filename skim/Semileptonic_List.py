#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# SL skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *


def SemileptonicList():
    cutAndCopyList('e-:SLB', 'e-:all', 'p > 0.35', True)
    cutAndCopyList('mu-:SLB', 'mu-:all', 'p > 0.35', True)
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    BplusChannels = ['D0:all e+:SLB',
                     'D0:all mu+:SLB',
                     'D*0:all e+:SLB',
                     'D*0:all mu+:SLB'
                     ]

    B0Channels = ['D+:all e-:SLB',
                  'D+:all mu-:SLB',
                  'D*+:all e-:SLB',
                  'D*+:all mu-:SLB'
                  ]

    bplusList = []
    for chID, channel in enumerate(BplusChannels):
        reconstructDecay('B+:SL' + str(chID) + ' -> ' + channel, Bcuts, chID)
        bplusList.append('B+:SL' + str(chID))

    b0List = []
    for chID, channel in enumerate(B0Channels):
        reconstructDecay('B0:SL' + str(chID) + ' -> ' + channel, Bcuts, chID)
        b0List.append('B0:SL' + str(chID))

    allLists = b0List + bplusList
    return allLists


def LeptonicList():
    cutAndCopyList('e-:highP', 'e-:all', 'p > 2.0', True)
    cutAndCopyList('mu-:highP', 'mu-:all', 'p > 2.0', True)
    reconstructDecay('B-:L0 -> e-:highP', '', 1)
    reconstructDecay('B-:L1 -> mu-:highP', '', 2)
    lepList = ['B-:L0', 'B-:L1']
    return lepList
