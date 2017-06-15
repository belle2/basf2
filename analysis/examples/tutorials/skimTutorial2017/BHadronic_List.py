#!/usr/bin/env/python3
# -*- coding: utf-8 -*-

#####################################
#
#   Hadronic B Skim
# R.Cheaib, 30/05/2017
#
######################################

from basf2 import *
from modularAnalysis import *


def D0ToYpZm():
    Dcuts = '1.8 < InvM <1.9'

    D0channels = ['K-:loose  pi+:loose',
                  'pi+:loose pi-:loose'
                  ]
    D0List = []
    for chID, channel in enumerate(D0channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, Dcuts, chID)
        D0List.append('D0:' + str(chID))

    Lists = D0List
    return Lists


def BHadronic():

    Bcuts = '5.24<Mbc<5.29 '

    D0List = D0ToYpZm()

    BList = []

    for chID, channel in enumerate(D0List):
        reconstructDecay('B+:' + str(chID) + ' -> pi+:all ' + channel, Bcuts, chID)
    copyLists('B+:all', ['B+:0', 'B+:1'])

    BList.append('B+:all')
    return BList
