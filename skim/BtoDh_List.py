#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(*) h skims
# P. Urquijo, 6/Jan/2015
#
######################################################
from basf2 import *
from modularAnalysis import *


def BtoDhList():
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    B0_Channels = ['D+:all pi-:all',
                   'D*+:all pi-:all',
                   'D0:all pi0:all',
                   'D*0:all pi0:all',
                   'D+:all K-:all',
                   'D*+:all K-:all',
                   'D+:all K*-:loose',
                   'D*+:all K*-:loose',
                   'D0:all K_S0:all',
                   'D*0:all K_S0:all',
                   'D0:all K*0:loose',
                   'D*0:all K*0:loose'
                   ]

    B0List = []
    for chID, channel in enumerate(B0_Channels):
        reconstructDecay('B0:Dh' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0List.append('B0:Dh' + str(chID))

    Lists = B0List
    return Lists
