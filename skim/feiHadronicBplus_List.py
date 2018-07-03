#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
    FEI Hadronic B+ tag skim list for generic analysis in WG1:
    (Semi-)Leptonic and Missing Energy
    Skim LFN code: 11180200
    Physics channels: (All available FEI B+ Hadronic tags are
    reconstructed)
    *B+ -> anti-D0 pi+
    *B+ -> anti-D0 pi+ pi0
    *B+ -> anti-D0 pi+ pi0 pi0
    *B+ -> anti-D0 pi+ pi+ pi-
    *B+ -> anti-D0 pi+ pi+ pi- pi0
    *B+ -> anti-D0 D+
    *B+ -> anti-D0 D+ KS0
    *B+ -> anti-D0* D+ KS0
    *B+ -> anti-D0 D+* KS0
    *B+ -> anti-D0* D+* KS0
    *B+ -> anti-D0 D0 K+
    *B+ -> anti-D0* D0 K+
    *B+ -> anti-D0 D*(2010)0 K+
    *B+ -> anti-D0* D*(2010)0 K+
    *B+ -> Ds+ anti-D0
    *B+ -> anti-D0* pi+
    *B+ -> anti-D0* pi+ pi0
    *B+ -> anti-D0* pi+ pi0 pi0
    *B+ -> anti-D0* pi+ pi+ pi-
    *B+ -> anti-D0* pi+ pi+ pi- pi0
    *B+ -> Ds+* anti-D0
    *B+ -> Ds+ anti-D0*
    *B+ -> anti-D0 K+
    *B+ -> D- pi+ pi+
    *B+ -> D- pi+ pi+ pi0
    *B+ -> J/Psi K+
    *B+ -> J/Psi K+ pi+ pi-
    *B+ -> J/Psi K+ pi0
    *B+ -> J/Psi KS0 pi+

    Skimming script reconstructs hadronic Btag using generically trained
    FEI.

    Skim Liasons: S. Hollitt & H. Wakeling

    Cuts applied are::
        Event precuts:
        R2EventLevel < 0.4
        nTracks > 4 

        Tag side B:
        Mbc > 5.24
        abs(deltaE) < 0.200
        sigProb > 0.001
    """

__author__ = "R. Cheaib & S. Hollitt"

from basf2 import *
from modularAnalysis import *

from variables import *
variables.addAlias('sigProb', 'extraInfo(SignalProbability)')


def BplusHadronic():
    """FEI Hadronic B+ skim list"""
    # Reconstruct tag side
    # Apply cuts
    applyCuts('B+:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')

    BplushadronicList = ['B+:generic']
    return BplushadronicList
