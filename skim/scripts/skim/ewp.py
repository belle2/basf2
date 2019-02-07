#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Skim list building functions for radiative and electroweak
:math:`b\\to (s,\\,d)\\gamma`, and :math:`b\\to (s,\\,d)\\ell\\ell` analyses
"""

__authors__ = [
    ""
]

from basf2 import *
from modularAnalysis import *

# take the loose stdPhotons SPL and require a bit of energy for eta candidates


def Xs0Modes():
    list = [
        # ordered as in BN1480
        'K+:ewp pi-:ewp',  # 1
        'K_S0:ewp pi0:ewpHigh',  # 4
        'K_S0:ewp pi+:ewpHigh pi-:ewp2High',  # 6
        'K+:ewp pi-:ewp pi0:ewpHigh',  # 7
        'K+:ewp pi-:ewpHigh pi+:ewp2High pi-:ewp',  # 9
        'K_S0:ewp pi+:ewpHigh pi-:ewp2High pi0:ewpHigh',  # 12
        'K_S0:ewp pi+:ewpHigh pi-:ewp2High pi+:ewp pi-:ewp',  # 14
        'K+:ewp pi+:ewpHigh pi-:ewp2High pi+:ewp pi0:ewpHigh',  # 15
        'K_S0:ewp pi0:ewpHigh pi0:ewpHigh',  # 18
        'K+:ewp pi-:ewp pi0:ewpHigh pi0:ewpHigh',  # 19
        'K_S0:ewp pi+:ewpHigh pi-:ewp2High pi0:ewpHigh pi0:ewpHigh',  # 22
        # 'K_S0:ewp eta:ewp',  # 24
        # 'K+:ewp eta:ewp pi-:ewp',  # 25
        # 'K_S0:ewp eta:ewp pi0:ewpHigh',  # 28
        # 'K_S0:ewp eta:ewp pi+:ewp pi-:ewp',  # 30
        # 'K+:ewp eta:ewp pi-:ewp pi0:ewpHigh',  # 31
        'K+:ewp K-:ewp K_S0:ewp',  # 34
        'K+:ewp K+:ewp K-:ewp pi-:ewp',  # 35
        'K+:ewp K-:ewp K_S0:ewp pi0:ewpHigh',  # 38
    ]
    return list


def XsplusModes():
    list = [
        'K_S0:ewp pi+:ewp',  # 2
        'K+:ewp pi0:ewpHigh',  # 3
        'K+:ewp pi+:ewpHigh pi-:ewp2High',  # 5
        'K_S0:ewp pi+:ewp pi0:ewpHigh',  # 8
        'K_S0:ewp pi+:ewpHigh pi+:ewp2High pi-:ewp',  # 10
        'K+:ewp pi+:ewpHigh pi-:ewp2High pi0:ewpHigh',  # 11
        'K+:ewp pi+:ewpHigh pi-:ewp2High pi+:ewp pi-:ewp',  # 13
        'K_S0:ewp pi+:ewpHigh pi+:ewp2High pi-:ewp pi0:ewpHigh',  # 16
        'K+:ewp pi0:ewpHigh pi0:ewpHigh',  # 17
        'K_S0:ewp pi+:ewp pi0:ewpHigh pi0:ewpHigh',  # 20
        'K+:ewp pi+:ewpHigh pi-:ewp2High pi0:ewpHigh pi0:ewpHigh',  # 21
        # 'K+:ewp eta:ewp',  # 23
        # 'K_S0:ewp eta:ewp pi+:ewp',  # 26
        # 'K+:ewp eta:ewp pi0:ewpHigh',  # 27
        # 'K+:ewp eta:ewp pi+:ewp pi-:ewp',  # 29
        # 'K_S0:ewp eta:ewp pi+:ewp pi0:ewp ',  # 32
        'K+:ewp K+:ewp K-:ewp',  # 33
        'K+:ewp K-:ewp K_S0:ewp pi+:ewp',  # 36
        'K+:ewp K+:ewp K-:ewp pi0:ewpHigh',  # 37
    ]
    return list


def Xd0Modes():
    list = [
        'pi+:ewpHigh pi-:ewp2High',
        'pi+:ewpHigh pi-:ewp2High pi0:ewpHigh',
        'pi+:ewpHigh pi-:ewp2High pi0:ewpHigh pi0:ewpHigh',
    ]
    return list


def XdplusModes():
    list = [
        'pi+:ewp pi0:ewpHigh',
        'pi+:ewpHigh pi+:ewp2High pi-:ewp pi0:ewpHigh',
        # 'pi+:ewp eta:ewp',
        'pi+:ewpHigh pi+:ewp2High pi-:ewp',
    ]
    return list


def B2XgammaList(path):
    """Build the skim list for B --> X(s,d) gamma decays"""

    # event level cuts: R2 and require a minimum number of tracks + decent photons
    applyEventCuts(
        'R2EventLevel < 0.7 and ' +
        'formula(nTracks + nParticlesInList(gamma:loose) / 2) > 4', path=path)
    #
    # cuts in addition to the standard particle lists
    # should be revised for each new SPL release
    cutAndCopyList('K+:ewp', 'K+:95eff', 'abs(d0) < 1.0 and abs(z0) < 4.0', path=path)
    cutAndCopyList('pi+:ewp', 'pi+:95eff', 'abs(d0) < 1.0 and abs(z0) < 4.0', path=path)
    cutAndCopyList('pi+:ewpHigh', 'pi+:95eff', 'p > 0.25 and abs(d0) < 1.0 and abs(z0) < 4.0', path=path)
    cutAndCopyList('pi+:ewp2High', 'pi+:95eff', 'p > 0.10 and abs(d0) < 1.0 and abs(z0) < 4.0', path=path)
    #
    cutAndCopyList('pi0:ewp', 'pi0:skim', 'p > 0.25 and 0.120 < M < 0.145', path=path)
    cutAndCopyList('pi0:ewpHigh', 'pi0:skim', 'p > 0.50 and 0.120 < M < 0.145', path=path)
    cutAndCopyList('K_S0:ewp', 'K_S0:all', 'p > 0.50 and 0.4776 < M < 0.5176', path=path)  # 20 MeV width
    #
    #
    # take the tight stdPhotons SPL (timing cuts dependent on regions) and add
    # a minimum lab-frame energy requirement (1.5 GeV) and cluster shape e9oe21
    cutAndCopyList('gamma:ewpE15', 'gamma:tight', 'clusterE9E21 > 0.9 and 1.5 < E < 100', path=path)
    #
    # invariant mass and dE windows for all modes
    btoxgammacuts = '5.2 < Mbc < 5.29 and -0.5 < deltaE < 0.3'

    # B0 --> Xd0 gamma
    B02dgammaList = []
    for chID, channel in enumerate(Xd0Modes()):
        reconstructDecay('B0:EWP_b2dgamma' + str(chID) + ' -> ' + channel + ' gamma:ewpE15', btoxgammacuts, chID, True, path=path)
        rankByLowest('B0:EWP_b2dgamma' + str(chID), 'abs(dM)', numBest=3, path=path)
        B02dgammaList.append('B0:EWP_b2dgamma' + str(chID))

    # B0 --> Xs0 gamma
    B02sgammaList = []
    for chID, channel in enumerate(Xs0Modes()):
        reconstructDecay('B0:EWP_b2sgamma' + str(chID) + ' -> ' + channel + ' gamma:ewpE15', btoxgammacuts, chID, True, path=path)
        rankByLowest('B0:EWP_b2sgamma' + str(chID), 'abs(dM)', numBest=3, path=path)
        B02sgammaList.append('B0:EWP_b2sgamma' + str(chID))

    # B+ --> Xd+ gamma
    Bplus2dgammaList = []
    for chID, channel in enumerate(XdplusModes()):
        reconstructDecay('B+:EWP_b2dgamma' + str(chID) + ' -> ' + channel + ' gamma:ewpE15', btoxgammacuts, chID, True, path=path)
        rankByLowest('B+:EWP_b2dgamma' + str(chID), 'abs(dM)', numBest=3, path=path)
        Bplus2dgammaList.append('B+:EWP_b2dgamma' + str(chID))

    # B+ --> Xs+ gamma
    Bplus2sgammaList = []
    for chID, channel in enumerate(XsplusModes()):
        reconstructDecay('B+:EWP_b2sgamma' + str(chID) + ' -> ' + channel + ' gamma:ewpE15', btoxgammacuts, chID, True, path=path)
        rankByLowest('B+:EWP_b2sgamma' + str(chID), 'abs(dM)', numBest=3, path=path)
        Bplus2sgammaList.append('B+:EWP_b2sgamma' + str(chID))

    return B02dgammaList + B02sgammaList + Bplus2dgammaList + Bplus2sgammaList


def B2XllList(path):
    """Build the skim list for B --> X(s,d) l+ l- decays"""

    # event level cuts: R2 and require a minimum number of tracks
    applyEventCuts('R2EventLevel < 0.7 and nTracks > 4', path=path)

    # cuts in addition to the standard particle lists
    # should be revised for each new SPL release
    cutAndCopyList('mu+:ewpHigh', 'mu+:95eff', 'p > 0.70', path=path)
    cutAndCopyList('e+:ewpHigh', 'e+:95eff', 'p > 0.40', path=path)
    #
    cutAndCopyList('K+:ewp', 'K+:95eff', 'abs(d0) < 0.2 and abs(z0) < 0.2', path=path)
    cutAndCopyList('pi+:ewp', 'pi+:95eff', 'abs(d0) < 0.2 and abs(z0) < 0.2', path=path)
    cutAndCopyList('pi+:ewpHigh', 'pi+:95eff', 'p > 0.40', path=path)
    cutAndCopyList('pi+:ewp2High', 'pi+:95eff', 'p > 0.25', path=path)
    #
    cutAndCopyList('pi0:ewp', 'pi0:skim', 'p > 0.20 and 0.115 < M < 0.145', path=path)
    cutAndCopyList('pi0:ewpHigh', 'pi0:skim', 'p > 0.40 and 0.115 < M < 0.145', path=path)
    cutAndCopyList('K_S0:ewp', 'K_S0:all', '0.4776 < M < 0.5176', path=path)  # 20 MeV width

    # invariant mass and dE windows for all modes
    btoxlldilepton = 'formula(daughter(0, E)+daughter(1, E)) > 1.5'  # dilepton energy sum in a dirty way
    btoxllcuts = '5.2 < Mbc < 5.29 and -0.5 < deltaE < 0.3 and ' + btoxlldilepton

    # B0 --> Xd0 l+ l-
    B02dllList = []
    for chID, channel in enumerate(Xd0Modes() + [' pi0:ewp ', ' eta:ewp ']):
        reconstructDecay('B0:EWP_b2dee' + str(chID) + ' ->  e-:ewpHigh  e+:ewpHigh  ' + channel, btoxllcuts, chID, True, path=path)
        reconstructDecay(
            'B0:EWP_b2dmumu' +
            str(chID) +
            ' ->  mu+:ewpHigh mu-:ewpHigh ' +
            channel,
            btoxllcuts,
            chID,
            True,
            path=path)
        rankByLowest('B0:EWP_b2dee' + str(chID), 'abs(dM)', numBest=3, path=path)
        rankByLowest('B0:EWP_b2dmumu' + str(chID), 'abs(dM)', numBest=3, path=path)
        B02dllList.append('B0:EWP_b2dee' + str(chID))
        B02dllList.append('B0:EWP_b2dmumu' + str(chID))

    # B0 --> Xs0 l+ l-
    B02sllList = []
    for chID, channel in enumerate(Xs0Modes() + [' K_S0:ewp ']):
        reconstructDecay('B0:EWP_b2see' + str(chID) + ' -> e-:ewpHigh  e+:ewpHigh  ' + channel, btoxllcuts, chID, True, path=path)
        reconstructDecay('B0:EWP_b2smumu' + str(chID) + ' -> mu+:ewpHigh mu-:ewpHigh ' + channel, btoxllcuts, chID, True, path=path)
        rankByLowest('B0:EWP_b2see' + str(chID), 'abs(dM)', numBest=3, path=path)
        rankByLowest('B0:EWP_b2smumu' + str(chID), 'abs(dM)', numBest=3, path=path)
        B02sllList.append('B0:EWP_b2see' + str(chID))
        B02sllList.append('B0:EWP_b2smumu' + str(chID))
    # B+ --> Xd+ l+ l-
    Bplus2dllList = []
    for chID, channel in enumerate(XdplusModes() + ['pi+:95eff ']):
        reconstructDecay('B-:EWP_b2dee' + str(chID) + ' -> e-:ewpHigh e+:ewpHigh   ' + channel, btoxllcuts, chID, True, path=path)
        reconstructDecay('B-:EWP_b2dmumu' + str(chID) + ' -> mu+:ewpHigh mu-:ewpHigh ' + channel, btoxllcuts, chID, True, path=path)
        rankByLowest('B-:EWP_b2dee' + str(chID), 'abs(dM)', numBest=3, path=path)
        rankByLowest('B-:EWP_b2dmumu' + str(chID), 'abs(dM)', numBest=3, path=path)
        Bplus2dllList.append('B-:EWP_b2dee' + str(chID))
        Bplus2dllList.append('B-:EWP_b2dmumu' + str(chID))

    # B+ --> Xs+ l+ l-
    Bplus2sllList = []
    for chID, channel in enumerate(XsplusModes() + [' K+:95eff ']):
        reconstructDecay('B-:EWP_b2see' + str(chID) + ' -> e-:ewpHigh  e+:ewpHigh  ' + channel, btoxllcuts, chID, True, path=path)
        reconstructDecay('B-:EWP_b2smumu' + str(chID) + ' -> mu+:ewpHigh mu-:ewpHigh ' + channel, btoxllcuts, chID, True, path=path)
        rankByLowest('B-:EWP_b2see' + str(chID), 'abs(dM)', numBest=3, path=path)
        rankByLowest('B-:EWP_b2smumu' + str(chID), 'abs(dM)', numBest=3, path=path)
        Bplus2sllList.append('B-:EWP_b2see' + str(chID))
        Bplus2sllList.append('B-:EWP_b2smumu' + str(chID))

    return B02dllList + B02sllList + Bplus2dllList + Bplus2sllList
