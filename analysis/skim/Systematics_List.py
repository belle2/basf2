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
    Lists += PFromLambdaList()
    Lists += JpsieeTagProbe()
    Lists += JpsimumuTagProbe()

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


def JpsieeTagProbe():
    Cuts = '2.7 < M < 3.4'
    Channel = 'e+:all e-:loose'
    jpsiList = []
    chID = 0
    reconstructDecay('J/psi:eetagprobe' + str(chID) + ' -> ' + Channel, Cuts, chID)
    jpsiList.append('J/psi:eetagprobe' + str(chID))
    return jpsiList


def JpsimumuTagProbe():
    Cuts = '2.8 < M < 3.4'
    Channel = 'mu+:all mu-:loose'
    jpsiList = []
    chID = 0
    reconstructDecay('J/psi:mumutagprobe' + str(chID) + ' -> ' + Channel, Cuts, chID)
    jpsiList.append('J/psi:mumutagprobe' + str(chID))
    return jpsiList
