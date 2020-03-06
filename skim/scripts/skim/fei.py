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

import basf2 as b2
import fei
import modularAnalysis as ma

from variables import variables as vm
vm.addAlias('sigProb', 'extraInfo(SignalProbability)')
vm.addAlias('log10_sigProb', 'log10(extraInfo(SignalProbability))')
vm.addAlias('dmID', 'extraInfo(decayModeID)')
vm.addAlias('foxWolframR2_maskedNaN', 'ifNANgiveX(foxWolframR2,1)')
vm.addAlias('cosThetaBY', 'cosThetaBetweenParticleAndNominalB')
vm.addAlias('d1_p_CMSframe', 'useCMSFrame(daughter(1,p))')
vm.addAlias('d2_p_CMSframe', 'useCMSFrame(daughter(2,p))')


def B0Hadronic(path):
    """
    Note:
        * **Skim description**: Hadronic :math:`B^0` tag FEI skim for
          generic analysis.
        * **Skim LFN code**: 11180100
        * **FEI training**: FEIv4_2020_MC13_release_04_01_01
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

    >>> from skim.fei import runFEIforB0Hadronic, B0Hadronic
    >>> runFEIforB0Hadronic(path)
    >>> B0Hadronic(path)
    ['B0:generic']

    Tag modes
        All available FEI :math:`B^0` hadronic tags are reconstructed.

        0. :math:`B^0 \\to D^- \\pi^+`
        1. :math:`B^0 \\to D^- \\pi^+ \\pi^0`
        2. :math:`B^0 \\to D^- \\pi^+ \\pi^0 \\pi^0`
        3. :math:`B^0 \\to D^- \\pi^+ \\pi^+ \\pi^-`
        4. :math:`B^0 \\to D^- \\pi^+ \\pi^+ \\pi^- \\pi^0`
        5. :math:`B^0 \\to \\overline{D^0} \\pi^+ \\pi^-`
        6. :math:`B^0 \\to D^- D^0 K^+`
        7. :math:`B^0 \\to D^- D^{0*} K^+`
        8. :math:`B^0 \\to D^{-*} D^0 K^+`
        9. :math:`B^0 \\to D^{-*} D^{0*} K^+`
        10. :math:`B^0 \\to D^- D^+ K^0_S`
        11. :math:`B^0 \\to D^{-*} D^+ K^0_S`
        12. :math:`B^0 \\to D^- D^{+*} K^0_S`
        13. :math:`B^0 \\to D^{-*} D^{+*} K^0_S`
        14. :math:`B^0 \\to D^+_s D^-`
        15. :math:`B^0 \\to D^{-*} \\pi^+`
        16. :math:`B^0 \\to D^{-*} \\pi^+ \\pi^0`
        17. :math:`B^0 \\to D^{-*} \\pi^+ \\pi^0 \\pi^0`
        18. :math:`B^0 \\to D^{-*} \\pi^+ \\pi^+ \\pi^-`
        19. :math:`B^0 \\to D^{-*} \\pi^+ \\pi^+ \\pi^- \\pi^0`
        20. :math:`B^0 \\to D^{+*}_s D^-`
        21. :math:`B^0 \\to D^+_s D^{-*}`
        22. :math:`B^0 \\to D^{+*}_s D^{-*}`
        23. :math:`B^0 \\to J/\\psi K^0_S`
        24. :math:`B^0 \\to J/\\psi K^+ \\pi^-`
        25. :math:`B^0 \\to J/\\psi K^0_S \\pi^+ \\pi^-`
        26. :math:`B^0 \\to \\Lambda^{-}_{c} p \\pi^+ \\pi^-`
        27. :math:`B^0 \\to \\overline{D^0} p \\bar{p}`
        28. :math:`B^0 \\to D^- p \\bar{p} \\pi^+`
        29. :math:`B^0 \\to D^{-*} p \\bar{p} \\pi^+`
        30. :math:`B^0 \\to \\overline{D^0} p \\bar{p} \\pi^+ \\pi^-`
        31. :math:`B^0 \\to \\overline{D^{0*}} p \\bar{p} \\pi^+ \\pi^-`

        From `Thomas Keck's thesis <https://docs.belle2.org/record/275/files/BELLE2-MTHESIS-2015-001.pdf>`_,
        "the channel :math:`B^0 \\to \\overline{D}^0 \\pi^0` was used
        by the FR, but is not yet used in the FEI due to unexpected
        technical restrictions in the KFitter algorithm".

    Cuts applied
       This skim uses the following track and cluster definitions.

       * Cleaned tracks (``pi+:eventShapeForSkims``): :math:`d_0 < 0.5\\,\\text{cm}`, :math:`|z_0| < 2\\,\\text{cm}\\,`,
         and :math:`p_T > 0.1\\,\\text{GeV}`
       * Cleaned ECL clusters (``gamma:eventShapeForSkims``): :math:`0.296706 < \\theta < 2.61799`,
         and :math:`E>0.1\\,\\text{GeV}`

        Event pre-cuts:

        * :math:`R_2 < 0.4` (`foxWolframR2` from
          `modularAnalysis.buildEventShape`, calculated using all cleaned tracks and clusters)
        * :math:`n_{\\text{tracks}} \\geq 4`
        * :math:`n_{\\text{cleaned tracks}} \\geq 3`
        * :math:`n_{\\text{cleaned ECL clusters}} \\geq 3`
        * :math:`\\text{Visible energy of event (CMS frame)}>4\\,\\text{GeV}`
        * :math:`2\\,\\text{GeV}<E_{\\text{cleaned tracks & clusters in ECL}}<7\\,\\text{GeV}`

        Tag side :math:`B` cuts:

        * :math:`M_{\\text{bc}} > 5.24\\,\\text{GeV}`
        * :math:`|\\Delta E| < 0.2\\,\\text{GeV}`
        * :math:`\\text{signal probability} > 0.001` (omitted for decay mode 23)

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        B0HadronicList (list(str)): A Python list containing the string
        :code:`'B0:generic'`, the name of the particle list for
        hadronic :math:`B^0` skim candidates.
    """

    ma.applyCuts(
        'B0:generic',
        '[[Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001] or [extraInfo(decayModeID)==23 and Mbc>5.24 and abs(deltaE)<0.200]]',
        path=path)
    B0HadronicList = ['B0:generic']
    return B0HadronicList


def BplusHadronic(path):
    """
    Note:
        * **Skim description**: Hadronic :math:`B^+` tag FEI skim for
          generic analysis.
        * **Skim LFN code**: 11180200
        * **FEI training**: FEIv4_2020_MC13_release_04_01_01
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

        0. :math:`B^+ \\to \\overline{D^0} \\pi^+`
        1. :math:`B^+ \\to \\overline{D^0} \\pi^+ \\pi^0`
        2. :math:`B^+ \\to \\overline{D^0} \\pi^+ \\pi^0 \\pi^0`
        3. :math:`B^+ \\to \\overline{D^0} \\pi^+ \\pi^+ \\pi^-`
        4. :math:`B^+ \\to \\overline{D^0} \\pi^+ \\pi^+ \\pi^- \\pi^0`
        5. :math:`B^+ \\to \\overline{D^0} D^+`
        6. :math:`B^+ \\to \\overline{D^0} D^+ K^0_S`
        7. :math:`B^+ \\to \\overline{D^{0*}} D^+ K^0_S`
        8. :math:`B^+ \\to \\overline{D^0} D^{+*} K^0_S`
        9. :math:`B^+ \\to \\overline{D^{0*}} D^{+*} K^0_S`
        10. :math:`B^+ \\to \\overline{D^0} D^0 K^+`
        11. :math:`B^+ \\to \\overline{D^{0*}} D^0 K^+`
        12. :math:`B^+ \\to \\overline{D^0} D^{0*} K^+`
        13. :math:`B^+ \\to \\overline{D^{0*}} D^{0*} K^+`
        14. :math:`B^+ \\to D^+_s \\overline{D^0}`
        15. :math:`B^+ \\to \\overline{D^{0*}} \\pi^+`
        16. :math:`B^+ \\to \\overline{D^{0*}} \\pi^+ \\pi^0`
        17. :math:`B^+ \\to \\overline{D^{0*}} \\pi^+ \\pi^0 \\pi^0`
        18. :math:`B^+ \\to \\overline{D^{0*}} \\pi^+ \\pi^+ \\pi^-`
        19. :math:`B^+ \\to \\overline{D^{0*}} \\pi^+ \\pi^+ \\pi^- \\pi^0`
        20. :math:`B^+ \\to D^{+*}_s \\overline{D^0}`
        21. :math:`B^+ \\to D^+_s \\overline{D^{0*}}`
        22. :math:`B^+ \\to \\overline{D^0} K^+`
        23. :math:`B^+ \\to D^- \\pi^+ \\pi^+`
        24. :math:`B^+ \\to D^- \\pi^+ \\pi^+ \\pi^0`
        25. :math:`B^+ \\to J/\\psi K^+`
        26. :math:`B^+ \\to J/\\psi K^+ \\pi^+ \\pi^-`
        27. :math:`B^+ \\to J/\\psi K^+ \\pi^0`
        28. :math:`B^+ \\to J/\\psi K^0_S \\pi^+`
        29. :math:`B^+ \\to \\Lambda^{-}_{c} p \\pi^+ \\pi^0`
        30. :math:`B^+ \\to \\Lambda^{-}_{c} p \\pi^+ \\pi^- \\pi^+`
        31. :math:`B^+ \\to \\overline{D^0} p \\bar{p} \\pi^+`
        32. :math:`B^+ \\to \\overline{D^{0*}} p \\bar{p} \\pi^+`
        33. :math:`B^+ \\to D^+ p \\bar{p} \\pi^+ \\pi^-`
        34. :math:`B^+ \\to D^{+*} p \\bar{p} \\pi^+ \\pi^-`
        35. :math:`B^+ \\to \\Lambda^{-}_{c} p \\pi^+`

    Cuts applied
       This skim uses the following track and cluster definitions.

       * Cleaned tracks (``pi+:eventShapeForSkims``): :math:`d_0 < 0.5\\,\\text{cm}`, :math:`|z_0| < 2\\,\\text{cm}\\,`,
         and :math:`p_T > 0.1\\,\\text{GeV}`
       * Cleaned ECL clusters (``gamma:eventShapeForSkims``): :math:`0.296706 < \\theta < 2.61799`,
         and :math:`E>0.1\\,\\text{GeV}`

        Event pre-cuts:

        * :math:`R_2 < 0.4` (`foxWolframR2` from
          `modularAnalysis.buildEventShape`, calculated using all cleaned tracks and clusters)
        * :math:`n_{\\text{tracks}} \\geq 4`
        * :math:`n_{\\text{cleaned tracks}} \\geq 3`
        * :math:`n_{\\text{cleaned ECL clusters}} \\geq 3`
        * :math:`\\text{Visible energy of event (CMS frame)}>4\\,\\text{GeV}`
        * :math:`2\\,\\text{GeV}<E_{\\text{cleaned tracks & clusters in ECL}}<7\\,\\text{GeV}`

        Tag side :math:`B` cuts:

        * :math:`M_{\\text{bc}} > 5.24\\,\\text{GeV}`
        * :math:`|\\Delta E| < 0.2\\,\\text{GeV}`
        * :math:`\\text{signal probability} > 0.001` (omitted for decay mode 25)

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        BplusHadronicList (list(str)): A Python list containing the string
        :code:`'B+:generic'`, the name of the particle list for
        hadronic :math:`B^+` skim candidates.
    """

    # B+:generic list from FEI must already exist in path
    # Apply cuts
    ma.applyCuts(
        'B+:generic',
        '[[Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001] or [extraInfo(decayModeID)==25 and Mbc>5.24 and abs(deltaE)<0.200]]',
        path=path)

    BplusHadronicList = ['B+:generic']
    return BplusHadronicList


def runFEIforB0Hadronic(path):
    """
    Reconstruct hadronic :math:`B^0` tags using the generically
    trained FEI. Skim pre-cuts are applied before running the FEI.

    Remaining skim cuts are applied by `skim.fei.B0Hadronic`.

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        No return value. Fills particle list :code:`B0:generic` for
        supplied path.

    See also:
        `skim.fei.B0Hadronic` for skim details, FEI training, list of
        reconstructed tag modes, and pre-cuts applied.
    """
    # Pre-selection cuts

    ma.fillParticleList(decayString='pi+:eventShapeForSkims',
                        cut='abs(d0)<0.5 and -2<z0<2 and pt>0.1', path=path)
    ma.fillParticleList(decayString='gamma:eventShapeForSkims',
                        cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)
    vm.addAlias('E_ECL_pi', 'totalECLEnergyOfParticlesInList(pi+:eventShapeForSkims)')
    vm.addAlias('E_ECL_gamma', 'totalECLEnergyOfParticlesInList(gamma:eventShapeForSkims)')
    vm.addAlias('E_ECL', 'formula(E_ECL_pi+E_ECL_gamma)')

    ma.applyEventCuts('nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and pt>0.1)>=3', path=path)
    ma.applyEventCuts('nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.1)>=3', path=path)
    ma.buildEventKinematics(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'], path=path)
    ma.applyEventCuts('visibleEnergyOfEventCMS>4', path=path)
    ma.applyEventCuts('2<E_ECL<7', path=path)

    ma.buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
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

    ma.applyEventCuts('foxWolframR2_maskedNaN<0.4 and nTracks>=4', path=path)
    # Run FEI
    b2.conditions.globaltags = ['analysis_tools_release-04']

    particles = fei.get_default_channels(
        neutralB=True,
        chargedB=False,
        hadronic=True,
        semileptonic=False,
        KLong=False,
        baryonic=True)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2020_MC13_release_04_01_01', training=False, monitor=False)
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
    ma.fillParticleList(decayString='pi+:eventShapeForSkims',
                        cut=' abs(d0)<0.5 and -2<z0<2 and pt>0.1', path=path)
    ma.fillParticleList(decayString='gamma:eventShapeForSkims',
                        cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

    vm.addAlias('E_ECL_pi', 'totalECLEnergyOfParticlesInList(pi+:eventShapeForSkims)')
    vm.addAlias('E_ECL_gamma', 'totalECLEnergyOfParticlesInList(gamma:eventShapeForSkims)')
    vm.addAlias('E_ECL', 'formula(E_ECL_pi+E_ECL_gamma)')

    ma.applyEventCuts('nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and pt>0.1)>=3', path=path)
    ma.applyEventCuts('nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.1)>=3', path=path)
    ma.buildEventKinematics(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'], path=path)
    ma.applyEventCuts('visibleEnergyOfEventCMS>4', path=path)
    ma.applyEventCuts('2<E_ECL<7', path=path)
    ma.buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
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

    ma.applyEventCuts('foxWolframR2_maskedNaN<0.4 and nTracks>=4', path=path)

    # Run FEI
    b2.conditions.globaltags = ['analysis_tools_release-04']

    particles = fei.get_default_channels(
        neutralB=False,
        chargedB=True,
        hadronic=True,
        semileptonic=False,
        KLong=False,
        baryonic=True)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2020_MC13_release_04_01_01', training=False, monitor=False)
    feistate = fei.get_path(particles, configuration)
    path.add_path(feistate.path)


def runFEIforHadronicCombined(path):
    """
    Reconstruct hadronic :math:`B^0` and :math:`B^+` tags using the
    generically trained FEI. Skim pre-cuts are applied before running
    the FEI (the pre-cuts are the same for all FEI skims, and are
    documented in their respective list functions).

    Remaining cuts for skims are applied by `skim.fei.B0Hadronic` and
    `skim.fei.BplusHadronic`.

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders

    Returns:
        No return value. Fills particle lists :code:`B0:generic` and
        :code:`B+:generic` for supplied path.

    See also:
        `skim.fei.B0Hadronic` and `skim.fei.BplusHadronic` for skim
        details, FEI training, lists of reconstructed tag modes, and
        pre-cuts applied.
    """
    # Pre-selection cuts

    ma.applyEventCuts('nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and pt>0.1)>=3', path=path)
    ma.applyEventCuts('nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.1)>=3', path=path)

    ma.fillParticleList(decayString='pi+:eventShapeForSkims',
                        cut=' abs(d0)<0.5 and -2<z0<2 and pt>0.1', path=path)
    ma.fillParticleList(decayString='gamma:eventShapeForSkims',
                        cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

    vm.addAlias('E_ECL_pi', 'totalECLEnergyOfParticlesInList(pi+:eventShapeForSkims)')
    vm.addAlias('E_ECL_gamma', 'totalECLEnergyOfParticlesInList(gamma:eventShapeForSkims)')
    vm.addAlias('E_ECL', 'formula(E_ECL_pi+E_ECL_gamma)')

    ma.buildEventKinematics(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'], path=path)
    ma.applyEventCuts('visibleEnergyOfEventCMS>4', path=path)
    ma.applyEventCuts('2<E_ECL<7', path=path)
    ma.buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
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

    ma.applyEventCuts('foxWolframR2_maskedNaN<0.4 and nTracks>=4', path=path)

    # Run FEI
    b2.conditions.globaltags = ['analysis_tools_release-04']

    particles = fei.get_default_channels(
        neutralB=True,
        chargedB=True,
        hadronic=True,
        semileptonic=False,
        KLong=False,
        baryonic=True)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2020_MC13_release_04_01_01', training=False, monitor=False)
    feistate = fei.get_path(particles, configuration)
    path.add_path(feistate.path)


def B0SL(path):
    """
    Note:
        * **Skim description**: Semileptonic :math:`B^0` tag FEI skim
          for generic analysis.
        * **Skim LFN code**: 11180300
        * **FEI training**: FEIv4_2020_MC13_release_04_01_01
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

        0. :math:`B^0 \\to D^- e^+`
        1. :math:`B^0 \\to D^- \\mu^+`
        2. :math:`B^0 \\to D^{-*} e^+`
        3. :math:`B^0 \\to D^{-*} \\mu^+`
        4. :math:`B^0 \\to \\overline{D^0} \\pi^- e^+`
        5. :math:`B^0 \\to \\overline{D^0} \\pi^- \\mu^+`
        6. :math:`B^0 \\to \\overline{D^{0*}} \\pi^- e^+`
        7. :math:`B^0 \\to \\overline{D^{0*}} \\pi^- \\mu^+`

    Cuts applied
       This skim uses the following track and cluster definitions.

       * Cleaned tracks (``pi+:eventShapeForSkims``): :math:`d_0 < 0.5\\,\\text{cm}`, :math:`|z_0| < 2\\,\\text{cm}\\,`,
         and :math:`p_T > 0.1\\,\\text{GeV}`
       * Cleaned ECL clusters (``gamma:eventShapeForSkims``): :math:`0.296706 < \\theta < 2.61799`,
         and :math:`E>0.1\\,\\text{GeV}`

        Event pre-cuts:

        * :math:`R_2 < 0.4` (`foxWolframR2` from
          `modularAnalysis.buildEventShape`, calculated using all cleaned tracks and clusters)
        * :math:`n_{\\text{tracks}} \\geq 4`
        * :math:`n_{\\text{cleaned tracks}} \\geq 3`
        * :math:`n_{\\text{cleaned ECL clusters}} \\geq 3`
        * :math:`\\text{Visible energy of event (CMS frame)}>4\\,\\text{GeV}`
        * :math:`2\\,\\text{GeV}<E_{\\text{cleaned tracks & clusters in ECL}}<7\\,\\text{GeV}`

        Tag side :math:`B` cuts:

        * :math:`-4 < \\cos\\theta_{BY} < 3`
        * :math:`\\text{Decay mode ID} < 8` (no SL :math:`D` channels)
        * :math:`\\log_{10}(\\text{signal probability}) > -2.4`
        * :math:`p_{\\ell}^{*} > 1.0\\,\\text{GeV}` in CMS frame
          (``daughter(1, p) > 1.0`` or ``daughter(2, p) > 1.0``, depending
          on decay mode ID)

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        B0SLList (list(str)): A Python list containing the string
        :code:`'B0:semileptonic'`, the name of the particle list for
        SL :math:`B^0` skim candidates.
    """
    vm.addAlias('p_lepton_CMSframe', 'conditionalVariableSelector(dmID<4, d1_p_CMSframe, d2_p_CMSframe)')

    # Apply cuts
    ma.applyCuts('B0:semileptonic', 'dmID<8', path=path)
    ma.applyCuts('B0:semileptonic', 'log10_sigProb>-2.4', path=path)
    ma.applyCuts('B0:semileptonic', '-4.0<cosThetaBY<3.0', path=path)
    ma.applyCuts('B0:semileptonic', 'p_lepton_CMSframe>1.0', path=path)

    B0SLList = ['B0:semileptonic']
    return B0SLList


def BplusSL(path):
    """
    Note:
        * **Skim description**: Semileptonic :math:`B^+` tag FEI skim
          for generic analysis.
        * **Skim LFN code**: 11180400
        * **FEI training**: FEIv4_2020_MC13_release_04_01_01
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

        0. :math:`B^0 \\to \\overline{D^0} e^+`
        1. :math:`B^0 \\to \\overline{D^0} \\mu^+`
        2. :math:`B^0 \\to \\overline{D^{0*}} e^+`
        3. :math:`B^0 \\to \\overline{D^{0*}} \\mu^+`
        4. :math:`B^0 \\to D^- \\pi^+ e^+`
        5. :math:`B^0 \\to D^- \\pi^+ \\mu^+`
        6. :math:`B^0 \\to D^{-*} \\pi^+ e^+`
        7. :math:`B^0 \\to D^{-*} \\pi^+ \\mu^+`

    Cuts applied
       This skim uses the following track and cluster definitions.

       * Cleaned tracks (``pi+:eventShapeForSkims``): :math:`d_0 < 0.5\\,\\text{cm}`, :math:`|z_0| < 2\\,\\text{cm}\\,`,
         and :math:`p_T > 0.1\\,\\text{GeV}`
       * Cleaned ECL clusters (``gamma:eventShapeForSkims``): :math:`0.296706 < \\theta < 2.61799`,
         and :math:`E>0.1\\,\\text{GeV}`

        Event pre-cuts:

        * :math:`R_2 < 0.4` (`foxWolframR2` from
          `modularAnalysis.buildEventShape`, calculated using all cleaned tracks and clusters)
        * :math:`n_{\\text{tracks}} \\geq 4`
        * :math:`n_{\\text{cleaned tracks}} \\geq 3`
        * :math:`n_{\\text{cleaned ECL clusters}} \\geq 3`
        * :math:`\\text{Visible energy of event (CMS frame)}>4\\,\\text{GeV}`
        * :math:`2\\,\\text{GeV}<E_{\\text{cleaned tracks & clusters in ECL}}<7\\,\\text{GeV}`


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
        BplusSLList (list(str)): A Python list containing the string
        :code:`'B+:semileptonic'`, the name of the particle list for
        SL :math:`B^+` skim candidates.
    """
    vm.addAlias('p_lepton_CMSframe', 'conditionalVariableSelector(dmID<4, d1_p_CMSframe, d2_p_CMSframe)')

    # Apply cuts
    ma.applyCuts('B+:semileptonic', 'dmID<8', path=path)
    ma.applyCuts('B+:semileptonic', 'log10_sigProb>-2.4', path=path)
    ma.applyCuts('B+:semileptonic', '-4.0<cosThetaBY<3.0', path=path)
    ma.applyCuts('B+:semileptonic', 'p_lepton_CMSframe>1.0', path=path)

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

    ma.applyEventCuts('nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and pt>0.1)>=3', path=path)
    ma.applyEventCuts('nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.1)>=3', path=path)
    ma.fillParticleList(decayString='pi+:eventShapeForSkims',
                        cut='pt > 0.1 and abs(d0)<0.5 and -2<z0<2', path=path)
    ma.fillParticleList(decayString='gamma:eventShapeForSkims',
                        cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

    vm.addAlias('E_ECL_pi', 'totalECLEnergyOfParticlesInList(pi+:eventShapeForSkims)')
    vm.addAlias('E_ECL_gamma', 'totalECLEnergyOfParticlesInList(gamma:eventShapeForSkims)')
    vm.addAlias('E_ECL', 'formula(E_ECL_pi+E_ECL_gamma)')
    ma.buildEventKinematics(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'], path=path)
    ma.applyEventCuts('visibleEnergyOfEventCMS>4', path=path)
    ma.applyEventCuts('2<E_ECL<7', path=path)
    ma.buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
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

    ma.applyEventCuts('foxWolframR2_maskedNaN<0.4 and nTracks>=4', path=path)

    # Run FEI
    b2.conditions.globaltags = ['analysis_tools_release-04']

    particles = fei.get_default_channels(
        neutralB=True,
        chargedB=False,
        hadronic=False,
        semileptonic=True,
        KLong=False,
        baryonic=True,
        removeSLD=True)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2020_MC13_release_04_01_01', training=False, monitor=False)
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
    ma.fillParticleList(decayString='pi+:eventShapeForSkims',
                        cut='pt > 0.1 and abs(d0)<0.5 and -2<z0<2', path=path)
    ma.fillParticleList(decayString='gamma:eventShapeForSkims',
                        cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

    ma.applyEventCuts('nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and pt>0.1)>=3', path=path)
    ma.applyEventCuts('nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.1)>=3', path=path)

    vm.addAlias('E_ECL_pi', 'totalECLEnergyOfParticlesInList(pi+:eventShapeForSkims)')
    vm.addAlias('E_ECL_gamma', 'totalECLEnergyOfParticlesInList(gamma:eventShapeForSkims)')
    vm.addAlias('E_ECL', 'formula(E_ECL_pi+E_ECL_gamma)')
    ma.buildEventKinematics(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'], path=path)
    ma.applyEventCuts('visibleEnergyOfEventCMS>4', path=path)
    ma.applyEventCuts('2<E_ECL<7', path=path)
    ma.buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
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

    ma.applyEventCuts('foxWolframR2_maskedNaN<0.4 and nTracks>=4', path=path)

    # Run FEI
    b2.conditions.globaltags = ['analysis_tools_release-04']

    particles = fei.get_default_channels(
        neutralB=False,
        chargedB=True,
        hadronic=False,
        semileptonic=True,
        KLong=False,
        baryonic=True,
        removeSLD=True)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2020_MC13_release_04_01_01', training=False, monitor=False)
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
    ma.fillParticleList(decayString='pi+:eventShapeForSkims',
                        cut=' abs(d0)<0.5 and -2<z0<2 and pt>0.1', path=path)
    ma.fillParticleList(decayString='gamma:eventShapeForSkims',
                        cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

    ma.applyEventCuts('nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and pt>0.1)>=3', path=path)
    ma.applyEventCuts('nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.1)>=3', path=path)

    vm.addAlias('E_ECL_pi', 'totalECLEnergyOfParticlesInList(pi+:eventShapeForSkims)')
    vm.addAlias('E_ECL_gamma', 'totalECLEnergyOfParticlesInList(gamma:eventShapeForSkims)')
    vm.addAlias('E_ECL', 'formula(E_ECL_pi+E_ECL_gamma)')

    ma.buildEventKinematics(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'], path=path)
    ma.applyEventCuts('visibleEnergyOfEventCMS>4', path=path)
    ma.applyEventCuts('2<E_ECL<7', path=path)
    ma.buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
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

    ma.applyEventCuts('foxWolframR2_maskedNaN<0.4 and nTracks>=4', path=path)

    # Run FEI
    b2.conditions.globaltags = ['analysis_tools_release-04']

    particles = fei.get_default_channels(
        neutralB=True,
        chargedB=True,
        hadronic=False,
        semileptonic=True,
        KLong=False,
        baryonic=True,
        removeSLD=True)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2020_MC13_release_04_01_01', training=False, monitor=False)
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

    Remaining cuts for skims are applied by `skim.fei.B0Hadronic`,
    `skim.fei.BplusHadronic`, `skim.fei.B0SL`, and `skim.fei.BplusSL`.

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders

    Returns:
        No return value. Fills particle lists :code:`B0:generic`,
        :code:`B+:generic`, :code:`B+:semileptonic`, and
        :code:`B0:semileptonic` for supplied path.

    See also:
        `skim.fei.B0Hadronic`, `skim.fei.BplusHadronic`,
        `skim.fei.B0SL`, and `skim.fei.BplusSL` for skim details, FEI
        training, lists of reconstructed tag modes, and pre-cuts
        applied.
    """
    # Pre-selection cuts
    ma.fillParticleList(decayString='pi+:eventShapeForSkims',
                        cut='abs(d0)<0.5 and -2<z0<2 and pt>0.1', path=path)
    ma.fillParticleList(decayString='gamma:eventShapeForSkims',
                        cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

    ma.applyEventCuts('nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and pt>0.1)>=3', path=path)
    ma.applyEventCuts('nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.1)>=3', path=path)

    vm.addAlias('E_ECL_pi', 'totalECLEnergyOfParticlesInList(pi+:eventShapeForSkims)')
    vm.addAlias('E_ECL_gamma', 'totalECLEnergyOfParticlesInList(gamma:eventShapeForSkims)')
    vm.addAlias('E_ECL', 'formula(E_ECL_pi+E_ECL_gamma)')

    ma.buildEventKinematics(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'], path=path)
    ma.applyEventCuts('visibleEnergyOfEventCMS>4', path=path)
    ma.applyEventCuts('2<E_ECL<7', path=path)

    ma.buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
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

    ma.applyEventCuts('foxWolframR2_maskedNaN<0.4 and nTracks>=4', path=path)
    # Run FEI
    b2.conditions.globaltags = ['analysis_tools_release-04']

    particles = fei.get_default_channels(
        neutralB=True,
        chargedB=True,
        hadronic=True,
        semileptonic=True,
        KLong=False,
        baryonic=True,
        removeSLD=True)
    configuration = fei.config.FeiConfiguration(prefix='FEIv4_2020_MC13_release_04_01_01', training=False, monitor=False)
    feistate = fei.get_path(particles, configuration)
    path.add_path(feistate.path)
