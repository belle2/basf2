#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Double charm skims
# P. Urquijo, 6/Jan/2015
#
######################################################
from basf2 import *
from modularAnalysis import *

# VERY HIGH RETENTION RATE: NEEDS TO BE REDUCED


def DoubleCharmList():
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    B0_Channels = ['D0:all D0:all',
                   'D0:all D*0:all',
                   'D*0:all D0:all',
                   'D*0:all D*0:all']

    B0List = []
    for chID, channel in enumerate(B0_Channels):
        reconstructDecay('B0:DC' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0List.append('B0:DC' + str(chID))

    Lists = B0List
    return Lists
