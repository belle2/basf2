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
    applyEventCuts('nTracks>4')
    cutAndCopyList('e-:SLB', 'e-:all', 'p>0.35', True)
    cutAndCopyList('mu-:SLB', 'mu-:all', 'p>0.35', True)
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
        applyCuts('B+:SL' + str(chID), 'nTracks>4')
        bplusList.append('B+:SL' + str(chID))

    b0List = []
    for chID, channel in enumerate(B0Channels):
        reconstructDecay('B0:SL' + str(chID) + ' -> ' + channel, Bcuts, chID)
        applyCuts('B+:SL' + str(chID), 'nTracks>4')
        b0List.append('B0:SL' + str(chID))

    allLists = b0List + bplusList
    return allLists
