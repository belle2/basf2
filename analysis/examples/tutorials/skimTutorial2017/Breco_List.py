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


def Breco():

    Lists = []
    Lists += BplusHadronic()
    Lists += B0Hadronic()

    return Lists


def D0ToYpZm():
    Dcuts = '1.8 < InvM <1.9'

    D0channels = ['K-:loose  pi+:loose',
                  'pi+:loose pi-:loose'
                  ]
    D0List = []
    for chID, channel in enumerate(D0channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, Dcuts, chID)
        D0List.append('D0:' + str(chID))
    copyLists('D0:loose', D0List)
    return D0List


def BplusHadronic():

    Bcuts = '5.24<Mbc<5.29 '

    D0List = D0ToYpZm()

    BplusList = []

    for chID, channel in enumerate(D0List):
        reconstructDecay('B+:' + str(chID) + ' -> pi+:all ' + channel, Bcuts, chID)
        BplusList.append('B+:' + str(chID))

    return BplusList


def B0Hadronic():

    Bcuts = '5.24 < Mbc < 5.29'
    B0channels = ['D0:loose pi+:loose pi-:loose',
                  'D0:loose K+:loose K-:loose'
                  ]

    B0List = []
    for chID, channel in enumerate(B0channels):
        reconstructDecay('B0:' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0List.append('B0:' + str(chID))

    return B0List
