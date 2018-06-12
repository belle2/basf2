#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmonium skim
# Sen Jia, 11/Jun/2018
#
######################################################

from basf2 import *
from modularAnalysis import *


def ISRpipiccList():
    """
    Skim selection of the following channels:
    - e+e- -> pi+ pi- J/psi -> e+e-
    - e+e- -> pi+ pi- J/psi -> mu+mu-
    - e+e- -> pi+ pi- psi(2S) -> pi+ pi- J/psi -> e+e-
    - e+e- -> pi+ pi- psi(2S) -> pi+ pi- J/psi -> mu+mu-
    selection criteria are listed below
    (1) standard e/mu/pi ParticleLists
    (2) mass window cut for J/psi and psi(2S) candidates
    (3) Apply -4 < the recoil mass square of hadrons < 4 GeV^{2}/c^{4} to extract ISR signal events
    """

    # create and fill standard e/mu/pi ParticleLists
    fillParticleList('e+:loose', '')
    fillParticleList('mu+:loose', '')
    fillParticleList('pi+:loose', '')

    # intermediate state J/psi and psi(2S) are reconstructed
    # add mass window cut for J/psi and psi(2S) candidates
    reconstructDecay('J/psi:ee -> e+:loose e-:loose', 'M>3.0 and M<3.2')
    reconstructDecay('J/psi:mumu -> mu+:loose mu-:loose', 'M>3.0 and M<3.2')
    reconstructDecay('psi(2S):ee -> pi+:loose pi-:loose e+:loose e-:loose', 'M>3.64 and M<3.74')
    reconstructDecay('psi(2S):mumu -> pi+:loose pi-:loose mu+:loose mu-:loose', 'M>3.64 and M<3.74')

    # the requirement of recoil mass square of hadrons
    MMScuts = '-4 < m2Recoil < 4'

    # four ISR modes are reconstructed
    # e+e- -> pi+ pi- J/psi -> e+e- via ISR
    # e+e- -> pi+ pi- J/psi -> mu+mu- via ISR
    # e+e- -> pi+ pi- psi(2S) -> pi+ pi- J/psi -> e+e- via ISR
    # e+e- -> pi+ pi- psi(2S) -> pi+ pi- J/psi -> mu+mu- via ISR
    vpho_Channels = [
        'pi+:loose pi-:loose J/psi:ee',
        'pi+:loose pi-:loose J/psi:mumu',
        'pi+:loose pi-:loose psi(2S):ee',
        'pi+:loose pi-:loose psi(2S):mumu'
    ]

    # define the ISR process list
    vphoList = []

    # reconstruct the different ISR channels and append to the virtual photon
    for chID, channel in enumerate(vpho_Channels):
        reconstructDecay('vpho:myCombination' + str(chID) + ' -> ' + channel, MMScuts, chID)
        vphoList.append('vpho:myCombination' + str(chID))

    # return the ISR process list
    Lists = vphoList
    return Lists
