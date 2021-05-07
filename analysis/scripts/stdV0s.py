#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import B2ERROR
import modularAnalysis as ma
from stdCharged import stdPi, stdPr
import vertex


def stdKshorts(prioritiseV0=True, fitter='TreeFit', path=None):
    """
    Load a combined list of the Kshorts list from V0 objects merged with
    a list of particles combined using the analysis ParticleCombiner module.

    The ParticleList is named ``K_S0:merged``. A vertex fit is performed and only
    candidates with an invariant mass in the range :math:`0.450 < M < 0.550~GeV`,
    and for which the vertex fit did not fail, are kept.

    The vertex fitter can be selected among ``TreeFit``, ``KFit``, and ``Rave``.

    Parameters:
        prioritiseV0 (bool): should the V0 mdst objects be prioritised when merging?
        fitter (str): vertex fitter name, valid options are ``TreeFit``, ``KFit``, and ``Rave``.
        path (basf2.Path): the path to load the modules
    """
    # Fill one list from V0
    ma.fillParticleList('K_S0:V0 -> pi+ pi-', '', True, path=path)
    ma.cutAndCopyList('K_S0:V0_MassWindow', 'K_S0:V0', '0.3 < M < 0.7', path=path)
    # Perform vertex fit and apply tighter mass window
    if fitter == 'TreeFit':
        vertex.treeFit('K_S0:V0_MassWindow', conf_level=0.0, path=path)
    elif fitter == 'KFit':
        vertex.kFit('K_S0:V0_MassWindow', conf_level=0.0, path=path)
    elif fitter == 'Rave':
        vertex.raveFit('K_S0:V0_MassWindow', conf_level=0.0, path=path, silence_warning=True)
    else:
        B2ERROR("Valid fitter options for Kshorts are 'TreeFit', 'KFit', and 'Rave'. However, the latter is not recommended.")
    ma.applyCuts('K_S0:V0_MassWindow', '0.450 < M < 0.550', path=path)
    # Reconstruct a second list
    stdPi('all', path=path)  # no quality cuts
    ma.reconstructDecay('K_S0:RD -> pi+:all pi-:all', '0.3 < M < 0.7', 1, True, path=path)
    # Again perform vertex fit and apply tighter mass window
    if fitter == 'TreeFit':
        vertex.treeFit('K_S0:RD', conf_level=0.0, path=path)
    elif fitter == 'KFit':
        vertex.kFit('K_S0:RD', conf_level=0.0, path=path)
    elif fitter == 'Rave':
        vertex.raveFit('K_S0:RD', conf_level=0.0, path=path, silence_warning=True)
    ma.applyCuts('K_S0:RD', '0.450 < M < 0.550', path=path)
    # Create merged list based on provided priority
    ma.mergeListsWithBestDuplicate('K_S0:merged', ['K_S0:V0_MassWindow', 'K_S0:RD'],
                                   variable='particleSource', preferLowest=prioritiseV0, path=path)


def goodBelleKshort(path):
    """
    Load the Belle goodKshort list. Creates a ParticleList named
    ``K_S0:legacyGoodKS``. A vertex fit is performed and only candidates that
    satisfy the :b2:var:`goodBelleKshort` criteria, with an invariant mass in the range
    :math:`0.468 < M < 0.528~GeV`, and for which the vertex fit did not fail, are kept

    Parameters:
        path (basf2.Path): the path to load the modules
    """
    ma.fillParticleList('K_S0:legacyGoodKS -> pi+ pi-', '0.3 < M < 0.7', True, path=path)
    vertex.kFit('K_S0:legacyGoodKS', conf_level=0.0, path=path)
    ma.applyCuts('K_S0:legacyGoodKS', '0.468 < M < 0.528 and goodBelleKshort==1', path=path)


def stdLambdas(prioritiseV0=True, fitter='TreeFit', path=None):
    """
    Load a combined list of the Lambda list from V0 objects merged with
    a list of particles combined using the analysis ParticleCombiner module.

    The ParticleList is named ``Lambda0:merged``. A vertex fit is performed and only
    candidates with an invariant mass in the range :math:`1.10 < M < 1.13~GeV`,
    and for which the vertex fit did not fail, are kept.

    The vertex fitter can be selected among ``TreeFit``, ``KFit``, and ``Rave``.

    Parameters:
        prioritiseV0 (bool): should the V0 mdst objects be prioritised when merging?
        fitter (str): vertex fitter name, valid options are ``TreeFit``, ``KFit``, and ``Rave``.
        path (basf2.Path): the path to load the modules
    """
    # Fill one list from V0
    ma.fillParticleList('Lambda0:V0 -> p+ pi-', '', True, path=path)
    ma.cutAndCopyList('Lambda0:V0_MassWindow', 'Lambda0:V0', '0.9 < M < 1.3', path=path)
    # Perform vertex fit and apply tighter mass window
    if fitter == 'TreeFit':
        vertex.treeFit('Lambda0:V0_MassWindow', conf_level=0.0, path=path)
    elif fitter == 'KFit':
        vertex.kFit('Lambda0:V0_MassWindow', conf_level=0.0, path=path)
    elif fitter == 'Rave':
        vertex.raveFit('Lambda0:V0_MassWindow', conf_level=0.0, path=path, silence_warning=True)
    else:
        B2ERROR("Valid fitter options for Lambdas are 'TreeFit', 'KFit', and 'Rave'. However, the latter is not recommended.")
    ma.applyCuts('Lambda0:V0_MassWindow', '1.10 < M < 1.13', path=path)
    # Find V0 duplicate with better vertex fit quality
    ma.markDuplicate('Lambda0:V0_MassWindow', False, path=path)
    ma.applyCuts('Lambda0:V0_MassWindow', 'extraInfo(highQualityVertex)', path=path)
    # Reconstruct a second list
    stdPi('all', path=path)  # no quality cuts
    stdPr('all', path=path)  # no quality cuts
    ma.reconstructDecay('Lambda0:RD -> p+:all pi-:all', '0.9 < M < 1.3', 1, True, path=path)
    # Again perform vertex fit and apply tighter mass window
    if fitter == 'TreeFit':
        vertex.treeFit('Lambda0:RD', conf_level=0.0, path=path)
    elif fitter == 'KFit':
        vertex.kFit('Lambda0:RD', conf_level=0.0, path=path)
    elif fitter == 'Rave':
        vertex.raveFit('Lambda0:RD', conf_level=0.0, path=path, silence_warning=True)
    ma.applyCuts('Lambda0:RD', '1.10 < M < 1.13', path=path)
    # Find RD duplicate with better vertex fit quality
    ma.markDuplicate('Lambda0:RD', False, path=path)
    ma.applyCuts('Lambda0:RD', 'extraInfo(highQualityVertex)', path=path)
    ma.mergeListsWithBestDuplicate('Lambda0:merged', ['Lambda0:V0_MassWindow', 'Lambda0:RD'],
                                   variable='particleSource', preferLowest=prioritiseV0, path=path)
