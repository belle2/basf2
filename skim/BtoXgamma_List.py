#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# EWP skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *


def B02XsModes():
    list = ['K+:loose pi-:loose',  # 1
            'K+:loose pi-:loose pi+:loose pi-:loose',  # 2
            'K+:loose pi-:loose pi0:ewp',  # 3
            'K+:loose pi-:loose pi0:ewp pi0:ewp',  # 4
            'K_S0:all pi+:loose pi-:loose',  # 5
            'K_S0:all pi+:loose pi-:loose pi+:loose pi-:loose',  # 6
            'K_S0:all pi0:ewp',  # 7
            'K_S0:all pi+:loose pi-:loose pi0:ewp',  # 8
            'K_S0:all pi0:ewp pi0:ewp',  # 9
            'K_S0:all pi+:loose pi-:loose pi0:ewp pi0:ewp',  # 10
            'K+:loose pi-:loose eta:loose',  # 11
            'K_S0:all eta:loose',  # 12
            'K_S0:all eta:loose pi+:loose pi-:loose',  # 13
            'K_S0:all eta:loose pi0:ewp',  # 14
            'K+:loose K-:loose K_S0:all',  # 15
            'K+:loose K-:loose K_S0:all pi0:ewp',  # 16
            'K+:loose K+:loose K-:loose pi-:loose',  # 17
            'phi:loose'  # 18
            ]
    return list


def Bplus2XsModes():
    list = ['K+:loose pi+:loose pi-:loose',  # 1
            'K+:loose pi+:loose pi-:loose pi+:loose pi-:loose',  # 2
            'K+:loose pi0:ewp',  # 3
            'K+:loose pi+:loose pi-:loose pi0:ewp',  # 4
            'K+:loose pi0:ewp pi0:ewp',  # 5
            'K+:loose pi+:loose pi-:loose pi0:ewp pi0:ewp',  # 6
            'K_S0:all pi+:loose',  # 7
            'K_S0:all pi+:loose pi+:loose pi-:loose',  # 8
            'K_S0:all pi+:loose pi0:ewp',  # 9
            'K_S0:all pi+:loose pi0:ewp pi0:ewp',  # 10
            'K+:loose eta:loose',  # 11
            'K+:loose eta:loose pi0:ewp',  # 12
            'K+:loose eta:loose pi+:loose pi-:loose',  # 13
            'K_S0:all pi+:loose eta:loose',  # 14
            'K_S0:all pi+:loose pi0:ewp eta:loose',  # 15
            'K+:loose K+:loose K-:loose',  # 16
            'K+:loose K+:loose K-:loose pi0:ewp',  # 17
            'K+:loose K-:loose K_S0:all pi+:loose'  # 18
            ]
    return list


def B02XdModes():
    list = ['rho0:loose',  # 1
            'omega:loose',  # 2
            'pi+:loose pi-:loose',  # 3
            'pi+:loose pi-:loose pi0:loose',  # 4
            'pi+:loose pi-:loose pi0:loose pi0:loose'  # 5
            ]
    return list


def Bplus2XdModes():
    list = ['rho+:loose',  # 1
            'pi+:loose pi0:ewp',  # 2
            'pi+:loose pi+:loose pi-:loose pi0:ewp',  # 3
            'pi+:loose eta:loose',  # 4
            'pi+:loose pi+:loose pi-:loose'  # 5
            ]
    return list


def B2XgammaList():
    cutAndCopyList('pi0:ewp', 'pi0:skim', 'p > 0.1', True)
    btoxgammacuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    B02dgammaList = []
    for chID, channel in enumerate(B02XdModes()):
        reconstructDecay('B0:EWP_b2dgamma' + str(chID) + ' -> ' + channel + ' gamma:E15', btoxgammacuts, chID, True)
        applyCuts('B0:EWP_b2dgamma' + str(chID), 'nTracks>4')
        B02dgammaList.append('B0:EWP_b2dgamma' + str(chID))

    B02sgammaList = []
    for chID, channel in enumerate(B02XsModes()):
        reconstructDecay('B0:EWP_b2sgamma' + str(chID) + ' -> ' + channel + ' gamma:E15', btoxgammacuts, chID, True)
        applyCuts('B0:EWP_b2sgamma' + str(chID), 'nTracks>4')
        B02sgammaList.append('B0:EWP_b2sgamma' + str(chID))

    Bplus2dgammaList = []
    for chID, channel in enumerate(Bplus2XdModes()):
        reconstructDecay('B+:EWP_b2dgamma' + str(chID) + ' -> ' + channel + ' gamma:E15', btoxgammacuts, chID, True)
        applyCuts('B+:EWP_b2dgamma' + str(chID), 'nTracks>4')
        Bplus2dgammaList.append('B+:EWP_b2dgamma' + str(chID))

    Bplus2sgammaList = []
    for chID, channel in enumerate(Bplus2XsModes()):
        reconstructDecay('B+:EWP_b2sgamma' + str(chID) + ' -> ' + channel + ' gamma:E15', btoxgammacuts, chID, True)
        applyCuts('B+:EWP_b2sgamma' + str(chID), 'nTracks>4')
        Bplus2sgammaList.append('B+:EWP_b2sgamma' + str(chID))

    return B02dgammaList + B02sgammaList + Bplus2dgammaList + Bplus2sgammaList
