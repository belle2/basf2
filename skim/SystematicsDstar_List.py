#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Sysetmatics Skims
# P. Urquijo, 1/Oct/2016
#
######################################################

from basf2 import *
from modularAnalysis import *


def SystematicsList():

    Lists = []
    Lists += PiKFromDstarList()

    return Lists


def PiKFromDstarList():
    D0Cuts = '1.835 < M < 1.895'
    DstarCuts = 'massDifference(0)<0.16'

    D0Channel = ['K-:all pi+:all'
                 ]

    D0List = []
    for chID, channel in enumerate(D0Channel):
        reconstructDecay('D0:syst' + str(chID) + ' -> ' + channel, D0Cuts, chID)
        vertexRave('D0:syst' + str(chID), 0.0)
        D0List.append('D0:syst' + str(chID))

    DstarChannel = []
    for channel in D0List:
        DstarChannel.append(channel + ' pi-:all')

    DstarList = []
    for chID, channel in enumerate(DstarChannel):
        reconstructDecay('D*-:syst' + str(chID) + ' -> ' + channel, DstarCuts, chID)
        DstarList.append('D*-:syst' + str(chID))

    return DstarList
