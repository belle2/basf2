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


def D0ToHpJm():
    charmcuts = '1.79 < M < 1.93 and useCMSFrame(p)>2'
    D0_Channels = ['K-:loose pi+:loose',
                   'pi+:loose pi-:loose',
                   'K+:loose K-:loose',
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        vertexRave('D0:' + str(chID), 0.001)
        D0List.append('D0:' + str(chID))

    Lists = D0List
    return Lists


def DstToD0PiD0ToHpJm():

    D0List = D0ToHpJm()

    Dstcuts = '0 < Q < 0.022'

    DstList = []
    for chID, channel in enumerate(D0List):
        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all ' + channel, Dstcuts, chID)
        vertexRave('D*+:' + str(chID), 0.001)
        DstList.append('D*+:' + str(chID))

    return DstList
