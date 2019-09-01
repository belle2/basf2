#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
(Semi-)Leptonic Working Group Skims for missing energy modes that use the `FullEventInterpretation` (FEI) algorithm.
"""

__authors__ = [
    "Sophie Hollit",
    "Racha Cheaib",
    "Hannah Wakeling",
    "Phil Grace"
]

import basf2
import fei
from modularAnalysis import *

from variables import *
variables.addAlias('sigProb', 'extraInfo(SignalProbability)')
variables.addAlias('log10_sigProb', 'log10(extraInfo(SignalProbability))')
variables.addAlias('dmID', 'extraInfo(decayModeID)')
variables.addAlias('cosThetaBY', 'cosThetaBetweenParticleAndNominalB')
variables.addAlias('d1_p_CMSframe', 'useCMSFrame(daughter(1,p))')
variables.addAlias('d2_p_CMSframe', 'useCMSFrame(daughter(2,p))')

from stdCharged import *


def B0hadronic(path):
    """FEI Hadronic B0 tag skim list for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group.

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

    Skim Liaisons: S. Hollitt, H. Wakeling, & P. Grace

    **Cuts applied are**:

        Event precuts:

        * foxWolframR2 < 0.4
        * nTracks >= 4

        Tag side B:

        * Mbc > 5.24
        * abs(deltaE) < 0.200
        * sigProb > 0.001

    Parameters:
        path (basf2.Path) the path to add the skim list builders

    Returns:
        list name of the skim candidates

    """

    applyCuts('B0:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001', path=path)

    B0hadronicList = ['B0:generic']
    return B0hadronicList


def BplusHadronic(path):
    """FEI Hadronic B+ tag skim list for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group.

    Skim LFN code: 11180200

    Physics channels: (All available FEI B+ Hadronic tags are
    reconstructed)

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

    This function applies cuts to the FEI-reconstructed tag side B, and
    the pre-cuts and FEI must be applied separately.

    Skim Liaisons: S. Hollitt, H. Wakeling, & P. Grace

    **Cuts applied are**:

        Event precuts:

        * foxWolframR2 < 0.4
        * nTracks >= 4

        Tag side B:

        * Mbc > 5.24
        * abs(deltaE) < 0.200
        * sigProb > 0.001

    Parameters:
        path (basf2.Path) the path to add the skim list builders

    Returns:
        list name of the skim candidates
    """

    # B+:generic list from FEI must already exist in path
    # Apply cuts
    applyCuts('B+:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001', path=path)

    BplushadronicList = ['B+:generic']
    return BplushadronicList


def runFEIforB0Hadronic(path):
    """Generates FEI B0:generic list, including applying FEI weights and skim
    pre-selection cuts. Use B0hadronic(path) for skim cuts on FEI output list.

    (Semi-)Leptonic and Missing Energy Working Group.

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

    FEI weightfiles: FEIv4_2019_MC12_release_03_01_01

    From Thomas Keck's thesis, 'the channel B0 -> anti-D0 pi0 was
    used by the FR, but is not yet used in the FEI due to unexpected
    technical restrictions in the KFitter algorithm'.

    Skim Liaisons: S. Hollitt, H. Wakeling, & P. Grace

   ** Cuts applied are**:

        Event precuts:

        * foxWolframR2 < 0.4
        * nTracks >= 4

        Tag side B:

        * Mbc > 5.24
        * abs(deltaE) < 0.200
        * sigProb > 0.001

    Parameters:
        path (basf2.Path) the path to add the skim list builders

    Returns:
        list name of the skim candidates
    """
    # Pre-selection cuts
    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt> 0.1', path=path)
    fillParticleList(decayString='gamma:eventShapeForSkims',
                     cut='E > 0.1', path=path)

    buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
                    allMoments=False,
                    foxWolfram=True,
                    harmonicMoments=False,
                    cleoCones=False,
                    thrust=False,
                    collisionAxis=False,
                    jets=False,
                    sphericity=False,
                    checkForDuplicates=False,
                    path=path)

    applyEventCuts('foxWolframR2<0.4 and nTracks>=4', path=path)

    # Run FEI
    basf2.use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')

    particles = fei.get_default_channels(neutralB=True, chargedB=False, hadronic=True, semileptonic=False, KLong=False)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2019_MC12_release_03_01_01', training=False, monitor=False)
    feistate = fei.get_path(particles, configuration)
    path.add_path(feistate.path)


def runFEIforBplusHadronic(path):
    """Generates FEI B+:generic list, including applying FEI weights and skim
    pre-selection cuts. Use BplusHadronic(path) for skim cuts on FEI output list.

    (Semi-)Leptonic and Missing Energy Working Group.

    Skim LFN code: 11180200

    Physics channels: (All available FEI B+ Hadronic tags are
    reconstructed)

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

    FEI weightfiles: FEIv4_2019_MC12_release_03_01_01

    Skim Liaisons: S. Hollitt, H. Wakeling, & P. Grace

    **Cuts applied are**:

        Event precuts:

        * foxWolframR2 < 0.4
        * nTracks >= 4

        Tag side B:

        * Mbc > 5.24
        * abs(deltaE) < 0.200
        * sigProb > 0.001

    Parameters:
        path (basf2.Path) the path to add the skim list builders

    Returns:
        list name of the skim candidates
    """
    # Pre-selection cuts
    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt> 0.1', path=path)
    fillParticleList(decayString='gamma:eventShapeForSkims',
                     cut='E > 0.1', path=path)

    buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
                    allMoments=False,
                    foxWolfram=True,
                    harmonicMoments=False,
                    cleoCones=False,
                    thrust=False,
                    collisionAxis=False,
                    jets=False,
                    sphericity=False,
                    checkForDuplicates=False,
                    path=path)

    applyEventCuts('foxWolframR2<0.4 and nTracks>=4', path=path)

    # Run FEI
    basf2.use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')

    particles = fei.get_default_channels(neutralB=False, chargedB=True, hadronic=True, semileptonic=False, KLong=False)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2019_MC12_release_03_01_01', training=False, monitor=False)
    feistate = fei.get_path(particles, configuration)
    path.add_path(feistate.path)


def runFEIforHadronicCombined(path):
    """Generates FEI B+:generic and B0:generic list for FEI Hadronic skims, including
    applying FEI weights and skim pre-selection cuts.
    Use BplusHadronic(path) and B0Hadronic(path) for skim cuts on FEI output lists

    (Semi-)Leptonic and Missing Energy Working Group.

    Skim LFN code: 11180100, 11180200

    Physics channels: (All available FEI B0 and B+ Hadronic tags are
    reconstructed)

    FEI weightfiles: FEIv4_2019_MC12_release_03_01_01

    Skim Liaisons: S. Hollitt, H. Wakeling, & P. Grace

    **Cuts applied are**:

        Event precuts:

        * foxWolframR2 < 0.4
        * nTracks >= 4

    Parameters:
        path (basf2.Path) the path to add the skim list builders

    Returns:
        list name of the skim candidates
    """
    # Pre-selection cuts
    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt> 0.1', path=path)
    fillParticleList(decayString='gamma:eventShapeForSkims',
                     cut='E > 0.1', path=path)

    buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
                    allMoments=False,
                    foxWolfram=True,
                    harmonicMoments=False,
                    cleoCones=False,
                    thrust=False,
                    collisionAxis=False,
                    jets=False,
                    sphericity=False,
                    checkForDuplicates=False,
                    path=path)

    applyEventCuts('foxWolframR2<0.4 and nTracks>=4', path=path)

    # Run FEI
    basf2.use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')

    particles = fei.get_default_channels(neutralB=True, chargedB=True, hadronic=True, semileptonic=False, KLong=False)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2019_MC12_release_03_01_01', training=False, monitor=False)
    feistate = fei.get_path(particles, configuration)
    path.add_path(feistate.path)


def B0SL(path):
    """FEI Semi-Leptonic B0 tag skim list for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group.

    Skim LFN code: 11180300

    Physics channels: (FEI B0 SL tags are reconstructed. Hadronic B with SL D
    not reconstructed)

    * B0 -> D- e+ nu
    * B0 -> D- mu+ nu
    * B0 -> D-* e+ nu
    * B0 -> D-* mu+ nu
    * B0 -> anti-D0 pi- e+ nu
    * B0 -> anti-D0 pi- mu+ nu
    * B0 -> anti-D0* pi- e+ nu
    * B0 -> anti-D0* pi- mu+ nu

    This function applies cuts to the FEI-reconstructed tag side B, and
    the pre-cuts and FEI must be applied separately.

    Skimming script reconstructs SL Btag using generically trained FEI.
    FEI is run with removeSLD=True to deactivate rare but time-intensive
    semileptonic D channels in skim.

    Skim Liaisons: S. Hollitt, H. Wakeling, & P. Grace

    **Cuts applied are**:

        Event precuts:

        * foxWolframR2 < 0.4
        * nTracks >= 4

        Tag side B:

        * -4 < cosThetaBetweenParticleAndNominalB < 3
        * extraInfo(decayModeID) < 8 to remove semileptonic D channels
        * log10(sigProb) > -2.4 to lower retention (corresponds to sigProb > 0.003981)
        * lepton momentum > 1.0 in CMS frame (daughter(1,p) or daughter(2,p), depending on dmID)

    Parameters:
        path (basf2.Path) the path to add the skim list builders

    Returns:
        list name of the skim candidates

    """
    # Apply cuts
    applyCuts('B0:semileptonic', 'log10_sigProb>-2.4', path=path)
    applyCuts('B0:semileptonic', '-4.0<cosThetaBY<3.0', path=path)
    applyCuts('B0:semileptonic', 'dmID<8', path=path)
    # Decay mode IDs 0--3 (B -> D l) need to be treated differently to
    # IDs 4--7 (B -> D pi l) to make a cut on tag-side lepton momentum
    applyCuts('B0:semileptonic', '[[dmID<4 and d1_p_CMSframe>1.0] or [dmID>=4 and d2_p_CMSframe>1.0]]', path=path)

    BtagList = ['B0:semileptonic']
    return BtagList


def BplusSL(path):
    """FEI semi-leptonic (SL) Bplus tag for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group.

    Skim LFN code: 11180400

    Physics channels: (FEI Bplus SL tags are reconstructed.
    Hadronic Bplus with SL D meson not reconstructed)

    * B+ -> anti-D0 e+ nu
    * B+ -> anti-D0 mu+ nu
    * B+ -> anti-D0* e+ nu
    * B+ -> anti-D0* mu+ nu
    * B+ -> D- pi+ e+ nu
    * B+ -> D- pi+ mu+ nu
    * B+ -> D-* pi+ e+ nu
    * B+ -> D-* pi+ mu+ nu

    This function applies cuts to the FEI-reconstructed tag side B, and
    the pre-cuts and FEI must be applied separately.

    Skimming script reconstructs SL Btag using generically trained FEI.
    FEI is run with removeSLD=True to deactivate rare but time-intensive
    semileptonic D channels in skim.

    Skim Liaisons: S. Hollitt, H. Wakeling, & P. Grace

        Event precuts:

        * foxWolframR2 < 0.4
        * nTracks >= 4

        Tag side B:

        * -4 < cosThetaBetweenParticleAndNominalB < 3
        * extraInfo(decayModeID) < 8 to remove semileptonic D channels
        * log10(sigProb) > -2.4 to lower retention (corresponds to sigProb > 0.003981)
        * lepton momentum > 1.0 in CMS frame (daughter(1,p) or daughter(2,p), depending on dmID)

    Parameters:
        path (basf2.Path) the path to add the skim list builders

    Returns:
        list name of the skim candidates

    """
    # Apply cuts
    BplusSLcuts = ['log10_sigProb>-2.4', '-4.0<cosThetaBY<3.0', 'dmID<8',
                   # Decay mode IDs 0--3 (B -> D l) need to be treated differently to
                   # IDs 4--7 (B -> D pi l) to make a cut on tag-side lepton momentum.
                   '[[dmID<4 and d1_p_CMSframe>1.0] or [dmID>=4 and d2_p_CMSframe>1.0]]']

    applyCuts('B+:semileptonic', ' and '.join(BplusSLcuts), path=path)

    BtagList = ['B+:semileptonic']
    return BtagList


def runFEIforB0SL(path):
    """Generates FEI B0:semileptonic list with FEI removeSLD=True flag.
    This includes applying FEI weights and skim
    pre-selection cuts. Use B0SL(path) for skim cuts on FEI output list.

    (Semi-)Leptonic and Missing Energy Working Group

    Skim LFN code: 11180300

    Physics channels: (FEI B0 SL tags are reconstructed. Hadronic B0 with SL D
    not reconstructed)

    * B0 -> D- e+ nu
    * B0 -> D- mu+ nu
    * B0 -> D+* e+ nu
    * B0 -> D+* mu+ nu
    * B0 -> anti-D0 pi- e+ nu
    * B0 -> anti-D0 pi- mu+ nu
    * B0 -> anti-D0* pi- e+ nu
    * B0 -> anti-D0* pi- mu+ nu

    FEI weightfiles: FEIv4_2019_MC12_release_03_01_01

    Skimming script reconstructs SL Btag using generically trained FEI.
    FEI is run with removeSLD=True to deactivate rare but time-intensive
    semileptonic D channels in skim.

    Skim Liaisons: S. Hollitt, H. Wakeling, & P. Grace

    **Cuts applied are**:

        Event precuts:

        * foxWolframR2 < 0.4
        * nTracks >= 4

        Tag side B:

        * -4 < cosThetaBetweenParticleAndNominalB < 3
        * extraInfo(decayModeID) < 8 to remove semileptonic D channels
        * log10(sigProb) > -2.4 to lower retention (corresponds to sigProb > 0.003981)
        * useCMSFrame(daughter(1,p)) > 1.0

    Parameters:
        path (basf2.Path) the path to add the skim list builders

    Returns:
        list name of the skim candidates

    """
    # Pre-selection cuts
    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt> 0.1', path=path)
    fillParticleList(decayString='gamma:eventShapeForSkims',
                     cut='E > 0.1', path=path)

    buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
                    allMoments=False,
                    foxWolfram=True,
                    harmonicMoments=False,
                    cleoCones=False,
                    thrust=False,
                    collisionAxis=False,
                    jets=False,
                    sphericity=False,
                    checkForDuplicates=False,
                    path=path)

    applyEventCuts('foxWolframR2<0.4 and nTracks>=4', path=path)

    # Run FEI
    basf2.use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')

    particles = fei.get_default_channels(
        neutralB=True,
        chargedB=False,
        hadronic=False,
        semileptonic=True,
        KLong=False,
        removeSLD=True)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2019_MC12_release_03_01_01', training=False, monitor=False)
    feistate = fei.get_path(particles, configuration)
    path.add_path(feistate.path)


def runFEIforBplusSL(path):
    """Generates FEI B+:semileptonic list with FEI removeSLD=True flag.
    This includes applying FEI weights and skim
    pre-selection cuts. Use BplusSL(path) for skim cuts on FEI output list.

    (Semi-)Leptonic and Missing Energy Working Group

    Skim LFN code: 11180400

    Physics channels: (FEI Bplus SL tags are reconstructed.
    Hadronic Bplus with SL D meson not reconstructed)

    * B+ -> anti-D0 e+ nu
    * B+ -> anti-D0 mu+ nu
    * B+ -> anti-D0* e+ nu
    * B+ -> anti-D0* mu+ nu
    * B+ -> D- pi+ e+ nu
    * B+ -> D- pi+ mu+ nu
    * B+ -> D+* pi+ e+ nu
    * B+ -> D+* pi+ mu+ nu

    FEI weightfiles: FEIv4_2019_MC12_release_03_01_01

    Skimming script reconstructs SL Btag using generically trained FEI.
    FEI is run with removeSLD=True to deactivate rare but time-intensive
    semileptonic D channels in skim.

    Skim Liaisons: S. Hollitt, H. Wakeling, & P. Grace

    **Cuts applied are**:

        Event precuts:

        * foxWolframR2 < 0.4
        * nTracks >= 4

        Tag side B:

        * -4 < cosThetaBetweenParticleAndNominalB < 3
        * extraInfo(decayModeID) < 8 to remove semileptonic D channels
        * log10(sigProb) > -2.4 to lower retention (corresponds to sigProb > 0.003981)
        * useCMSFrame(daughter(1,p)) > 1.0

        Signal side:

        * electron or muon from list 95eff
        * B Mbc > 0

    Parameters:
        path (basf2.Path) the path to add the skim list builders
    Returns:
        list name of the skim candidates

    """
    # Pre-selection cuts
    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt> 0.1', path=path)
    fillParticleList(decayString='gamma:eventShapeForSkims',
                     cut='E > 0.1', path=path)

    buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
                    allMoments=False,
                    foxWolfram=True,
                    harmonicMoments=False,
                    cleoCones=False,
                    thrust=False,
                    collisionAxis=False,
                    jets=False,
                    sphericity=False,
                    checkForDuplicates=False,
                    path=path)

    applyEventCuts('foxWolframR2<0.4 and nTracks>=4', path=path)

    # Run FEI
    basf2.use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')

    particles = fei.get_default_channels(
        neutralB=False,
        chargedB=True,
        hadronic=False,
        semileptonic=True,
        KLong=False,
        removeSLD=True)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2019_MC12_release_03_01_01', training=False, monitor=False)
    feistate = fei.get_path(particles, configuration)
    path.add_path(feistate.path)


def runFEIforSLCombined(path):
    """Generates FEI B+:semileptonic and B0:semileptonic lists
    (without semileptonic D candidates) for FEI Semileptonic+lepton skims, including
    applying FEI weights and skim pre-selection cuts.
    Use BplusSL(path) and B0SL(path) for skim cuts on FEI output lists

    (Semi-)Leptonic and Missing Energy Working Group.

    Skim LFN code: 11180300, 11180400

    Physics channels: (FEI B0 and B+ semileptonic tags are
    reconstructed. The Hadronic B with semileptonic D tags are
    not reconstructed)

    FEI weightfiles: FEIv4_2019_MC12_release_03_01_01

    Skim Liaisons: S. Hollitt, H. Wakeling, & P. Grace

    **Cuts applied are**:

        Event precuts:

        * foxWolframR2 < 0.4
        * nTracks >= 4
    """
    # Pre-selection cuts
    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt> 0.1', path=path)
    fillParticleList(decayString='gamma:eventShapeForSkims',
                     cut='E > 0.1', path=path)

    buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
                    allMoments=False,
                    foxWolfram=True,
                    harmonicMoments=False,
                    cleoCones=False,
                    thrust=False,
                    collisionAxis=False,
                    jets=False,
                    sphericity=False,
                    checkForDuplicates=False,
                    path=path)

    applyEventCuts('foxWolframR2<0.4 and nTracks>=4', path=path)

    # Run FEI
    basf2.use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')

    particles = fei.get_default_channels(
        neutralB=True,
        chargedB=True,
        hadronic=False,
        semileptonic=True,
        KLong=False,
        removeSLD=True)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2019_MC12_release_03_01_01', training=False, monitor=False)
    feistate = fei.get_path(particles, configuration)
    path.add_path(feistate.path)


def runFEIforSkimCombined(path):
    """Generates FEI B0:generic, B+:generic, B+:semileptonic and B0:semileptonic lists
    (without semileptonic D candidates) for all FEI skims, including
    applying FEI weights and skim pre-selection cuts.
    Use B0Hadronic(path), BplusHadronic(path), BplusSL(path)
    and B0SL(path) for skim cuts on FEI output lists

    (Semi-)Leptonic and Missing Energy Working Group.

    Skim LFN code: 11180100, 11180200, 11180300, 11180400

    Physics channels: (FEI B0 and B+ hadronic and semileptonic tags are
    reconstructed. The Hadronic B with semileptonic D tags from the semileptonic
    FEI are not reconstructed)

    FEI weightfiles: FEIv4_2019_MC12_release_03_01_01

    Skim Liaisons: S. Hollitt, H. Wakeling, & P. Grace

    **Cuts applied are**:

        Event precuts:

        * foxWolframR2 < 0.4
        * nTracks >= 4

    Parameters:
        path (basf2.Path) the path to add the skim list builders

    Returns:
        list name of the skim candidates
    """
    # Pre-selection cuts
    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt> 0.1', path=path)
    fillParticleList(decayString='gamma:eventShapeForSkims',
                     cut='E > 0.1', path=path)

    buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
                    allMoments=False,
                    foxWolfram=True,
                    harmonicMoments=False,
                    cleoCones=False,
                    thrust=False,
                    collisionAxis=False,
                    jets=False,
                    sphericity=False,
                    checkForDuplicates=False,
                    path=path)

    applyEventCuts('foxWolframR2<0.4 and nTracks>=4', path=path)

    # Run FEI
    basf2.use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')

    particles = fei.get_default_channels(
        neutralB=True,
        chargedB=True,
        hadronic=True,
        semileptonic=True,
        KLong=False,
        removeSLD=True)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2019_MC12_release_03_01_01', training=False, monitor=False)
    feistate = fei.get_path(particles, configuration)
    path.add_path(feistate.path)
