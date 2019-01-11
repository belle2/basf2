#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for quarkonium analyses: bottomonium, charmonium, resonance """

__authors__ = [
    "Stefano Spataro"
    "Sen Jia"
    "..."
]

from basf2 import *
from modularAnalysis import *


def EtabList(path):
    """
    Skim code: 15420200
    Skim selection of the following channel:
    - eta_b -> gamma gamma
    selection criteria are listed below
    (1) 2 std photon with E > 3.5 GeV
    (2) 7 < M(eta_b) < 10 GeV/c^2
    (3) R2 < 0.995
    """
    __author__ = "Stefano Spataro & Sen Jia"

    # create and fill hard photon
    cutAndCopyList('gamma:hard', 'gamma:loose', 'E>3.5', path=path)
    applyCuts('gamma:hard', 'R2EventLevel < 0.995', path=path)

    # the requirement of 7 < M(eta_b) < 10 GeV/c2
    Etabcuts = 'M > 7 and M < 10'

    # eta_b candidates are reconstructed
    Etab_Channels = ['gamma:hard gamma:hard']

    # define the eta_b decay list
    EtabList = []

    # reconstruct the decay eta_b -> gamma gamma
    for chID, channel in enumerate(Etab_Channels):
        reconstructDecay('eta_b:all' + str(chID) + ' -> ' + channel, Etabcuts, chID, path=path)
        EtabList.append('eta_b:all' + str(chID))

    # return the eta_b decaylist
    return EtabList


def UpsilonList(path):
    """
    Skim code: 15440100
    Skim selection of the following channel:
    - Y(1S,2S) -> l^+ l^{-} (l = e or mu)
    selection criteria are listed below
    (1) 2 tracks with momentum ranging between 3.5 < p < 15,
    (2) At least 1 track p < 1.5 or 1 std photon with E > 150 MeV
    (3) M(Y(1S,2S)) > 8 GeV/c^2
    (4) R2 < 0.995
    """
    __author__ = "Stefano Spataro & Sen Jia"

    Ycuts = ''
    # create and fill e/mu/pi/photon ParticleLists
    fillParticleList('mu+:loose', 'p<15 and p>3.5', path=path)
    fillParticleList('e+:loose', 'p<15 and p>3.5', path=path)
    fillParticleList('pi+:loose', 'p<1.5 and pt>0.05', path=path)
    cutAndCopyList('gamma:soft', 'gamma:loose', 'E>0.15', path=path)

    # Y(1S,2S) are reconstructed with e^+ e^- or mu^+ mu^-
    reconstructDecay('Upsilon:ee -> e+:loose e-:loose', 'M > 8', path=path)
    reconstructDecay('Upsilon:mumu -> mu+:loose mu-:loose', 'M > 8', path=path)
    copyLists('Upsilon:all', ['Upsilon:ee', 'Upsilon:mumu'], path=path)

    # require R2 < 0.995
    applyCuts('Upsilon:all', 'R2EventLevel < 0.995', path=path)

    # Y(1S,2S) with pi+ or photon are reconstructed
    Upsilon_Channels = ['Upsilon:all pi+:loose',
                        'Upsilon:all gamma:soft']

    # define the Y(1S,2S) decay channel list
    UpsilonList = []

    # reconstruct the decay channel
    for chID, channel in enumerate(Upsilon_Channels):
        reconstructDecay('junction:all' + str(chID) + ' -> ' + channel, Ycuts, chID, path=path)
        UpsilonList.append('junction:all' + str(chID))

    # reture the list
    return UpsilonList


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
    __author__ = "Sen Jia"

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
    return vphoList
