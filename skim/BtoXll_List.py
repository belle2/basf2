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


def B2XllList():
    applyEventCuts('nTracks>4')
    cutAndCopyList('pi0:ewp', 'pi0:loose', 'p > 0.1', True)
    cutAndCopyList('e+:ewp', 'e+:loose', 'pt > 0.4', True)
    cutAndCopyList('mu+:ewp', 'mu+:loose', 'pt > 0.5', True)
    btoxgammacuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.4'

    B02dllList = []
    for chID, channel in enumerate(B02XdModes()):
        reconstructDecay('B0:EWP_b2dee' + str(chID) + ' -> ' + channel + ' e-:ewp e+:ewp', btoxgammacuts, chID, True)
        reconstructDecay('B0:EWP_b2dmumu' + str(chID) + ' -> ' + channel + ' mu+:ewp mu-:ewp', btoxgammacuts, chID, True)
        B02dllList.append('B0:EWP_b2dee' + str(chID))
        B02dllList.append('B0:EWP_b2dmumu' + str(chID))

    B02sllList = []
    for chID, channel in enumerate(B02XsModes()):
        reconstructDecay('B0:EWP_b2see' + str(chID) + ' -> ' + channel + ' e-:ewp e+:ewp', btoxgammacuts, chID, True)
        reconstructDecay('B0:EWP_b2smumu' + str(chID) + ' -> ' + channel + ' mu+:ewp mu-:ewp', btoxgammacuts, chID, True)
        B02sllList.append('B0:EWP_b2see' + str(chID))
        B02sllList.append('B0:EWP_b2smumu' + str(chID))

    Bplus2dllList = []
    for chID, channel in enumerate(Bplus2XdModes()):
        reconstructDecay('B-:EWP_b2dee' + str(chID) + ' -> ' + channel + ' e-:ewp e+:ewp', btoxgammacuts, chID, True)
        reconstructDecay('B-:EWP_b2dmumu' + str(chID) + ' -> ' + channel + ' mu+:ewp mu-:ewp', btoxgammacuts, chID, True)
        Bplus2dllList.append('B-:EWP_b2dee' + str(chID))
        Bplus2dllList.append('B-:EWP_b2dmumu' + str(chID))

    Bplus2sllList = []
    for chID, channel in enumerate(Bplus2XsModes()):
        reconstructDecay('B-:EWP_b2see' + str(chID) + ' -> ' + channel + ' e-:ewp e+:ewp', btoxgammacuts, chID, True)
        reconstructDecay('B-:EWP_b2smumu' + str(chID) + ' -> ' + channel + ' mu+:ewp mu-:ewp', btoxgammacuts, chID, True)
        Bplus2sllList.append('B-:EWP_b2see' + str(chID))
        Bplus2sllList.append('B-:EWP_b2smumu' + str(chID))

    return B02dllList + B02sllList + Bplus2dllList + Bplus2sllList
