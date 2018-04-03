#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skim list for B --> X gamma.
# EWP group.
#
# P. Urquijo, F. Tenchini  Jan 2015
# S. Cunliffe and A. Ishkikawa  Feb 2018
#
######################################################

from basf2 import *
from modularAnalysis import *
from BtoXInclusiveCommon import Xs0Modes, Xd0Modes, XsplusModes, XdplusModes


def B2XgammaList():
    """Build the skim list for B --> X(s,d) gamma decays"""

    # event level cuts: R2 and require a minimum number of tracks + decent photons
    applyEventCuts((
        'R2EventLevel < 0.8 and ',
        'formula((nTracks + nParticlesInList(gamma:loose) / 2)) > 5')
    )
    #
    # cuts in addition to the standard particle lists
    # should be revised for each new SPL release
    cutAndCopyList('pi+:ewpHigh', 'pi+:95eff', 'p > 0.25')
    cutAndCopyList('pi+:ewp2High', 'pi+:95eff', 'p > 0.10')
    #
    cutAndCopyList('pi0:ewp', 'pi0:skim', 'p > 0.20 and 0.115 < M < 0.145')
    cutAndCopyList('pi0:ewpHigh', 'pi0:skim', 'p > 0.40 and 0.115 < M < 0.145')
    cutAndCopyList('K_S0:ewp', 'K_S0:all', '0.4776 < M < 0.5176')  # 20 MeV width
    #
    # take the loose stdPhotons SPL and require a bit of energy for eta candidates
    cutAndCopyList('gamma:ewp', 'gamma:loose', 'E > 0.1')
    reconstructDecay('eta:ewp -> gamma:ewp gamma:ewp', '0.505 < M < 0.580')
    #
    # take the tight stdPhotons SPL (timing cuts dependent on regions) and add
    # a minimum lab-frame energy requirement (1.5 GeV) and cluster shape e9oe21
    cutAndCopyList('gamma:ewpE15', 'gamma:tight', 'clusterE9E21 > 0.9 and E > 1.5')
    #
    # invariant mass and dE windows for all modes
    btoxgammacuts = '5.2 < Mbc < 5.29 and -0.5 < deltaE < 0.3'

    # B0 --> Xd0 gamma
    B02dgammaList = []
    for chID, channel in enumerate(Xd0Modes()):
        reconstructDecay('B0:EWP_b2dgamma' + str(chID) + ' -> ' + channel + ' gamma:ewpE15', btoxgammacuts, chID, True)
        rankByLowest('B0:EWP_b2dgamma' + str(chID), 'abs(dM)', numBest=3)
        B02dgammaList.append('B0:EWP_b2dgamma' + str(chID))

    # B0 --> Xs0 gamma
    B02sgammaList = []
    for chID, channel in enumerate(Xs0Modes()):
        reconstructDecay('B0:EWP_b2sgamma' + str(chID) + ' -> ' + channel + ' gamma:ewpE15', btoxgammacuts, chID, True)
        rankByLowest('B0:EWP_b2sgamma' + str(chID), 'abs(dM)', numBest=3)
        B02sgammaList.append('B0:EWP_b2sgamma' + str(chID))

    # B+ --> Xd+ gamma
    Bplus2dgammaList = []
    for chID, channel in enumerate(XdplusModes()):
        reconstructDecay('B+:EWP_b2dgamma' + str(chID) + ' -> ' + channel + ' gamma:ewpE15', btoxgammacuts, chID, True)
        rankByLowest('B+:EWP_b2dgamma' + str(chID), 'abs(dM)', numBest=3)
        Bplus2dgammaList.append('B+:EWP_b2dgamma' + str(chID))

    # B+ --> Xs+ gamma
    Bplus2sgammaList = []
    for chID, channel in enumerate(XsplusModes()):
        reconstructDecay('B+:EWP_b2sgamma' + str(chID) + ' -> ' + channel + ' gamma:ewpE15', btoxgammacuts, chID, True)
        rankByLowest('B+:EWP_b2sgamma' + str(chID), 'abs(dM)', numBest=3)
        Bplus2sgammaList.append('B+:EWP_b2sgamma' + str(chID))

    return B02dgammaList + B02sgammaList + Bplus2dgammaList + Bplus2sgammaList
