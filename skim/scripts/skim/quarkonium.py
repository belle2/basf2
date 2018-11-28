#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for quarkonium analyses: bottomonium, charmonium, resonance """

__authors__ = [
    "Sen Jia"
    "..."
]

from basf2 import *
from modularAnalysis import *


def EtabList():
    Etabcuts = 'M > 7 and M < 10'
    fillParticleList('gamma:hard', 'E>3.5')
    applyCuts('gamma:hard', 'R2EventLevel < 0.995')

    Etab_Channels = ['gamma:hard gamma:hard']

    EtabList = []
    for chID, channel in enumerate(Etab_Channels):
        reconstructDecay('eta_b:all' + str(chID) + ' -> ' + channel, Etabcuts, chID)
        EtabList.append('eta_b:all' + str(chID))

    Lists = EtabList
    return Lists


def UpsilonList():
    Ycuts = ''
    fillParticleList('mu+:stiff', 'p<15 and p>3.5')
    fillParticleList('e+:stiff', 'p<15 and p>3.5')
    fillParticleList('pi+:soft', 'p<1.5 and pt>0.05')
    cutAndCopyList('gamma:soft', 'gamma:loose', 'E>0.15')
    reconstructDecay('Upsilon:ee -> e+:stiff e-:stiff', 'M > 8')
    reconstructDecay('Upsilon:mumu -> mu+:stiff mu-:stiff', 'M > 8')
    copyLists('Upsilon:all', ['Upsilon:ee', 'Upsilon:mumu'])

    applyCuts('Upsilon:all', 'R2EventLevel < 0.995')
    Upsilon_Channels = ['Upsilon:all pi+:soft',
                        'Upsilon:all gamma:soft']

    UpsilonList = []
    for chID, channel in enumerate(Upsilon_Channels):
        reconstructDecay('junction:all' + str(chID) + ' -> ' + channel, Ycuts, chID)

        UpsilonList.append('junction:all' + str(chID))

    Lists = UpsilonList
    return Lists


def ISRpipiccList(path):
    """
    Skim code: 16460100
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

    # intermediate state J/psi and psi(2S) are reconstructed
    # add mass window cut for J/psi and psi(2S) candidates
    reconstructDecay('J/psi:ee -> e+:loose e-:loose', 'M>3.0 and M<3.2', path=path)
    reconstructDecay('J/psi:mumu -> mu+:loose mu-:loose', 'M>3.0 and M<3.2', path=path)
    reconstructDecay('psi(2S):ee -> pi+:loose pi-:loose e+:loose e-:loose', 'M>3.64 and M<3.74', path=path)
    reconstructDecay('psi(2S):mumu -> pi+:loose pi-:loose mu+:loose mu-:loose', 'M>3.64 and M<3.74', path=path)

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
        reconstructDecay('vpho:myCombination' + str(chID) + ' -> ' + channel, MMScuts, chID, path=path)
        vphoList.append('vpho:myCombination' + str(chID))

    # return the ISR process list
    Lists = vphoList
    return Lists
