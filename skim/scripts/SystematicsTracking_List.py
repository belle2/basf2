#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Sysetmatics Tracking
# Y. Kato, 1/Oct/2016
#
######################################################

from basf2 import *
from modularAnalysis import *


def SystematicsTrackingList():

    Lists = []
    Lists += BtoDStarPiList()
    Lists += DstarToD0PiPartList()

    return Lists


def DstarToD0PiPartList():

    fillParticleList("pi+:fromks", "chiProb > 0.001 and pionID > 0.1 and d0 > 0.1")

# D-
    DminusCuts = '1.0 < M < 1.75'
    DminusChannel = ['pi+:fromks pi+:loose pi-:loose']

    for chID, channel in enumerate(DminusChannel):
        resonanceName = 'D-:loose' + str(chID)
        reconstructDecay(resonanceName + ' -> ' + channel, DminusCuts, chID)

# Dstar

    DstarCuts = 'massDifference(0)<0.2 and useCMSFrame(p) > 2.0'
    DstarChannel = []
    DstarChannel.append('D-:loose0' + ' pi+:loose')

    DstarList = []
    for chID, channel in enumerate(DstarChannel):
        resonanceName = 'D*0:loose' + str(chID)
        reconstructDecay(resonanceName + ' -> ' + channel, DstarCuts, chID)
        DstarList.append(resonanceName)
        matchMCTruth(resonanceName)

    return DstarList


def BtoDStarPiList():
    D0Cuts = '1.835 < M < 1.895'
    DstarCuts = 'massDifference(0)<0.16'
    B0Cuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'

# D0

    D0Channel = ['K+:loose pi-:loose', 'K+:loose pi-:loose pi-:loose pi+:loose', 'K+:loose pi-:loose pi0:loose']

    D0List = []
    for chID, channel in enumerate(D0Channel):
        resonanceName = 'anti-D0:loose' + str(chID)
        reconstructDecay(resonanceName + ' -> ' + channel, D0Cuts, chID)
#        vertexRave(resonanceName, 0.0)
        matchMCTruth(resonanceName)
    copyLists('anti-D0:loose', ["anti-D0:loose0", "anti-D0:loose1", "anti-D0:loose2"])
    D0List.append('anti-D0:loose')
# Dstar

    DstarChannel = []
    for channel in D0List:
        DstarChannel.append(channel + ' pi-:loose')

    DstarList = []
    for chID, channel in enumerate(DstarChannel):
        resonanceName = 'D*-:loose' + str(chID)
        reconstructDecay(resonanceName + ' -> ' + channel, DstarCuts, chID)
#        vertexRave(resonanceName, 0.0)
        DstarList.append(resonanceName)
        matchMCTruth(resonanceName)

# B0

    B0Channel = []
    for channel in DstarList:
        B0Channel.append(channel + ' pi+:loose')

    B0List = []
    for chID, channel in enumerate(B0Channel):
        resonanceName = 'B0:sys' + str(chID)
        reconstructDecay(resonanceName + ' -> ' + channel, B0Cuts, chID)
        B0List.append(resonanceName)
#        vertexRave(resonanceName, 0.0)
        matchMCTruth(resonanceName)

    return B0List
