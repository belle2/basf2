#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
    FEI Hadronic B0 tag skim list for generic analysis in WG1:
    (Semi-)Leptonic and Missing Energy
    Skim LFN code: 11180100
    Physics channels: (All available FEI B0 Hadronic tags are
    reconstructed)
    * B0 -> D- pi+
    * B0 -> D- pi+ pi0
    * B0 -> D- pi+ pi0 pi0
    * B0 -> D- pi+ pi+ pi-
    * B0 -> D- pi+ pi+ pi- pi0
    * B0 -> anti-D0 pi+ pi0
    * B0 -> D- D0 K+
    * B0 -> D- D*(2010)0 K+
    * B0 -> D+* D0 K+
    * B0 -> D+* D*(2010)0 K+
    * B0 -> D- D+ KS0
    * B0 -> D+* D+ KS0
    * B0 -> D- D+* KS0
    * B0 -> D+* D+* KS0
    * B0 -> Ds+ D-
    * B0 -> D+* pi+
    * B0 -> D+* pi+ pi0
    * B0 -> D+* pi+ pi0 pi0
    * B0 -> D+* pi+ pi+ pi-
    * B0 -> D+* pi+ pi+ pi- pi0
    * B0 -> Ds+* D-
    * B0 -> Ds+ D+*
    * B0 -> Ds+* D+*
    * B0 -> J/Psi KS0
    * B0 -> J/Psi K+ pi-
    * B0 -> J/Psi KS0 pi+ pi-

    Skimming script reconstructs hadronic Btag using generically trained
    FEI. From Thomas Keck's thesis, 'the channel B0 -> anti-D0 pi0 was
    used by the FR, but is not yet used in the FEI due to unexpected
    technical restrictions in the KFitter algorithm'.

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


def B0hadronic():
    """FEI Hadronic B0 skim list"""
    # Reconstruct tag side
    # Apply cuts
    applyCuts('B0:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')

    B0hadronicList = ['B0:generic']
    return B0hadronicList
