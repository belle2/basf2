#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
(Semi-)Leptonic Working Group Skims for missing energy modes that use the `FullEventInterpretation` (FEI) algorithm.
"""

__authors__ = [
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
    """
    Note:
        * **Skim description**: Hadronic :math:`B^0` tag FEI skim for
          generic analysis.
        * **Skim LFN code**: 11180100
        * **FEI training**: FEIv4_2019_MC12_release_03_01_01
        * **Working Group**: (Semi-)Leptonic and Missing Energy
          Working Group (WG1)
        * **Skim liaisons**: Hannah Wakeling & Phil Grace

    Apply cuts to the FEI-reconstructed hadronic :math:`B` tag
    candidates in list ``B0:generic``, and supply the name of the
    list.

    One of `skim.fei.runFEIforB0Hadronic`,
    `skim.fei.runFEIforHadronicCombined` or
    `skim.fei.runFEIforSkimCombined` must be run first to reconstruct
    hadronic tag :math:`B`'s.

    Example usage:

    >>> from skim.fei import runFEIforB0Hadronic, B0hadronic
    >>> runFEIforB0Hadronic(path)
    >>> B0hadronic(path)
    ['B0:generic']

    Tag modes
        All available FEI :math:`B^0` hadronic tags are reconstructed.

        * :math:`B^0 \\to D^- \\pi^+`
        * :math:`B^0 \\to D^- \\pi^+ \\pi^0`
        * :math:`B^0 \\to D^- \\pi^+ \\pi^0 \\pi^0`
        * :math:`B^0 \\to D^- \\pi^+ \\pi^+ \\pi^-`
        * :math:`B^0 \\to D^- \\pi^+ \\pi^+ \\pi^- \\pi^0`
        * :math:`B^0 \\to \\overline{D}^0 \\pi^+ \\pi^0`
        * :math:`B^0 \\to D^- D^0 K^+`
        * :math:`B^0 \\to D^- D^{0*}(2010) K^+`
        * :math:`B^0 \\to D^{+*} D^0 K^+`
        * :math:`B^0 \\to D^{+*} D^{0*}(2010) K^+`
        * :math:`B^0 \\to D^- D^+ K_S^0`
        * :math:`B^0 \\to D^{+*} D^+ K_S^0`
        * :math:`B^0 \\to D^- D^{+*} K_S^0`
        * :math:`B^0 \\to D^{+*} D^{+*} K_S^0`
        * :math:`B^0 \\to D_s^+ D^-`
        * :math:`B^0 \\to D^{+*} \\pi^+`
        * :math:`B^0 \\to D^{+*} \\pi^+ \\pi^0`
        * :math:`B^0 \\to D^{+*} \\pi^+ \\pi^0 \\pi^0`
        * :math:`B^0 \\to D^{+*} \\pi^+ \\pi^+ \\pi^-`
        * :math:`B^0 \\to D^{+*} \\pi^+ \\pi^+ \\pi^- \\pi^0`
        * :math:`B^0 \\to D_s^{+*} D^-`
        * :math:`B^0 \\to D_s^+ D^{+*}`
        * :math:`B^0 \\to D_s^{+*} D^{+*}`
        * :math:`B^0 \\to J/\\psi\\, K_S^0`
        * :math:`B^0 \\to J/\\psi\\, K^+ \\pi^-`
        * :math:`B^0 \\to J/\\psi\\, K_S^0 \\pi^+ \\pi^-`

        From `Thomas Keck's thesis <https://docs.belle2.org/record/275/files/BELLE2-MTHESIS-2015-001.pdf>`_,
        "the channel :math:`B^0 \\to \\overline{D}^0 \\pi^0` was used
        by the FR, but is not yet used in the FEI due to unexpected
        technical restrictions in the KFitter algorithm".

    Cuts applied
        Event pre-cuts:

        * :math:`R_2 < 0.4` (`foxWolframR2` from
          `modularAnalysis.buildEventShape`, calculated using all
          neutral clusters with :math:`E>0.1\\,\\text{GeV}`, and all
          charged tracks with :math:`p_T>0.1\\,\\text{GeV}`)
        * :math:`n_{\\text{tracks}} \\geq 4`

        Tag side :math:`B` cuts:

        * :math:`M_{\\text{bc}} > 5.24\\,\\text{GeV}`
        * :math:`|\\Delta E| < 0.2\\,\\text{GeV}`
        * :math:`\\text{signal probability} > 0.001`

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        ``B0hadronicList``, a Python list containing the string
        :code:`'B0:generic'`, the name of the particle list for
        hadronic :math:`B^0` skim candidates.
    """

    applyCuts('B0:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001', path=path)

    B0hadronicList = ['B0:generic']
    return B0hadronicList


def BplusHadronic(path):
    """
    Note:
        * **Skim description**: Hadronic :math:`B^+` tag FEI skim for
          generic analysis.
        * **Skim LFN code**: 11180200
        * **FEI training**: FEIv4_2019_MC12_release_03_01_01
        * **Working Group**: (Semi-)Leptonic and Missing Energy
          Working Group (WG1)
        * **Skim liaisons**: Hannah Wakeling & Phil Grace

    Apply cuts to the FEI-reconstructed hadronic :math:`B` tag
    candidates in list ``B+:generic``, and supply the name of the
    list.

    One of `skim.fei.runFEIforBplusHadronic`,
    `skim.fei.runFEIforHadronicCombined` or
    `skim.fei.runFEIforSkimCombined` must be run first to reconstruct
    hadronic tag :math:`B`'s.

    Example usage:

    >>> from skim.fei import runFEIforBplusHadronic, BplusHadronic
    >>> runFEIforBplusHadronic(path)
    >>> BplusHadronic(path)
    ['B+:generic']

    Tag modes
        All available FEI :math:`B^+` hadronic tags are reconstructed.

        * :math:`B^+ \\to \\overline{D}^0 \\pi^+`
        * :math:`B^+ \\to \\overline{D}^0 \\pi^+ \\pi^0`
        * :math:`B^+ \\to \\overline{D}^0 \\pi^+ \\pi^0 \\pi^0`
        * :math:`B^+ \\to \\overline{D}^0 \\pi^+ \\pi^+ \\pi^-`
        * :math:`B^+ \\to \\overline{D}^0 \\pi^+ \\pi^+ \\pi^- \\pi^0`
        * :math:`B^+ \\to \\overline{D}^0 D^+`
        * :math:`B^+ \\to \\overline{D}^0 D^+ K_S^0`
        * :math:`B^+ \\to \\overline{D}^{0*} D^+ K_S^0`
        * :math:`B^+ \\to \\overline{D}^0 D^{+*} K_S^0`
        * :math:`B^+ \\to \\overline{D}^{0*} D^{+*} K_S^0`
        * :math:`B^+ \\to \\overline{D}^0 D^0 K^+`
        * :math:`B^+ \\to \\overline{D}^{0*} D^0 K^+`
        * :math:`B^+ \\to \\overline{D}^0 D^{0*}(2010) K^+`
        * :math:`B^+ \\to \\overline{D}^{0*} D^{0*}(2010) K^+`
        * :math:`B^+ \\to D_s^+ \\overline{D}^0`
        * :math:`B^+ \\to \\overline{D}^{0*} \\pi^+`
        * :math:`B^+ \\to \\overline{D}^{0*} \\pi^+ \\pi^0`
        * :math:`B^+ \\to \\overline{D}^{0*} \\pi^+ \\pi^0 \\pi^0`
        * :math:`B^+ \\to \\overline{D}^{0*} \\pi^+ \\pi^+ \\pi^-`
        * :math:`B^+ \\to \\overline{D}^{0*} \\pi^+ \\pi^+ \\pi^- \\pi^0`
        * :math:`B^+ \\to D_s^{+*} \\overline{D}^0`
        * :math:`B^+ \\to D_s^+ \\overline{D}^{0*}`
        * :math:`B^+ \\to \\overline{D}^0 K^+`
        * :math:`B^+ \\to D^- \\pi^+ \\pi^+`
        * :math:`B^+ \\to D^- \\pi^+ \\pi^+ \\pi^0`
        * :math:`B^+ \\to J/\\psi\\, K^+`
        * :math:`B^+ \\to J/\\psi\\, K^+ \\pi^+ \\pi^-`
        * :math:`B^+ \\to J/\\psi\\, K^+ \\pi^0`
        * :math:`B^+ \\to J/\\psi\\, K_S^0 \\pi^+`

    Cuts applied
        Event pre-cuts:

        * :math:`R_2 < 0.4` (`foxWolframR2` from
          `modularAnalysis.buildEventShape`, calculated using all
          neutral clusters with :math:`E>0.1\\,\\text{GeV}`, and all
          charged tracks with :math:`p_T>0.1\\,\\text{GeV}`)
        * :math:`n_{\\text{tracks}} \\geq 4`

        Tag side :math:`B` cuts:

        * :math:`M_{\\text{bc}} > 5.24\\,\\text{GeV}`
        * :math:`|\\Delta E| < 0.2\\,\\text{GeV}`
        * :math:`\\text{signal probability} > 0.001`

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        ``BplusHadronicList``, a Python list containing the string
        :code:`'B+:generic'`, the name of the particle list for
        hadronic :math:`B^+` skim candidates.
    """

    # B+:generic list from FEI must already exist in path
    # Apply cuts
    applyCuts('B+:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001', path=path)

    BplusHadronicList = ['B+:generic']
    return BplusHadronicList


def runFEIforB0Hadronic(path):
    """
    Reconstruct hadronic :math:`B^0` tags using the generically
    trained FEI. Skim pre-cuts are applied before running the FEI.

    Remaining skim cuts are applied by `skim.fei.B0hadronic`.

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        No return value. Fills particle list :code:`B0:generic` for
        supplied path.

    See also:
        `skim.fei.B0hadronic` for skim details, FEI training, list of
        reconstructed tag modes, and pre-cuts applied.
    """
    # Pre-selection cuts
    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt > 0.1', path=path)
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
    """
    Reconstruct hadronic :math:`B^0` tags using the generically
    trained FEI. Skim pre-cuts are applied before running the FEI.

    Remaining skim cuts are applied by `skim.fei.BplusHadronic`.

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        No return value. Fills particle list :code:`B+:generic` for
        supplied path.

    See also:
        `skim.fei.BplusHadronic` for skim details, FEI training, list
        of reconstructed tag modes, and pre-cuts applied.
    """
    # Pre-selection cuts
    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt > 0.1', path=path)
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
    """
    Reconstruct hadronic :math:`B^0` and :math:`B^+` tags using the
    generically trained FEI. Skim pre-cuts are applied before running
    the FEI (the pre-cuts are the same for all FEI skims, and are
    documented in their respective list functions).

    Remaining cuts for skims are applied by `skim.fei.B0hadronic` and
    `skim.fei.BplusHadronic`.

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders

    Returns:
        No return value. Fills particle lists :code:`B0:generic` and
        :code:`B+:generic` for supplied path.

    See also:
        `skim.fei.B0hadronic` and `skim.fei.BplusHadronic` for skim
        details, FEI training, lists of reconstructed tag modes, and
        pre-cuts applied.
    """
    # Pre-selection cuts
    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt > 0.1', path=path)
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
    """
    Note:
        * **Skim description**: Semileptonic :math:`B^0` tag FEI skim
          for generic analysis.
        * **Skim LFN code**: 11180300
        * **FEI training**: FEIv4_2019_MC12_release_03_01_01
        * **Working Group**: (Semi-)Leptonic and Missing Energy
          Working Group (WG1)
        * **Skim liaisons**: Hannah Wakeling & Phil Grace

    Apply cuts to the FEI-reconstructed SL :math:`B` tag candidates in
    list ``B0:semileptonic``, and supply the name of the list.

    One of `skim.fei.runFEIforB0SL`, `skim.fei.runFEIforSLCombined` or
    `skim.fei.runFEIforSkimCombined` must be run first to reconstruct
    SL tag :math:`B`'s.

    Example usage:

    >>> from skim.fei import runFEIforB0SL, B0SL
    >>> runFEIforB0SL(path)
    >>> B0SL(path)
    ['B0:semileptonic']

    Tag modes
        SL :math:`B^0` tags are reconstructed. Hadronic :math:`B` with
        SL :math:`D` are not reconstructed.
        * :math:`B^0 \\to D^- \\mu^+ \\nu_{e}`
        * :math:`B^0 \\to D^- \\mu^+ \\nu_{\\mu}`
        * :math:`B^0 \\to D^{-*} \\mu^+ \\nu_{e}`
        * :math:`B^0 \\to D^{-*} \\mu^+ \\nu_{\\mu}`
        * :math:`B^0 \\to \\overline{D}^0 \\pi^- \\mu^+ \\nu_{e}`
        * :math:`B^0 \\to \\overline{D}^0 \\pi^- \\mu^+ \\nu_{\\mu}`
        * :math:`B^0 \\to \\overline{D}^{0*} \\pi^- \\mu^+ \\nu_{e}`
        * :math:`B^0 \\to \\overline{D}^{0*} \\pi^- \\mu^+ \\nu_{\\mu}`

    Cuts applied
        Event pre-cuts:

        * :math:`R_2 < 0.4` (`foxWolframR2` from
          `modularAnalysis.buildEventShape`, calculated using all
          neutral clusters with :math:`E>0.1\\,\\text{GeV}`, and all
          charged tracks with :math:`p_T>0.1\\,\\text{GeV}`)
        * :math:`n_{\\text{tracks}} \\geq 4`

        Tag side :math:`B` cuts:

        * :math:`-4 < \\cos\\theta_{BY} < 3`
        * :math:`\\text{Decay mode ID} < 8` (no SL :math:`D` channels)
        * :math:`\\log_{10}(\\text{signal probability}) > -2.4`
        * :math:`p_{\\ell}^{*} > 1.0\\,\\text{GeV}` in CMS frame
          (``daughter(1,p)>1.0`` or ``daughter(2,p)>1.0``, depending
          on decay mode ID)

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        ``B0SLList``, a Python list containing the string
        :code:`'B0:semileptonic'`, the name of the particle list for
        SL :math:`B^0` skim candidates.
    """
    # Apply cuts
    B0SLcuts = ['log10_sigProb>-2.4', '-4.0<cosThetaBY<3.0', 'dmID<8',
                # Decay mode IDs 0--3 (B -> D l) need to be treated differently to
                # IDs 4--7 (B -> D pi l) to make a cut on tag-side lepton momentum.
                '[[dmID<4 and d1_p_CMSframe>1.0] or [dmID>=4 and d2_p_CMSframe>1.0]]']

    applyCuts('B0:semileptonic', ' and '.join(B0SLcuts), path=path)

    B0SLList = ['B0:semileptonic']
    return B0SLList


def BplusSL(path):
    """
    Note:
        * **Skim description**: Semileptonic :math:`B^+` tag FEI skim
          for generic analysis.
        * **Skim LFN code**: 11180400
        * **FEI training**: FEIv4_2019_MC12_release_03_01_01
        * **Working Group**: (Semi-)Leptonic and Missing Energy
          Working Group (WG1)
        * **Skim liaisons**: Hannah Wakeling & Phil Grace

    Apply cuts to the FEI-reconstructed SL :math:`B` tag candidates in
    list ``B+:semileptonic``, and supply the name of the list.

    One of `skim.fei.runFEIforBplusSL`, `skim.fei.runFEIforSLCombined`
    or `skim.fei.runFEIforSkimCombined` must be run first to
    reconstruct SL tag :math:`B`'s.

    Example usage:

    >>> from skim.fei import runFEIforBplusSL, BplusSL
    >>> runFEIforBplusSL(path)
    >>> BplusSL(path)
    ['B+:semileptonic']

    Tag modes
        SL :math:`B^+` tags are reconstructed. Hadronic :math:`B^+`
        with SL :math:`D` are not reconstructed.

        * :math:`B^+ \\to \\overline{D}^0 \\mu^+ \\nu_{e}`
        * :math:`B^+ \\to \\overline{D}^0 \\mu^+ \\nu_{\\mu}`
        * :math:`B^+ \\to \\overline{D}^{0*} \\mu^+ \\nu_{e}`
        * :math:`B^+ \\to \\overline{D}^{0*} \\mu^+ \\nu_{\\mu}`
        * :math:`B^+ \\to D^- \\pi^+ \\mu^+ \\nu_{e}`
        * :math:`B^+ \\to D^- \\pi^+ \\mu^+ \\nu_{\\mu}`
        * :math:`B^+ \\to D^{-*} \\pi^+ \\mu^+ \\nu_{e}`
        * :math:`B^+ \\to D^{-*} \\pi^+ \\mu^+ \\nu_{\\mu}`

    Cuts applied
        Event pre-cuts:

        * :math:`R_2 < 0.4` (`foxWolframR2` from
          `modularAnalysis.buildEventShape`, calculated using all
          neutral clusters with :math:`E>0.1\\,\\text{GeV}`, and all
          charged tracks with :math:`p_T>0.1\\,\\text{GeV}`)
        * :math:`n_{\\text{tracks}} \\geq 4`

        Tag side :math:`B` cuts:

        * :math:`-4 < \\cos\\theta_{BY} < 3`
        * :math:`\\text{Decay mode ID} < 8` (no SL :math:`D` channels)
        * :math:`\\log_{10}(\\text{signal probability}) > -2.4`
        * :math:`p_{\\ell}^{*} > 1.0\\,\\text{GeV}` in CMS frame
          (``daughter(1,p)>1.0`` or ``daughter(2,p)>1.0``, depending
          on decay mode ID)

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        ``BplusSLList``, a Python list containing the string
        :code:`'B+:semileptonic'`, the name of the particle list for
        SL :math:`B^+` skim candidates.
    """
    # Apply cuts
    BplusSLcuts = ['log10_sigProb>-2.4', '-4.0<cosThetaBY<3.0', 'dmID<8',
                   # Decay mode IDs 0--3 (B -> D l) need to be treated differently to
                   # IDs 4--7 (B -> D pi l) to make a cut on tag-side lepton momentum.
                   '[[dmID<4 and d1_p_CMSframe>1.0] or [dmID>=4 and d2_p_CMSframe>1.0]]']

    applyCuts('B+:semileptonic', ' and '.join(BplusSLcuts), path=path)

    BplusSLList = ['B+:semileptonic']
    return BplusSLList


def runFEIforB0SL(path):
    """
    Reconstruct semileptonic :math:`B^0` tags using the generically
    trained FEI. Skim pre-cuts are applied before running the FEI. FEI
    is run with ``removeSLD=True`` flag to deactivate rare but
    time-intensive SL :math:`D` channels in skim.

    Remaining skim cuts are applied by `skim.fei.B0SL`.

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        No return value. Fills particle list :code:`B0:semileptonic`
        for supplied path.

    See also:
        `skim.fei.B0SL` for skim details, FEI training, list of
        reconstructed tag modes, and pre-cuts applied.
    """
    # Pre-selection cuts
    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt > 0.1', path=path)
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
    """
    Reconstruct semileptonic :math:`B^+` tags using the generically
    trained FEI. Skim pre-cuts are applied before running the FEI. FEI
    is run with ``removeSLD=True`` flag to deactivate rare but
    time-intensive SL :math:`D` channels in skim.

    Remaining skim cuts are applied by `skim.fei.BplusSL`.

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        No return value. Fills particle list :code:`B+:semileptonic`
        for supplied path.

    See also:
        `skim.fei.BplusSL` for skim details, FEI training, list of
        reconstructed tag modes, and pre-cuts applied.
    """
    # Pre-selection cuts
    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt > 0.1', path=path)
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
    """
    Reconstruct semileptonic :math:`B^0` and :math:`B^+` tags using
    the generically trained FEI. Skim pre-cuts are applied before
    running the FEI (the pre-cuts are the same for all FEI skims, and
    are documented in their respective list functions). FEI is run
    with ``removeSLD=True`` flag to deactivate rare but time-intensive
    SL :math:`D` channels in skim.

    Remaining cuts for skims are applied by `skim.fei.B0SL` and
    `skim.fei.BplusSL`.

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders

    Returns:
        No return value. Fills particle lists :code:`B0:semileptonic`
        and :code:`B+:semileptonic` for supplied path.

    See also:
        `skim.fei.B0SL` and `skim.fei.BplusSL` for skim details, FEI
        training, lists of reconstructed tag modes, and pre-cuts
        applied.
    """
    # Pre-selection cuts
    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt > 0.1', path=path)
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
    """
    Reconstruct hadronic and semileptonic :math:`B^0` and :math:`B^+`
    tags using the generically trained FEI. Skim pre-cuts are applied
    before running the FEI (the pre-cuts are the same for all FEI
    skims, and are documented in their respective list functions). FEI
    is run with ``removeSLD=True`` flag to deactivate rare but
    time-intensive SL :math:`D` channels in skim.

    Remaining cuts for skims are applied by `skim.fei.B0hadronic`,
    `skim.fei.BplusHadronic`, `skim.fei.B0SL`, and `skim.fei.BplusSL`.

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders

    Returns:
        No return value. Fills particle lists :code:`B0:generic`,
        :code:`B+:generic`, :code:`B+:semileptonic`, and
        :code:`B0:semileptonic` for supplied path.

    See also:
        `skim.fei.B0hadronic`, `skim.fei.BplusHadronic`,
        `skim.fei.B0SL`, and `skim.fei.BplusSL` for skim details, FEI
        training, lists of reconstructed tag modes, and pre-cuts
        applied.
    """
    # Pre-selection cuts
    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt > 0.1', path=path)
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
