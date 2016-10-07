#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# P. Urquijo, 15/Jan/2015
# G. Casarosa, 7/Oct/2016
#
######################################################

from basf2 import *
from modularAnalysis import *


def Charm2BodyCharged():
    charmcuts = '1.78 < M < 1.94'
    D0_Channels = ['K-:loose pi+:loose',
                   'pi+:loose pi-:loose',
                   'K+:loose K-:loose',
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        reconstructDecay('D0:Hadronic' + str(chID) + ' -> ' + channel, charmcuts, chID)
        D0List.append('D0:Hadronic' + str(chID))

    Lists = D0List
    return Lists
