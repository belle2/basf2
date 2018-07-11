#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi


def stdKshorts(path=analysis_main):
    """
    Load :math:`K_{S}^{0}`s from the mdst V0 objects (created when the
    tracking was run). The ParticleList is named "K_S0:all". A vertex fit is
    performed and only candidates with an invariant mass in the range
    0.450 < M < 0.550 GeV, and for which the vertex fit did not fail, are kept

    Parameters:
        path (basf2.Path) the path to load the modules
    """
    fillParticleList('K_S0:all', '0.3 < M < 0.7', True, path=path)
    vertexKFit('K_S0:all', 0.0, '', '', path)
    applyCuts('K_S0:all', '0.450 < M < 0.550', path)


def mergedKshorts(prioritiseV0=True, path=analysis_main):
    """
    Load a combined list of the ``stdKshorts`` list from V0 objects merged with
    a list of particles combined using the analysis ParticleCombiner module.

    The same selection requirements are made as for ``stdKshorts``. See that
    docstring for more information. For instance with

        from stdV0s import stdKshorts
        help(stdKshorts)

    Parameters:
        prioritiseV0 (bool) should the V0 mdst objects be prioritised when merging?
        path (basf2.Path) the path to load the modules
    """
    fillParticleList('K_S0:V0', '0.3 < M < 0.7', True, path=path)
    stdPi('all')
    # applyCuts('pi+:all', 'chiProb>0.001') #deprecated
    # prescription from tracking soon?
    reconstructDecay('K_S0:RD -> pi-:all pi+:all', '0.3 < M < 0.7', 1, True, path)
    V0ListMerger('K_S0:V0', 'K_S0:RD', prioritiseV0)  # outputs K_S0:merged
    vertexKFit('K_S0:merged', 0.0, '', '', path)
    applyCuts('K_S0:merged', '0.450 < M < 0.550', path)


def goodBelleKshort(path=analysis_main):
    """
    Warning:
        Only used for tests of backward compatibility. Do not use for
        Belle II analyses.

    Load the Belle goodKshort list. Creates a ParticleList named
    "K_S0:legacyGoodKS". A vertex fit is performed: and only candidates that
    satisfy the 'goodBelleKshort' criteria, with an invariant mass in the range
    0.468 < M < 0.528 GeV, and for which the vertex fit did not fail, are kept

    Parameters:
        path (basf2.Path) the path to load the modules
    """
    fillParticleList('K_S0:legacyGoodKS', '0.3 < M < 0.7', True, path=path)
    vertexKFit('K_S0:legacyGoodKS', 0.0, '', '', path)
    applyCuts('K_S0:legacyGoodKS', '0.468 < M < 0.528 and goodBelleKshort==1', path)


def loadStdKS(path=analysis_main):
    """
    Warning:
        This function is deprecated. Please use ``stdKshorts`` directly

    Loads the standard :math:`K_{S}^{0}` list.
    Kept for backward compatibility.

    Parameters:
        path (basf2.Path) modules are added to this path

    """
    stdKshorts(path)
