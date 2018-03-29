#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmonium skims
# Sen Jia, 21/Mar/2018
#
######################################################

from basf2 import *
from modularAnalysis import *


def ISRpipiccList():
    fillParticleList('gamma:tight', '')
    fillParticleList('e+:loose', '')
    fillParticleList('mu+:loose', '')
    fillParticleList('pi+:loose', '')
    rankByHighest('gamma:tight', 'E', 1)
    reconstructDecay('J/psi:ee -> e+:loose e-:loose', 'M>3.0 and M<3.2')
    reconstructDecay('J/psi:mumu -> mu+:loose mu-:loose', 'M>3.0 and M<3.2')
    reconstructDecay('psi(2S):ee -> pi+:loose pi-:loose e+:loose e-:loose', 'M>3.64 and M<3.74')
    reconstructDecay('psi(2S):mumu -> pi+:loose pi-:loose mu+:loose mu-:loose', 'M>3.64 and M<3.74')
    vpho_Channels = [
                   'gamma:tight pi+:loose pi-:loose J/psi:ee',
                   'gamma:tight pi+:loose pi-:loose J/psi:mumu',
                   'gamma:tight pi+:loose pi-:loose psi(2S):ee',
                   'gamma:tight pi+:loose pi-:loose psi(2S):mumu'
                   ]
    vphoList = []
    for chID, channel in enumerate(vpho_Channels):
        reconstructDecay('vpho:myCombination' + str(chID) + ' -> ' + channel, '', chID)
        vphoList.append('vpho:myCombination' + str(chID))

    Lists = vphoList
    return Lists
