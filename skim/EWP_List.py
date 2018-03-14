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
    list = ['K+:95eff pi-:95eff',  # 1
            'K+:95eff pi-:95eff pi+:95eff pi-:95eff',  # 2
            'K+:95eff pi-:95eff pi0:loose',  # 3
            'K+:95eff pi-:95eff pi0:loose pi0:loose',  # 4
            'K_S0:all pi+:95eff pi-:95eff',  # 5
            'K_S0:all pi+:95eff pi-:95eff pi+:95eff pi-:95eff',  # 6
            'K_S0:all pi0:loose',  # 7
            'K_S0:all pi+:95eff pi-:95eff pi0:loose',  # 8
            'K_S0:all pi0:loose pi0:loose',  # 9
            'K_S0:all pi+:95eff pi-:95eff pi0:loose pi0:loose',  # 10
            'K+:95eff pi-:95eff eta:loose',  # 11
            'K_S0:all eta:loose',  # 12
            'K_S0:all eta:loose pi+:95eff pi-:95eff',  # 13
            'K_S0:all eta:loose pi0:loose',  # 14
            'K+:95eff K-:95eff K_S0:all',  # 15
            'K+:95eff K-:95eff K_S0:all pi0:loose',  # 16
            'K+:95eff K+:95eff K-:95eff pi-:95eff',  # 17
            'phi:loose'  # 18
            ]
    return list


def Bplus2XsModes():
    list = ['K+:95eff pi+:95eff pi-:95eff',  # 1
            'K+:95eff pi+:95eff pi-:95eff pi+:95eff pi-:95eff',  # 2
            'K+:95eff pi0:loose',  # 3
            'K+:95eff pi+:95eff pi-:95eff pi0:loose',  # 4
            'K+:95eff pi0:loose pi0:loose',  # 5
            'K+:95eff pi+:95eff pi-:95eff pi0:loose pi0:loose',  # 6
            'K_S0:all pi+:95eff',  # 7
            'K_S0:all pi+:95eff pi+:95eff pi-:95eff',  # 8
            'K_S0:all pi+:95eff pi0:loose',  # 9
            'K_S0:all pi+:95eff pi0:loose pi0:loose',  # 10
            'K+:95eff eta:loose',  # 11
            'K+:95eff eta:loose pi0:loose',  # 12
            'K+:95eff eta:loose pi+:95eff pi-:95eff',  # 13
            'K_S0:all pi+:95eff eta:loose',  # 14
            'K_S0:all pi+:95eff pi0:loose eta:loose',  # 15
            'K+:95eff K+:95eff K-:95eff',  # 16
            'K+:95eff K+:95eff K-:95eff pi0:loose',  # 17
            'K+:95eff K-:95eff K_S0:all pi+:95eff'  # 18
            ]
    return list


def B02XdModes():
    list = ['rho0:loose',  # 1
            'omega:loose',  # 2
            'pi+:95eff pi-:95eff',  # 3
            'pi+:95eff pi-:95eff pi0:loose',  # 4
            'pi+:95eff pi-:95eff pi0:loose pi0:loose'  # 5
            ]
    return list


def Bplus2XdModes():
    list = ['rho+:loose',  # 1
            'pi+:95eff pi0:loose',  # 2
            'pi+:95eff pi+:95eff pi-:95eff pi0:loose',  # 3
            'pi+:95eff eta:loose',  # 4
            'pi+:95eff pi+:95eff pi-:95eff'  # 5
            ]
    return list


def B2XgammaList():
    btoxgammacuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'
    applyCuts('K_S0:all', '0.4776 < M < 0.5176')  # 20 MeV width

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
    applyCuts('K_S0:all', '0.4776 < M < 0.5176')  # 20 MeV width
    # loose cut because we don't have proper benchmarks yet
    applyCuts('mu+:all', 'pt > 0.7 and abs(d0)<2 and abs(z0)<4 and chiProb > 0.001')
    # loose cut because we don't have proper benchmarks yet
    applyCuts('e+:all', 'pt > 0.4 and abs(d0)<2 and abs(z0)<4 and chiProb > 0.001')

    B02dllList = []
    for chID, channel in enumerate(B02XdModes()):
        reconstructDecay('B0:EWP_b2dee' + str(chID) + ' -> ' + channel + ' e-:all e+:all', btoxgammacuts, chID, True)
        reconstructDecay('B0:EWP_b2dmumu' + str(chID) + ' -> ' + channel + ' mu+:all mu-:all', btoxgammacuts, chID, True)
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
        reconstructDecay('B0:EWP_b2see' + str(chID) + ' -> ' + channel + ' e-:all e+:all', btoxgammacuts, chID, True)
        reconstructDecay('B0:EWP_b2smumu' + str(chID) + ' -> ' + channel + ' mu+:all mu-:all', btoxgammacuts, chID, True)
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
        reconstructDecay('B-:EWP_b2dee' + str(chID) + ' -> ' + channel + ' e-:all e+:all', btoxgammacuts, chID, True)
        reconstructDecay('B-:EWP_b2dmumu' + str(chID) + ' -> ' + channel + ' mu+:all mu-:all', btoxgammacuts, chID, True)
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
        reconstructDecay('B-:EWP_b2see' + str(chID) + ' -> ' + channel + ' e-:all e+:all', btoxgammacuts, chID, True)
        reconstructDecay('B-:EWP_b2smumu' + str(chID) + ' -> ' + channel + ' mu+:all mu-:all', btoxgammacuts, chID, True)
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
