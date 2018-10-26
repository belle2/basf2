#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
(Semi-)Leptonic Working Group Skims for missing energy modes that use the `FullEventInterpretation` (FEI) algorithm.
 The fei training is: MC9 based, release-02-00-00 'FEIv4_2018_MC9_release_02_00_00'
"""

__authors__ = [
    "Sophie Hollit",
    "Racha Cheaib",
    "Hannah Wakeling"
]

from basf2 import *
from modularAnalysis import *

from variables import *
variables.addAlias('sigProb', 'extraInfo(SignalProbability)')

from stdCharged import *


def B0hadronic():
    """
    Note:
        * FEI Hadronic B0 tag skim list
        * For generic analysis in the (Semi-)Leptonic and Missing Energy Working Group
        * Skim LFN code: 11180100
        * Physics channels: (All available FEI B0 Hadronic tags are reconstructed)

    **Decay Modes**:

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

    **Cuts applied are**:

        Event precuts:

        * R2EventLevel < 0.4
        * nTracks > 4

        Tag side B:

        * Mbc > 5.24
        * abs(deltaE) < 0.200
        * sigProb > 0.001
    """
    # Reconstruct tag side
    # Apply cuts
    applyCuts('B0:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')

    B0hadronicList = ['B0:generic']
    return B0hadronicList


def BplusHadronic():
    """
    Note:
        * FEI Hadronic B+ tag skim list
        * For generic analysis in the (Semi-)Leptonic and Missing Energy Working Group
        * Skim LFN code: 11180200
        * Physics channels: (All available FEI B+ Hadronic tags are reconstructed)

    **Decay Modes**:

        * B+ -> anti-D0 pi+
        * B+ -> anti-D0 pi+ pi0
        * B+ -> anti-D0 pi+ pi0 pi0
        * B+ -> anti-D0 pi+ pi+ pi-
        * B+ -> anti-D0 pi+ pi+ pi- pi0
        * B+ -> anti-D0 D+
        * B+ -> anti-D0 D+ KS0
        * B+ -> anti-D0* D+ KS0
        * B+ -> anti-D0 D+* KS0
        * B+ -> anti-D0* D+* KS0
        * B+ -> anti-D0 D0 K+
        * B+ -> anti-D0* D0 K+
        * B+ -> anti-D0 D*(2010)0 K+
        * B+ -> anti-D0* D*(2010)0 K+
        * B+ -> Ds+ anti-D0
        * B+ -> anti-D0* pi+
        * B+ -> anti-D0* pi+ pi0
        * B+ -> anti-D0* pi+ pi0 pi0
        * B+ -> anti-D0* pi+ pi+ pi-
        * B+ -> anti-D0* pi+ pi+ pi- pi0
        * B+ -> Ds+* anti-D0
        * B+ -> Ds+ anti-D0*
        * B+ -> anti-D0 K+
        * B+ -> D- pi+ pi+
        * B+ -> D- pi+ pi+ pi0
        * B+ -> J/Psi K+
        * B+ -> J/Psi K+ pi+ pi-
        * B+ -> J/Psi K+ pi0
        * B+ -> J/Psi KS0 pi+

    Skimming script reconstructs hadronic Btag using generically trained
    FEI.

    Skim Liasons: S. Hollitt & H. Wakeling

    **Cuts applied are**:

        Event precuts:

        * R2EventLevel < 0.4
        * nTracks > 4

        Tag side B:

        * Mbc > 5.24
        * abs(deltaE) < 0.200
        * sigProb > 0.001
    """
    # Reconstruct tag side
    # Apply cuts
    applyCuts('B+:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')

    BplushadronicList = ['B+:generic']
    return BplushadronicList


def B0SLWithOneLep():
    """
    Note:
        * FEI semi-leptonic (SL) B0 tag (Bsig with at least one lepton) skim list
        * For generic analysis in the (Semi-)Leptonic and Missing Energy Working Group.
        * Skim LFN code: 11180300
        * Physics channels: (All available FEI B0 SL tags are reconstructed)

    **Decay Modes**:

    * B0 -> D- e+ nu
    * B0 -> D- mu+ nu
    * B0 -> D+* e+ nu
    * B0 -> D+* mu+ nu
    * B0 -> anti-D0 pi- e+ nu
    * B0 -> anti-D0 pi- mu+ nu
    * B0 -> anti-D0* pi- e+ nu
    * B0 -> anti-D0* pi- mu+ nu

    In addition, all hadronic channels (listed in FEI hadronic skim are
    used, where one of the hadronic D mesons is replaced by a SL D meson.

    Skimming script reconstructs SL Btag using generically trained FEI
    and Bsig with at least one lepton (e, mu). Signal side lepton is not
    stored in skim output.

    Skim Liasons: S. Hollitt & H. Wakeling

    **Cuts applied are**:

        Event precuts:

        * R2EventLevel < 0.4
        * nTracks > 4

        Tag side B:

        * -5 < cosThetaBetweenParticleAndNominalB < 3
        * extraInfo(decayModeID) < 8 to remove semileptonic D channels
        * sigProb > 0.005 to give < 10% retention

        Signal side:

        * electron or muon from list 95eff
        * B Mbc > 0
"""
    # Reconstruct tag side
    # Apply cuts
    applyCuts('B0:semileptonic', '-5<cosThetaBetweenParticleAndNominalB<3 and sigProb>0.005 and extraInfo(decayModeID)<8')

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


def BplusSLWithOneLep():
    """
    Note:
        * FEI semi-leptonic B+ tag with one lepton skim list
        * For generic analysis in the (Semi-)Leptonic and Missing Energy Working Group
        * Skim LFN code: 11180400
        * Physics channels: (All available FEI Bplus SL tags are reconstructed)

    **Decay Modes**:

    * B+ -> anti-D0 e+ nu
    * B+ -> anti-D0 mu+ nu
    * B+ -> anti-D0* e+ nu
    * B+ -> anti-D0* mu+ nu
    * B+ -> D- pi+ e+ nu
    * B+ -> D- pi+ mu+ nu
    * B+ -> D+* pi+ e+ nu
    * B+ -> D+* pi+ mu+ nu

    In addition, all hadronic channels (listed in FEI
    Bplus hadronic skim are used, where one of the hadronic
    D mesons is replaced by a SL D meson.

    Skimming script reconstructs SL Btag using generically trained
    FEI and Bsig with at least one lepton (e, mu). Signal side lepton
    is not stored in skim output.

    Skim Liasons: S. Hollitt & H. Wakeling

    **Cuts applied are**:

        Event precuts:

        * R2EventLevel < 0.4
        * nTracks > 4

        Tag side B:

        * -5 < cosThetaBetweenParticleAndNominalB < 3
        * extraInfo(decayModeID) < 8 to remove semileptonic D channels
        * sigProb > 0.009 to give < 10% retention

        Signal side:
        * electron or muon from list 95eff
        * B Mbc > 0
    """

    # Reconstruct tag side
    # Apply cuts
    applyCuts('B+:semileptonic', '-5<cosThetaBetweenParticleAndNominalB<3 and sigProb>0.009 and extraInfo(decayModeID)<8')

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
