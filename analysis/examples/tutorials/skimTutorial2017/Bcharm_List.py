#!/usr/bin/env/python3
# -*- coding: utf-8 -*-

#####################################
#
#   Charm B Skim
# R.Cheaib, 30/05/2017
#
######################################

from basf2 import *
from modularAnalysis import *


def Bcharm():
    cutAndCopyList('e-:charm', 'e-:all', 'p>0.50', True)
    cutAndCopyList('mu-:charm', 'mu-:all', 'p>0.50', True)
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE)<0.5'

    Bpluschannels = ['D0:all e+:charm',
                     'D0:all mu+:charm',
                     'D*0:all e+:charm',
                     'D*0:all mu+:charm'
                     ]

    B0channels = ['D+:all e-:charm',
                  'D+:all mu-:charm',
                  'D*+:all e-:charm',
                  'D*+:all mu-:charm'
                  ]

    bplusList = []
    for chID, channel in enumerate(Bpluschannels):
        reconstructDecay('B+:SL' + str(chID) + ' -> ' + channel, Bcuts, chID)
        bplusList.append('B+:SL' + str(chID))

    b0List = []
    for chID, channel in enumerate(B0channels):
        reconstructDecay('B0:SL' + str(chID) + ' -> ' + channel, Bcuts, chID)
        b0List.append('B0:SL' + str(chID))

    allLists = b0List + bplusList
    return allLists
