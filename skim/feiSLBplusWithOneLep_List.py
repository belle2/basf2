#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
    FEI semi-leptonic (SL) Bplus tag with one lepton skim
    list for generic analysis in WG1: (Semi-)Leptonic and
    Missing Energy

    Skim LFN code: 11180600
    Physics channels: (All available FEI Bplus SL tags are reconstructed)

    *B+ -> anti-D0 e+ nu
    *B+ -> anti-D0 mu+ nu
    *B+ -> anti-D0* e+ nu
    *B+ -> anti-D0* mu+ nu
    *B+ -> D- pi+ e+ nu
    *B+ -> D- pi+ mu+ nu
    *B+ -> D+* pi+ e+ nu
    *B+ -> D+* pi+ mu+ nu
    In addition, all hadronic channels (listed in FEI
    Bplus hadronic skim are used, where one of the hadronic
    D mesons is replaced by a SL D meson.

    Skimming script reconstructs SL Btag using generically trained
    FEI and Bsig with at least one lepton (e, mu). Signal side lepton
    is not stored in skim output.

    Skim Liasons: S. Hollitt & H. Wakeling

    Cuts applied are::
    Event precuts:
    R2EventLevel < 0.4
    nTracks > 4

    Tag side B:
    -5 < cosThetaBetweenParticleAndTrueB) < 3
    extraInfo(decayModeID) < 8 to remove semileptonic D channels
    sigProb > 0.009 to give < 10% retention

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


def BplusSLWithOneLep():
    # Reconstruct tag side
    # Apply cuts
    applyCuts('B+:semileptonic', '-5<cosThetaBetweenParticleAndTrueB<3 and sigProb>0.009 and extraInfo(decayModeID)<8')

    # Reconstruct signal side to lepton
    stdE('95eff')
    stdMu('95eff')
    reconstructDecay('B+:sig1 -> e+:95eff', 'Mbc>0', 1)
    reconstructDecay('B+:sig2 -> mu+:95eff', 'Mbc>0', 2)
    reconstructDecay('B+:sig3 -> e-:95eff', 'Mbc>0', 3)
    reconstructDecay('B+:sig4 -> mu-:95eff', 'Mbc>0', 4)

    copyLists('B+:all', ['B+:sig1', 'B+:sig2', 'B+:sig3', 'B+:sig4'])

    reconstructDecay('Upsilon(4S):sig -> B-:semileptonic B+:all', '')
    # Apply cuts
    applyCuts('B+:semileptonic', 'nParticlesInList(Upsilon(4S):sig)>0')

    BtagList = ['B+:semileptonic']
    return BtagList
