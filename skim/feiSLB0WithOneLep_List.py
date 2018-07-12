#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
    FEI semi-leptonic (SL) B0 tag with one lepton
    skim list for generic analysis in WG1: (Semi-)Leptonic and
    Missing Energy
    Skim LFN code: 11180500
    Physics channels: (All available FEI B0 SL tags are reconstructed)

    *B0 -> D- e+ nu
    *B0 -> D- mu+ nu
    *B0 -> D+* e+ nu
    *B0 -> D+* mu+ nu
    *B0 -> anti-D0 pi- e+ nu
    *B0 -> anti-D0 pi- mu+ nu
    *B0 -> anti-D0* pi- e+ nu
    *B0 -> anti-D0* pi- mu+ nu
    In addition, all hadronic channels (listed in FEI hadronic skim are
    used, where one of the hadronic D mesons is replaced by a SL D meson.

    Skimming script reconstructs SL Btag using generically trained FEI
    and Bsig with at least one lepton (e, mu). Signal side lepton is not
    stored in skim output.

    Skim Liasons: S. Hollitt & H. Wakeling

    Cuts applied are::
        Event precuts:
        R2EventLevel < 0.4
        nTracks > 4

        Tag side B:
        -5 < cosThetaBetweenParticleAndTrueB) < 3
        extraInfo(decayModeID) < 8 to remove semileptonic D channels
        sigProb > 0.005 to give < 10% retention

        Signal side:
        electron or muon from list 95eff
        B Mbc > 0
    """

__author__ = "R. Cheaib & S. Hollitt"

from basf2 import *
from modularAnalysis import *

from variables import *
variables.addAlias('sigProb', 'extraInfo(SignalProbability)')

from stdCharged import *


def B0SLWithOneLep():
    """FEI Semi-Leptonic B0 tag (with Bsig with at least one lepton) skim
        list"""
    # Reconstruct tag side
    # Apply cuts
    applyCuts('B0:semileptonic', '-5<cosThetaBetweenParticleAndTrueB<3 and sigProb>0.005 and extraInfo(decayModeID)<8')

    # Reconstruct signal side to lepton
    stdE('95eff')
    stdMu('95eff')
    reconstructDecay('B0:sig1 -> e+:95eff', 'Mbc>0', 1)
    reconstructDecay('B0:sig2 -> mu+:95eff', 'Mbc>0', 2)
    reconstructDecay('B0:sig3 -> e-:95eff', 'Mbc>0', 3)
    reconstructDecay('B0:sig4 -> mu-:95eff', 'Mbc>0', 4)

    copyLists('B0:all', ['B0:sig1', 'B0:sig2', 'B0:sig3', 'B0:sig4'])

    reconstructDecay('Upsilon(4S):sig -> anti-B0:semileptonic B0:all', '')
    # Apply cuts
    applyCuts('B0:semileptonic', 'nParticlesInList(Upsilon(4S):sig)>0')

    BtagList = ['B0:semileptonic']
    return BtagList
