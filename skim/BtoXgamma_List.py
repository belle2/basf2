#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skim list for B --> X gamma.
# EWP group.
#
# P. Urquijo, F. Tenchini  Jan 2015
# S. Cunliffe              Jan 2018
#
######################################################

from basf2 import *
from modularAnalysis import *
from BtoXInclusiveCommon import Xs0Modes, Xd0Modes, XsplusModes, XdplusModes


def B2XgammaList():
    """Build the skim list for B --> X(s,d) gamma decays"""

    # event level cuts: R2 and require a minimum number of tracks
    applyEventCuts('R2EventLevel < 0.7 and nTracks>4')

    # cuts in addition to the standard particle lists
    # should be revised for each new SPL release
    cutAndCopyList('pi0:ewp', 'pi0:skim', 'p > 0.20', True)
    applyCuts('K_S0:all', '0.4776 < M < 0.5176')  # 20 MeV width

    # invariant mass and dE windows for all modes
    btoxgammacuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.4'

    # B0 --> Xd0 gamma
    B02dgammaList = []
    for chID, channel in enumerate(Xd0Modes()):
        reconstructDecay('B0:EWP_b2dgamma' + str(chID) + ' -> ' + channel + ' gamma:E15', btoxgammacuts, chID, True)
        B02dgammaList.append('B0:EWP_b2dgamma' + str(chID))

    # B0 --> Xs0 gamma
    B02sgammaList = []
    for chID, channel in enumerate(Xs0Modes()):
        reconstructDecay('B0:EWP_b2sgamma' + str(chID) + ' -> ' + channel + ' gamma:E15', btoxgammacuts, chID, True)
        B02sgammaList.append('B0:EWP_b2sgamma' + str(chID))

    # B+ --> Xd+ gamma
    Bplus2dgammaList = []
    for chID, channel in enumerate(XdplusModes()):
        reconstructDecay('B+:EWP_b2dgamma' + str(chID) + ' -> ' + channel + ' gamma:E15', btoxgammacuts, chID, True)
        Bplus2dgammaList.append('B+:EWP_b2dgamma' + str(chID))

    # B+ --> Xs+ gamma
    Bplus2sgammaList = []
    for chID, channel in enumerate(XsplusModes()):
        reconstructDecay('B+:EWP_b2sgamma' + str(chID) + ' -> ' + channel + ' gamma:E15', btoxgammacuts, chID, True)
        Bplus2sgammaList.append('B+:EWP_b2sgamma' + str(chID))

    return B02dgammaList + B02sgammaList + Bplus2dgammaList + Bplus2sgammaList
