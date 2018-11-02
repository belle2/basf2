#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
(Semi-)Leptonic Working Group Skims for missing energy modes that use the `FullEventInterpretation` (FEI) algorithm.
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


def B0hadronic(path):
    """FEI Hadronic B0 tag skim list for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180100
    Physics channels: (All available FEI B0 Hadronic tags are
    used)
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

    This function applies cuts to the FEI-reconstructed tag side B, and
    the pre-cuts and FEI must be applied separately.

    From Thomas Keck's thesis, 'the channel B0 -> anti-D0 pi0 was
    used by the FR, but is not yet used in the FEI due to unexpected
    technical restrictions in the KFitter algorithm'.

    Skim Liasons: S. Hollitt & H. Wakeling

    Cuts applied are::
        Event precuts:
        R2EventLevel < 0.4
        nTracks >= 4
=======
        Tag side B:
        Mbc > 5.24
        abs(deltaE) < 0.200
        sigProb > 0.001
"""
    # B0:generic list from FEI must already exist in path
    # Apply cuts
    applyCuts('B0:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001', path=path)

    B0hadronicList = ['B0:generic']
    return B0hadronicList


def BplusHadronic(path):
    """FEI Hadronic B+ tag skim list for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
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

    This function applies cuts to the FEI-reconstructed tag side B, and
    the pre-cuts and FEI must be applied separately.

    Skim Liasons: S. Hollitt & H. Wakeling

    Cuts applied are::
        Event precuts:
        R2EventLevel < 0.4
        nTracks >= 4
=======
        Tag side B:
        Mbc > 5.24
        abs(deltaE) < 0.200
        sigProb > 0.001"""

    # B+:generic list from FEI must already exist in path
    # Apply cuts
    applyCuts('B+:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001', path=path)

    BplushadronicList = ['B+:generic']
    return BplushadronicList


def runFEIforB0Hadronic(path):
    """Generates FEI B0:generic list, including applying FEI weights and skim
    pre-selection cuts. Use B0hadronic(path) for skim cuts on FEI output list.

    (Semi-)Leptonic and Missing Energy Working Group
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

    FEI weightfiles: FEIv4_2018_MC9_release_02_00_01

    From Thomas Keck's thesis, 'the channel B0 -> anti-D0 pi0 was
    used by the FR, but is not yet used in the FEI due to unexpected
    technical restrictions in the KFitter algorithm'.

    Skim Liasons: S. Hollitt & H. Wakeling

    Cuts applied are::
        Event precuts:
        R2EventLevel < 0.4
        nTracks >= 4
=======
        Tag side B:
        Mbc > 5.24
        abs(deltaE) < 0.200
        sigProb > 0.001
"""
    # Pre-selection cuts
    applyEventCuts('R2EventLevel<0.4 and nTracks>=4', path=path)

    # Run FEI
    from fei import backward_compatibility_layer
    backward_compatibility_layer.pid_renaming_oktober_2017()
    use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')

    import fei
    particles = fei.get_default_channels(neutralB=True, chargedB=False, hadronic=True, semileptonic=False, KLong=False)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_release_02_00_01', training=False, monitor=False)
    feistate = fei.get_path(particles, configuration)
    path.add_path(feistate.path)


def runFEIforBplusHadronic(path):
    """Generates FEI B+:generic list, including applying FEI weights and skim
    pre-selection cuts. Use BplusHadronic(path) for skim cuts on FEI output list

    (Semi-)Leptonic and Missing Energy Working Group
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

    FEI weightfiles: FEIv4_2018_MC9_release_02_00_01

    Skim Liasons: S. Hollitt & H. Wakeling

    Cuts applied are::
        Event precuts:
        R2EventLevel < 0.4
        nTracks >= 4
=======
        Tag side B:
        Mbc > 5.24
        abs(deltaE) < 0.200
        sigProb > 0.001"""

    # Pre-selection cuts
    applyEventCuts('R2EventLevel<0.4 and nTracks>=4', path=path)

    # Run FEI
    from fei import backward_compatibility_layer
    backward_compatibility_layer.pid_renaming_oktober_2017()
    use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')

    import fei
    particles = fei.get_default_channels(neutralB=False, chargedB=True, hadronic=True, semileptonic=False, KLong=False)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_release_02_00_01', training=False, monitor=False)
    feistate = fei.get_path(particles, configuration)
    path.add_path(feistate.path)


def runFEIforHadronicCombined(path):
    """Generates FEI B+:generic and B0:generic list for FEI Hadronic skims, including
    applying FEI weights and skim pre-selection cuts.
    Use BplusHadronic(path) and B0Hadronic(path) for skim cuts on FEI output lists

    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180100, 11180200
    Physics channels: (All available FEI B0 and B+ Hadronic tags are
    reconstructed)
    FEI weightfiles: FEIv4_2018_MC9_release_02_00_01

    Skim Liasons: S. Hollitt & H. Wakeling

    Cuts applied are::
        Event precuts:
        R2EventLevel < 0.4
        nTracks >= 4"""

    # Pre-selection cuts
    applyEventCuts('R2EventLevel<0.4 and nTracks>=4', path=path)

    # Run FEI
    from fei import backward_compatibility_layer
    backward_compatibility_layer.pid_renaming_oktober_2017()
    use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')

    import fei
    particles = fei.get_default_channels(neutralB=True, chargedB=True, hadronic=True, semileptonic=False, KLong=False)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_release_02_00_01', training=False, monitor=False)
    feistate = fei.get_path(particles, configuration)
    path.add_path(feistate.path)


def B0SLWithOneLep(path):
    """FEI Semi-Leptonic B0 tag (with Bsig with at least one lepton) skim list
    FEI semi-leptonic (SL) B0 tag with one lepton
    skim list for generic analysis in the (Semi-)Leptonic and
    Missing Energy Working Group
    Skim LFN code: 11180300
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
        nTracks >= 4

        Tag side B:
        -5 < cosThetaBetweenParticleAndNominalB < 3
        extraInfo(decayModeID) < 8 to remove semileptonic D channels
        sigProb > 0.005 to give < 10% retention

        Signal side:
        electron or muon from list 95eff
        B Mbc > 0
"""
    # Reconstruct tag side
    # Apply cuts
    applyCuts('B0:semileptonic', '-5<cosThetaBetweenParticleAndNominalB<3 and sigProb>0.005 and extraInfo(decayModeID)<8',
              path=path)

    # Reconstruct signal side to lepton
    stdE('95eff', path=path)
    stdMu('95eff', path=path)
    reconstructDecay('B0:sig1 -> e+:95eff', 'Mbc>0', 1, path=path)
    reconstructDecay('B0:sig2 -> mu+:95eff', 'Mbc>0', 2, path=path)
    reconstructDecay('B0:sig3 -> e-:95eff', 'Mbc>0', 3, path=path)
    reconstructDecay('B0:sig4 -> mu-:95eff', 'Mbc>0', 4, path=path)

    copyLists('B0:all', ['B0:sig1', 'B0:sig2', 'B0:sig3', 'B0:sig4'], path=path)

    reconstructDecay('Upsilon(4S):sig -> anti-B0:semileptonic B0:all', '', path=path)
    # Apply cuts
    applyCuts('B0:semileptonic', 'nParticlesInList(Upsilon(4S):sig)>0', path=path)

    BtagList = ['B0:semileptonic']
    return BtagList


def BplusSLWithOneLep(path):
    """ FEI semi-leptonic (SL) Bplus tag with one lepton skim
    list for generic analysis in the (Semi-)Leptonic and
    Missing Energy Working Group

    Skim LFN code: 11180400
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
    nTracks >= 4

    Tag side B:
    -5 < cosThetaBetweenParticleAndNominalB < 3
    extraInfo(decayModeID) < 8 to remove semileptonic D channels
    sigProb > 0.009 to give < 10% retention

    Signal side:
    electron or muon from list 95eff
    B Mbc > 0
    """

    # Reconstruct tag side
    # Apply cuts
    applyCuts('B+:semileptonic', '-5<cosThetaBetweenParticleAndNominalB<3 and sigProb>0.009 and extraInfo(decayModeID)<8',
              path=path)

    # Reconstruct signal side to lepton
    stdE('95eff', path=path)
    stdMu('95eff', path=path)
    reconstructDecay('B+:sig1 -> e+:95eff', 'Mbc>0', 1, path=path)
    reconstructDecay('B+:sig2 -> mu+:95eff', 'Mbc>0', 2, path=path)
    reconstructDecay('B+:sig3 -> e-:95eff', 'Mbc>0', 3, path=path)
    reconstructDecay('B+:sig4 -> mu-:95eff', 'Mbc>0', 4, path=path)

    copyLists('B+:all', ['B+:sig1', 'B+:sig2', 'B+:sig3', 'B+:sig4'], path=path)

    reconstructDecay('Upsilon(4S):sig -> B-:semileptonic B+:all', '', path=path)
    # Apply cuts
    applyCuts('B+:semileptonic', 'nParticlesInList(Upsilon(4S):sig)>0', path=path)

    BtagList = ['B+:semileptonic']
    return BtagList
