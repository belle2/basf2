#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmonium skim
# Sen Jia, 11/Jun/2018
# This skim script reconstructs four ISR processes
# (1) e+e- -> gamma_{ISR} pi+ pi- J/psi -> e+e-
# (2) e+e- -> gamma_{ISR} pi+ pi- J/psi -> mu+mu-
# (3) e+e- -> gamma_{ISR} pi+ pi- psi(2S) -> pi+ pi- J/psi -> e+e-
# (4) e+e- -> gamma_{ISR} pi+ pi- psi(2S) -> pi+ pi- J/psi -> mu+mu-
#
######################################################

from basf2 import *
from modularAnalysis import *


def ISRpipiccList():

    # create and fill standard gamma/e/mu/pi ParticleLists
    fillParticleList('gamma:tight', '')
    fillParticleList('e+:loose', '')
    fillParticleList('mu+:loose', '')
    fillParticleList('pi+:loose', '')

    # fill the maximum energy photon as ISR photon
    rankByHighest('gamma:tight', 'E', 1)

    # intermediate state J/psi and psi(2S) are reconstructed
    # add mass window cut for J/psi and psi(2S) candidates
    reconstructDecay('J/psi:ee -> e+:loose e-:loose', 'M>3.0 and M<3.2')
    reconstructDecay('J/psi:mumu -> mu+:loose mu-:loose', 'M>3.0 and M<3.2')
    reconstructDecay('psi(2S):ee -> pi+:loose pi-:loose e+:loose e-:loose', 'M>3.64 and M<3.74')
    reconstructDecay('psi(2S):mumu -> pi+:loose pi-:loose mu+:loose mu-:loose', 'M>3.64 and M<3.74')

    # four ISR modes are reconstructed
    # e+e- -> gamma_{ISR} pi+ pi- J/psi -> e+e-
    # e+e- -> gamma_{ISR} pi+ pi- J/psi -> mu+mu-
    # e+e- -> gamma_{ISR} pi+ pi- psi(2S) -> pi+ pi- J/psi -> e+e-
    # e+e- -> gamma_{ISR} pi+ pi- psi(2S) -> pi+ pi- J/psi -> mu+mu-
    vpho_Channels = [
        'gamma:tight pi+:loose pi-:loose J/psi:ee',
        'gamma:tight pi+:loose pi-:loose J/psi:mumu',
        'gamma:tight pi+:loose pi-:loose psi(2S):ee',
        'gamma:tight pi+:loose pi-:loose psi(2S):mumu'
    ]

    # define the ISR process list
    vphoList = []

    # reconstruct the different ISR channels and append to the virtual photon
    for chID, channel in enumerate(vpho_Channels):
        reconstructDecay('vpho:myCombination' + str(chID) + ' -> ' + channel, '', chID)
        vphoList.append('vpho:myCombination' + str(chID))

    # return the ISR process list
    Lists = vphoList
    return Lists
