#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims

# G. Casarosa, 7/Oct/2016
#
######################################################

from basf2 import *
from modularAnalysis import *
# RETENTION RATE STILL A BIT HIGh. PLEASE MODIFY


def D0ToHpJmPi0():
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2.0'
    D0_Channels = ['K-:loose pi+:loose pi0:skim',
                   'pi+:loose pi-:loose pi0:skim',
                   'K+:loose K-:loose pi0:skim',
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        vertexRave('D0:' + str(chID), 0.001)
        D0List.append('D0:' + str(chID))

    Lists = D0List
    return Lists


def DstToD0PiD0ToHpJmPi0():

    Dstcuts = '0 < Q < 0.02'
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2.0'
    D0_Channels = ['K-:loose pi+:loose pi0:skim',
                   'pi+:loose pi-:loose pi0:skim',
                   'K+:loose K-:loose pi0:skim',
                   ]

    DstList = []

    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        vertexRave('D0:' + str(chID), 0.001)
        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all D0:' + str(chID), Dstcuts, chID)
        vertexRave('D*+:' + str(chID), 0.001)
        DstList.append('D*+:' + str(chID))

    return DstList
