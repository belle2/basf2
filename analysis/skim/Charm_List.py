#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# P. Urquijo, 15/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *


def CharmRareList():
    charmcuts = '1.82 < M < 1.92'
    D0_Channels = ['gamma:all gamma:all',
                   'e+:loose e-:loose'
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:Rare' + str(chID) + ' -> ' + channel, charmcuts, chID)
        D0List.append('D0:Rare' + str(chID))

    Lists = D0List
    return Lists
