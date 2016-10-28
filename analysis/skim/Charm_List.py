#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# P. Urquijo, 15/Jan/2015
# G. Casarosa, 7/Oct/2016
#
######################################################

# deprecated
# decided to split the many charm skims in more than one file

from basf2 import *
from modularAnalysis import *


def CharmRareList():
    charmcuts = '1.78 < M < 1.94'
    D0_Channels = ['gamma:all gamma:all',
                   'e+:loose e-:loose',
                   'mu+:loose mu-:loose pi0:all'
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:Rare' + str(chID) + ' -> ' + channel, charmcuts, chID)
        D0List.append('D0:Rare' + str(chID))

    Lists = D0List
    return Lists


def CharmHadronicList():
    charmcuts = '1.78 < M < 1.94'
    D0_Channels = ['K-:loose pi+:loose',
                   'pi+:loose pi-:loose',
                   'K+:loose K-:loose',
                   'K-:loose pi+:loose pi0:all'
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:Hadronic' + str(chID) + ' -> ' + channel, charmcuts, chID)
        D0List.append('D0:Hadronic' + str(chID))

    Lists = D0List
    return Lists
