#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
# from stdCharged import *
# from stdPi0 import *
# from stdKS import *


def loadStdD0(path=analysis_main):
    # stdCharged()
    # stdPi0()
    # stdKS()
    Dcuts = '1.82 < M < 1.92'
    D_Channels = ['K-:loose pi+:loose',
                  'K-:loose pi+:loose pi+:loose pi-:loose',
                  'K-:loose pi+:loose pi0:loose',
                  'K_S0:all pi0:loose',
                  'K_S0:all pi+:loose pi-:loose',
                  ]

    DList = []
    for chID, channel in enumerate(D_Channels):
        reconstructDecay('D0:std' + str(chID) + ' -> ' + channel, Dcuts, chID)
        DList.append('D0:std' + str(chID))
    copyLists('D0:all', DList)
    return DList


def loadStdDplus(path=analysis_main):
    # stdCharged()
    # stdPi0()
    # stdKS()
    Dcuts = '1.82 < M < 1.92'
    D_Channels = ['K-:loose pi+:loose pi+:loose',
                  'K_S0:all pi+:loose',
                  'K_S0:all pi+:loose pi0:loose',
                  'K_S0:all pi+:loose pi+:loose pi-:loose',
                  ]

    DList = []
    for chID, channel in enumerate(D_Channels):
        reconstructDecay('D+:std' + str(chID) + ' -> ' + channel, Dcuts, chID)
        DList.append('D+:std' + str(chID))
    copyLists('D+:all', DList)
    return DList


def loadStdDstar0(path=analysis_main):
    # stdCharged()
    # stdPi0()
    # stdKS()
    Dcuts = '1.95 < M < 2.05'
    D_Channels = ['D0:all pi0:loose',
                  'D0:all gamma:loose']

    DList = []
    for chID, channel in enumerate(D_Channels):
        reconstructDecay('D*0:std' + str(chID) + ' -> ' + channel, Dcuts, chID)
        DList.append('D*0:std' + str(chID))
    copyLists('D*0:all', DList)
    return DList


def loadStdDstarPlus(path=analysis_main):
    # stdCharged()
    # stdPi0()
    # stdKS()
    Dcuts = '1.95 < M < 2.05'
    D_Channels = ['D0:all pi+:all',
                  'D+:all pi0:loose']

    DList = []
    for chID, channel in enumerate(D_Channels):
        reconstructDecay('D*+:std' + str(chID) + ' -> ' + channel, Dcuts, chID)
        DList.append('D*+:std' + str(chID))
    copyLists('D*+:all', DList)
    return DList
