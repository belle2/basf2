#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# G. Casarosa, 7/Oct/2016
# L.K. Li, 23/Nov/2017
#
######################################################

from basf2 import *
from modularAnalysis import *


def DstToD0PiD0ToHpHmKs():

    charmcuts = '1.80 < M < 1.93 and useCMSFrame(p)>2'
    Dstcuts = '0 < Q < 0.022'

    D0_Channels = ['pi-:loose pi+:loose K_S0:all',
                   'K-:loose K+:loose K_S0:all'
                   ]
    DstList = []

    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, charmcuts, chID)
        vertexRave('D0:' + str(chID), 0.001)

        reconstructDecay('D*+:' + str(chID) + ' -> pi+:all D0:' + str(chID), Dstcuts, chID)
        vertexRave('D*+:' + str(chID), 0.001)
        DstList.append('D*+:' + str(chID))

    return DstList
