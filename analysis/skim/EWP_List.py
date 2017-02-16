#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# EWP skims
# P. Urquijo, F. Tenchini 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *


def B02XsModes():
    list = ['K+:ewp pi-:ewp',  # 1
            'K+:ewp pi-:ewp pi+:ewp pi-:ewp',  # 2
            'K+:ewp pi-:ewp pi0:ewp',  # 3
            'K+:ewp pi-:ewp pi0:ewp pi0:ewp',  # 4
            'K_S0:ewp pi+:ewp pi-:ewp',  # 5
            'K_S0:ewp pi+:ewp pi-:ewp pi+:ewp pi-:ewp',  # 6
            'K_S0:ewp pi0:ewp',  # 7
            'K_S0:ewp pi+:ewp pi-:ewp pi0:ewp',  # 8
            'K_S0:ewp pi0:ewp pi0:ewp',  # 9
            'K_S0:ewp pi+:ewp pi-:ewp pi0:ewp pi0:ewp',  # 10
            'K+:ewp pi-:ewp eta:ewp',  # 11
            'K_S0:ewp eta:ewp',  # 12
            'K_S0:ewp eta:ewp pi+:ewp pi-:ewp',  # 13
            'K_S0:ewp eta:ewp pi0:ewp',  # 14
            'K+:ewp K-:ewp K_S0:ewp',  # 15
            'K+:ewp K-:ewp K_S0:ewp pi0:ewp',  # 16
            'K+:ewp K+:ewp K-:ewp pi-:ewp',  # 17
            'phi:ewp'  # 18
            ]
    return list


def Bplus2XsModes():
    list = ['K+:ewp pi+:ewp pi-:ewp',  # 1
            'K+:ewp pi+:ewp pi-:ewp pi+:ewp pi-:ewp',  # 2
            'K+:ewp pi0:ewp',  # 3
            'K+:ewp pi+:ewp pi-:ewp pi0:ewp',  # 4
            'K+:ewp pi0:ewp pi0:ewp',  # 5
            'K+:ewp pi+:ewp pi-:ewp pi0:ewp pi0:ewp',  # 6
            'K_S0:ewp pi+:ewp',  # 7
            'K_S0:ewp pi+:ewp pi+:ewp pi-:ewp',  # 8
            'K_S0:ewp pi+:ewp pi0:ewp',  # 9
            'K_S0:ewp pi+:ewp pi0:ewp pi0:ewp',  # 10
            'K+:ewp eta:ewp',  # 11
            'K+:ewp eta:ewp pi0:ewp',  # 12
            'K+:ewp eta:ewp pi+:ewp pi-:ewp',  # 13
            'K_S0:ewp pi+:ewp eta:ewp',  # 14
            'K_S0:ewp pi+:ewp pi0:ewp eta:ewp',  # 15
            'K+:ewp K+:ewp K-:ewp',  # 16
            'K+:ewp K+:ewp K-:ewp pi0:ewp',  # 17
            'K+:ewp K-:ewp K_S0:ewp pi+:ewp'  # 18
            ]
    return list


def B02XdModes():
    list = ['rho0:ewp',  # 1
            'omega:ewp',  # 2
            'pi+:ewp pi-:ewp',  # 3
            'pi+:ewp pi-:ewp pi0:ewp',  # 4
            'pi+:ewp pi-:ewp pi0:ewp pi0:ewp'  # 5
            ]
    return list


def Bplus2XdModes():
    list = ['rho+:ewp',  # 1
            'pi+:ewp pi0:ewp',  # 2
            'pi+:ewp pi+:ewp pi-:ewp pi0:ewp',  # 3
            'pi+:ewp eta:ewp',  # 4
            'pi+:ewp pi+:ewp pi-:ewp'  # 5
            ]
    return list


def B2XgammaList():
    btoxgammacuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    B02dgammaList = []
    for chID, channel in enumerate(B02XdModes()):
        reconstructDecay('B0:EWP_b2dgamma' + str(chID) + ' -> ' + channel + ' gamma:loose', btoxgammacuts, chID, True)
        #
        buildRestOfEvent('B0:EWP_b2dgamma' + str(chID))
        buildContinuumSuppression('B0:EWP_b2dgamma' + str(chID))
        applyCuts('B0:EWP_b2dgamma' + str(chID), 'R2<0.5')
        #
        B02dgammaList.append('B0:EWP_b2dgamma' + str(chID))

    B02sgammaList = []
    for chID, channel in enumerate(B02XsModes()):
        reconstructDecay('B0:EWP_b2sgamma' + str(chID) + ' -> ' + channel + ' gamma:loose', btoxgammacuts, chID, True)
        #
        buildRestOfEvent('B0:EWP_b2sgamma' + str(chID))
        buildContinuumSuppression('B0:EWP_b2sgamma' + str(chID))
        applyCuts('B0:EWP_b2sgamma' + str(chID), 'R2<0.5')
        #
        B02sgammaList.append('B0:EWP_b2sgamma' + str(chID))

    Bplus2dgammaList = []
    for chID, channel in enumerate(Bplus2XdModes()):
        reconstructDecay('B+:EWP_b2dgamma' + str(chID) + ' -> ' + channel + ' gamma:loose', btoxgammacuts, chID, True)
        #
        buildRestOfEvent('B+:EWP_b2dgamma' + str(chID))
        buildContinuumSuppression('B+:EWP_b2dgamma' + str(chID))
        applyCuts('B+:EWP_b2dgamma' + str(chID), 'R2<0.5')
        #
        Bplus2dgammaList.append('B+:EWP_b2dgamma' + str(chID))

    Bplus2sgammaList = []
    for chID, channel in enumerate(Bplus2XsModes()):
        reconstructDecay('B+:EWP_b2sgamma' + str(chID) + ' -> ' + channel + ' gamma:loose', btoxgammacuts, chID, True)
        #
        buildRestOfEvent('B+:EWP_b2sgamma' + str(chID))
        buildContinuumSuppression('B+:EWP_b2sgamma' + str(chID))
        applyCuts('B+:EWP_b2sgamma' + str(chID), 'R2<0.5')
        #
        Bplus2sgammaList.append('B+:EWP_b2sgamma' + str(chID))

    return B02dgammaList + B02sgammaList + Bplus2dgammaList + Bplus2sgammaList


def B2XllList():
    btoxgammacuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    B02dllList = []
    for chID, channel in enumerate(B02XdModes()):
        reconstructDecay('B0:EWP_b2dee' + str(chID) + ' -> ' + channel + ' e-:ewp e+:ewp', btoxgammacuts, chID, True)
        reconstructDecay('B0:EWP_b2dmumu' + str(chID) + ' -> ' + channel + ' mu+:ewp mu-:ewp', btoxgammacuts, chID, True)
        #
        buildRestOfEvent('B0:EWP_b2dee' + str(chID))
        buildRestOfEvent('B0:EWP_b2dmumu' + str(chID))
        buildContinuumSuppression('B0:EWP_b2dee' + str(chID))
        buildContinuumSuppression('B0:EWP_b2dmumu' + str(chID))
        applyCuts('B0:EWP_b2dee' + str(chID), 'R2<0.5')
        applyCuts('B0:EWP_b2dmumu' + str(chID), 'R2<0.5')
        #
        B02dllList.append('B0:EWP_b2dee' + str(chID))
        B02dllList.append('B0:EWP_b2dmumu' + str(chID))

    B02sllList = []
    for chID, channel in enumerate(B02XsModes()):
        reconstructDecay('B0:EWP_b2see' + str(chID) + ' -> ' + channel + ' e-:ewp e+:ewp', btoxgammacuts, chID, True)
        reconstructDecay('B0:EWP_b2smumu' + str(chID) + ' -> ' + channel + ' mu+:ewp mu-:ewp', btoxgammacuts, chID, True)
        #
        buildRestOfEvent('B0:EWP_b2see' + str(chID))
        buildRestOfEvent('B0:EWP_b2smumu' + str(chID))
        buildContinuumSuppression('B0:EWP_b2see' + str(chID))
        buildContinuumSuppression('B0:EWP_b2smumu' + str(chID))
        applyCuts('B0:EWP_b2see' + str(chID), 'R2<0.5')
        applyCuts('B0:EWP_b2smumu' + str(chID), 'R2<0.5')
        #
        B02sllList.append('B0:EWP_b2see' + str(chID))
        B02sllList.append('B0:EWP_b2smumu' + str(chID))

    Bplus2dllList = []
    for chID, channel in enumerate(Bplus2XdModes()):
        reconstructDecay('B-:EWP_b2dee' + str(chID) + ' -> ' + channel + ' e-:ewp e+:ewp', btoxgammacuts, chID, True)
        reconstructDecay('B-:EWP_b2dmumu' + str(chID) + ' -> ' + channel + ' mu+:ewp mu-:ewp', btoxgammacuts, chID, True)
        #
        buildRestOfEvent('B-:EWP_b2dee' + str(chID))
        buildRestOfEvent('B-:EWP_b2dmumu' + str(chID))
        buildContinuumSuppression('B-:EWP_b2dee' + str(chID))
        buildContinuumSuppression('B-:EWP_b2dmumu' + str(chID))
        applyCuts('B-:EWP_b2dee' + str(chID), 'R2<0.5')
        applyCuts('B-:EWP_b2dmumu' + str(chID), 'R2<0.5')
        #
        Bplus2dllList.append('B-:EWP_b2dee' + str(chID))
        Bplus2dllList.append('B-:EWP_b2dmumu' + str(chID))

    Bplus2sllList = []
    for chID, channel in enumerate(Bplus2XsModes()):
        reconstructDecay('B-:EWP_b2see' + str(chID) + ' -> ' + channel + ' e-:ewp e+:ewp', btoxgammacuts, chID, True)
        reconstructDecay('B-:EWP_b2smumu' + str(chID) + ' -> ' + channel + ' mu+:ewp mu-:ewp', btoxgammacuts, chID, True)
        #
        buildRestOfEvent('B-:EWP_b2see' + str(chID))
        buildRestOfEvent('B-:EWP_b2smumu' + str(chID))
        buildContinuumSuppression('B-:EWP_b2see' + str(chID))
        buildContinuumSuppression('B-:EWP_b2smumu' + str(chID))
        applyCuts('B-:EWP_b2see' + str(chID), 'R2<0.5')
        applyCuts('B-:EWP_b2smumu' + str(chID), 'R2<0.5')
        #
        Bplus2sllList.append('B-:EWP_b2see' + str(chID))
        Bplus2sllList.append('B-:EWP_b2smumu' + str(chID))

    return B02dllList + B02sllList + Bplus2dllList + Bplus2sllList
