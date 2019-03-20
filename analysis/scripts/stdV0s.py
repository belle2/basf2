#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdPr


def stdKshorts(path=analysis_main):
    """
    Load :math:`K_{S}^{0}` s from the mdst V0 objects (created when the
    tracking was run). The ParticleList is named 'K_S0:all'. A vertex fit is
    performed and only candidates with an invariant mass in the range
    :math:`0.450 < M < 0.550~GeV`, and for which the vertex fit did not fail, are kept

    Note:
      when filling a particleList, for V0s the whole decay chain has to be
      specified (the two daughters as well)

    Parameters:
        path (basf2.Path): the path to load the modules
    """
    fillParticleList('K_S0:all -> pi+ pi-', '0.3 < M < 0.7', True, path=path)
    vertexKFit('K_S0:all', conf_level=0.0, path=path, silence_warning=True)
    applyCuts('K_S0:all', '0.450 < M < 0.550', path=path)


def mergedKshorts(prioritiseV0=True, path=analysis_main):
    """
    Load a combined list of the :func:`stdKshorts` list from V0 objects merged with
    a list of particles combined using the analysis ParticleCombiner module.

    The same selection requirements are made as for :func:`stdKshorts`. See that
    docstring for more information. For instance with

    ::

        from stdV0s import stdKshorts
        help(stdKshorts)

    Parameters:
        prioritiseV0 (bool): should the V0 mdst objects be prioritised when merging?
        path (basf2.Path): the path to load the modules
    """
    # Fill one list from V0
    fillParticleList('K_S0:V0 -> pi+ pi-', '0.3 < M < 0.7', True, path=path)
    # Reconstruct a second list
    stdPi('all', path=path)  # no quality cuts
    reconstructDecay('K_S0:RD -> pi+:all pi-:all', '0.3 < M < 0.7', 1, True, path=path)
    # Create merged list, vertex it and run duplicate marker
    copyLists('K_S0:merged', ['K_S0:V0', 'K_S0:RD'], False, path=path)
    vertexKFit('K_S0:merged', conf_level=0.0, path=path, silence_warning=True)
    markDuplicate('K_S0:merged', prioritiseV0, path=path)
    # Select good duplicates with tighter mass window
    applyCuts('K_S0:merged', 'extraInfo(highQualityVertex) and 0.450 < M < 0.550', path=path)


def goodBelleKshort(path=analysis_main):
    """
    Load the Belle goodKshort list. Creates a ParticleList named
    'K_S0:legacyGoodKS'. A vertex fit is performed: and only candidates that
    satisfy the 'goodBelleKshort' criteria, with an invariant mass in the range
    :math:`0.468 < M < 0.528~GeV`, and for which the vertex fit did not fail, are kept

    Parameters:
        path (basf2.Path): the path to load the modules
    """
    fillParticleList('K_S0:legacyGoodKS -> pi+ pi-', '0.3 < M < 0.7', True, path=path)
    vertexKFit('K_S0:legacyGoodKS', conf_level=0.0, path=path, silence_warning=True)
    applyCuts('K_S0:legacyGoodKS', '0.468 < M < 0.528 and goodBelleKshort==1', path=path)


def stdLambdas(path=analysis_main):
    """
    Load :math:`\Lambda^{0}`s from the mdst V0 objects (created when the
    tracking was run). The ParticleList is named "Lambda0:all". A vertex fit is
    performed and only candidates with an invariant mass in the range
    :math:`1.10 < M < 1.13~GeV`, and for which the vertex fit did not fail, are
    kept.

    Note:
      when filling a particleList, for V0s the whole decay chain has to be
      specified (the two daughters as well)

    Parameters:
        path (basf2.Path): the path to load the modules
    """
    fillParticleList('Lambda0:all -> p+ pi-', '0.9 < M < 1.3', True, path=path)
    vertexKFit('Lambda0:all', conf_level=0.0, path=path, silence_warning=True)
    applyCuts('Lambda0:all', '1.10 < M < 1.13', path=path)


def mergedLambdas(prioritiseV0=True, path=analysis_main):
    """
    Load a combined list of the :func:`stdLambdas` list from V0 objects merged with
    a list of particles combined using the analysis ParticleCombiner module.

    The same selection requirements are made as for :func:`stdLambdas`. See that
    docstring for more information. For instance with

    ::

        from stdV0s import stdLambdas
        help(stdLambdas)

    Parameters:
        prioritiseV0 (bool): should the V0 mdst objects be prioritised when merging?
        path (basf2.Path): the path to load the modules
    """
    # Fill one list from V0
    fillParticleList('Lambda0:V0 -> p+ pi-', '0.9 < M < 1.3', True, path=path)
    # Reconstruct a second list
    stdPi('all', path=path)  # no quality cuts
    stdPr('all', path=path)  # no quality cuts
    reconstructDecay('Lambda0:RD -> p+:all pi-:all', '0.9 < M < 1.3', 1, True, path=path)
    # Create merged list, vertex it and run duplicate marker
    copyLists('Lambda0:merged', ['Lambda0:V0', 'Lambda0:RD'], False, path=path)
    vertexKFit('Lambda0:merged', conf_level=0.0, path=path, silence_warning=True)
    markDuplicate('Lambda0:merged', prioritiseV0, path=path)
    # Select good duplicates with tighter mass window
    applyCuts('Lambda0:merged', 'extraInfo(highQualityVertex) and 1.10 < M < 1.13', path=path)
