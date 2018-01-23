#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skim list for B --> X l+ l- (l=e, mu).
# EWP group.
#
# P. Urquijo   Jan 2015
# S. Cunliffe  Jan 2018
#
######################################################

from basf2 import *
from modularAnalysis import *
from BtoXInclusiveCommon import Xs0Modes, Xd0Modes, XsplusModes, XdplusModes


def B2XllList():
    """Build the skim list for B --> X(s,d) l+ l- decays"""

    # event level cuts: R2 and require a minimum number of tracks
    applyEventCuts('R2EventLevel < 0.7 and nTracks>4')

    # cuts in addition to the standard particle lists
    # should be revised for each new SPL release
    cutAndCopyList('pi0:ewp', 'pi0:loose', 'p > 0.2', True)
    applyCuts('K_S0:all', '0.4776 < M < 0.5176')  # 20 MeV width
    cutAndCopyList('e+:ewp', 'e+:loose', 'pt > 0.4', True)
    cutAndCopyList('mu+:ewp', 'mu+:loose', 'pt > 0.5', True)

    # invariant mass and dE windows for all modes
    btoxgammacuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    # B0 --> Xd0 l+ l-
    B02dllList = []
    for chID, channel in enumerate(Xd0Modes()):
        reconstructDecay('B0:EWP_b2dee' + str(chID) + ' -> ' + channel + ' e-:ewp e+:ewp', btoxgammacuts, chID, True)
        reconstructDecay('B0:EWP_b2dmumu' + str(chID) + ' -> ' + channel + ' mu+:ewp mu-:ewp', btoxgammacuts, chID, True)
        B02dllList.append('B0:EWP_b2dee' + str(chID))
        B02dllList.append('B0:EWP_b2dmumu' + str(chID))

    # B0 --> Xs0 l+ l-
    B02sllList = []
    for chID, channel in enumerate(Xs0Modes()):
        reconstructDecay('B0:EWP_b2see' + str(chID) + ' -> ' + channel + ' e-:ewp e+:ewp', btoxgammacuts, chID, True)
        reconstructDecay('B0:EWP_b2smumu' + str(chID) + ' -> ' + channel + ' mu+:ewp mu-:ewp', btoxgammacuts, chID, True)
        B02sllList.append('B0:EWP_b2see' + str(chID))
        B02sllList.append('B0:EWP_b2smumu' + str(chID))

    # B+ --> Xd+ l+ l-
    Bplus2dllList = []
    for chID, channel in enumerate(XdplusModes()):
        reconstructDecay('B-:EWP_b2dee' + str(chID) + ' -> ' + channel + ' e-:ewp e+:ewp', btoxgammacuts, chID, True)
        reconstructDecay('B-:EWP_b2dmumu' + str(chID) + ' -> ' + channel + ' mu+:ewp mu-:ewp', btoxgammacuts, chID, True)
        Bplus2dllList.append('B-:EWP_b2dee' + str(chID))
        Bplus2dllList.append('B-:EWP_b2dmumu' + str(chID))

    # B+ --> Xs+ l+ l-
    Bplus2sllList = []
    for chID, channel in enumerate(XsplusModes()):
        reconstructDecay('B-:EWP_b2see' + str(chID) + ' -> ' + channel + ' e-:ewp e+:ewp', btoxgammacuts, chID, True)
        reconstructDecay('B-:EWP_b2smumu' + str(chID) + ' -> ' + channel + ' mu+:ewp mu-:ewp', btoxgammacuts, chID, True)
        Bplus2sllList.append('B-:EWP_b2see' + str(chID))
        Bplus2sllList.append('B-:EWP_b2smumu' + str(chID))

    return B02dllList + B02sllList + Bplus2dllList + Bplus2sllList
