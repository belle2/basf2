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
# VERY LARGE RETENTION RATE: Please modify


def D0ToHpJmPi0():
    charmcuts = '1.78 < M < 1.94 and useCMSFrame(p)>2'
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
