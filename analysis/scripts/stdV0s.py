#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdPr


def stdKshorts(prioritiseV0=True, path=None):
    """
    Load a combined list of the Kshorts list from V0 objects merged with
    a list of particles combined using the analysis ParticleCombiner module.

    The ParticleList is named 'K_S0:merged'. A vertex fit is performed and only
    candidates with an invariant mass in the range :math:`0.450 < M < 0.550~GeV`,
    and for which the vertex fit did not fail, are kept.

    Parameters:
        prioritiseV0 (bool): should the V0 mdst objects be prioritised when merging?
        path (basf2.Path): the path to load the modules
    """
    # Fill one list from V0
    fillParticleList('K_S0:V0 -> pi+ pi-', '0.3 < M < 0.7', True, path=path)
    # Perform vertex fit and apply tighter mass window
    vertexRave('K_S0:V0', conf_level=0.0, path=path, silence_warning=True)
    applyCuts('K_S0:V0', '0.450 < M < 0.550', path=path)
    # Reconstruct a second list
    stdPi('all', path=path)  # no quality cuts
    reconstructDecay('K_S0:RD -> pi+:all pi-:all', '0.3 < M < 0.7', 1, True, path=path)
    # Again perform vertex fit and apply tighter mass window
    vertexRave('K_S0:RD', conf_level=0.0, path=path, silence_warning=True)
    applyCuts('K_S0:RD', '0.450 < M < 0.550', path=path)
    # Create merged list based on provided priority
    if prioritiseV0:
        copyLists('K_S0:merged', ['K_S0:V0', 'K_S0:RD'], False, path=path)
    else:
        copyLists('K_S0:merged', ['K_S0:RD', 'K_S0:V0'], False, path=path)


def mergedKshorts(prioritiseV0=True, path=None):
    """
    Load a combined list of the :func:`stdKshorts` list from V0 objects merged with
    a list of particles combined using the analysis ParticleCombiner module.

    WARNING:
      The mergedKshorts() list is now deprecated. Use instead the stdKshorts() list.
      See that docstring for more information. For instance with

    ::

       from stdV0s import stdKshorts
       help(stdKshorts)
    """
    B2WARNING("The mergedKshorts() list is now deprecated. Please use instead stdKshorts() list.")
    stdKshorts(prioritiseV0, path)


def goodBelleKshort(path):
    """
    Load the Belle goodKshort list. Creates a ParticleList named
    'K_S0:legacyGoodKS'. A vertex fit is performed: and only candidates that
    satisfy the 'goodBelleKshort' criteria, with an invariant mass in the range
    :math:`0.468 < M < 0.528~GeV`, and for which the vertex fit did not fail, are kept

    Parameters:
        path (basf2.Path): the path to load the modules
    """
    fillParticleList('K_S0:legacyGoodKS -> pi+ pi-', '0.3 < M < 0.7', True, path=path)
    vertexRave('K_S0:legacyGoodKS', conf_level=0.0, path=path, silence_warning=True)
    applyCuts('K_S0:legacyGoodKS', '0.468 < M < 0.528 and goodBelleKshort==1', path=path)


def stdLambdas(prioritiseV0=True, path=None):
    """
    Load a combined list of the Lambda list from V0 objects merged with
    a list of particles combined using the analysis ParticleCombiner module.

    The ParticleList is named 'Lambda0:merged'. A vertex fit is performed and only
    candidates with an invariant mass in the range :math:`1.10 < M < 1.13~GeV`,
    and for which the vertex fit did not fail, are kept.

    Parameters:
        prioritiseV0 (bool): should the V0 mdst objects be prioritised when merging?
        path (basf2.Path): the path to load the modules
    """
    # Fill one list from V0
    fillParticleList('Lambda0:V0 -> p+ pi-', '0.9 < M < 1.3', True, path=path)
    # Perform vertex fit and apply tighter mass window
    vertexRave('Lambda0:V0', conf_level=0.0, path=path, silence_warning=True)
    applyCuts('Lambda0:V0', '1.10 < M < 1.13', path=path)
    # Find V0 duplicate with better vertex fit quality
    markDuplicate('Lambda0:V0', False, path=path)
    applyCuts('Lambda0:V0', 'extraInfo(highQualityVertex)', path=path)
    # Reconstruct a second list
    stdPi('all', path=path)  # no quality cuts
    stdPr('all', path=path)  # no quality cuts
    reconstructDecay('Lambda0:RD -> p+:all pi-:all', '0.9 < M < 1.3', 1, True, path=path)
    # Again perform vertex fit and apply tighter mass window
    vertexRave('Lambda0:RD', conf_level=0.0, path=path, silence_warning=True)
    applyCuts('Lambda0:RD', '1.10 < M < 1.13', path=path)
    # Find RD duplicate with better vertex fit quality
    markDuplicate('Lambda0:RD', False, path=path)
    applyCuts('Lambda0:RD', 'extraInfo(highQualityVertex)', path=path)
    if prioritiseV0:
        copyLists('Lambda0:merged', ['Lambda0:V0', 'Lambda0:RD'], False, path=path)
    else:
        copyLists('Lambda0:merged', ['Lambda0:RD', 'Lambda0:V0'], False, path=path)


def mergedLambdas(prioritiseV0=True, path=None):
    """
    Load a combined list of the Lambdas list from V0 objects merged with
    a list of particles combined using the analysis ParticleCombiner module.

    WARNING:
      The mergedLambdas() list is now deprecated. Use instead the stdLambdas() list.
      See that docstring for more information. For instance with

    ::

        from stdV0s import stdLambdas
        help(stdLambdas)
    """
    B2WARNING("The mergedLambdas() list is now deprecated. Please use instead stdLambdas() list.")
    stdLambdas(prioritiseV0, path)
