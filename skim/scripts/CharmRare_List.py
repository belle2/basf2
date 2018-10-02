#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# P. Urquijo, 15/Jan/2015
# G. Casarosa, 7/Oct/2016
# L.K. Li, 23/Nov/2017
#
######################################################

from basf2 import *
from modularAnalysis import *


def CharmRareList():
    charmcuts = '1.7 < M < 2.0  and useCMSFrame(p)>2'
    D0_Channels = ['gamma:skim gamma:skim',
                   'e+:loose e-:loose',
                   'mu+:loose mu-:loose',
                   'e+:loose e-:loose pi0:skim',
                   'mu+:loose mu-:loose pi0:skim'
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:Rare' + str(chID) + ' -> ' + channel, charmcuts, chID)
        D0List.append('D0:Rare' + str(chID))

    Lists = D0List
    return Lists
