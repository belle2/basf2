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

# VERY HIGH RETENTION RATE: RECONSIDER SKIM


def SystematicsList():

    Lists = []
    Lists += PFromLambdaList()

    return Lists


def PFromLambdaList():
    LambdaCuts = '1.10 < M < 1.13'
    XiCuts = '1.3 < M < 1.34'

    LambdaChannel = ['p+:all pi-:all'
                     ]

    LambdaList = []
    for chID, channel in enumerate(LambdaChannel):
        reconstructDecay('Lambda0:syst' + str(chID) + ' -> ' + channel, LambdaCuts, chID)
        massVertexRave('Lambda0:syst' + str(chID), 0.001)
        LambdaList.append('Lambda0:syst' + str(chID))

    XiChannel = []
    for channel in LambdaList:
        XiChannel.append(channel + ' pi-:all')

    XiList = []
    for chID, channel in enumerate(XiChannel):
        reconstructDecay('Xi-:syst' + str(chID) + ' -> ' + channel, XiCuts, chID)
        XiList.append('Xi-:syst' + str(chID))

    return XiList
