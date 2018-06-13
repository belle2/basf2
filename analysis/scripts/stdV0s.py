#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi

# Prepare Kshorts from V0 mdst table
# the ParticleList is named "K_S0:all"
# vertex fit is performed and only
# candidates with inv. mass between
# 0.450 - 0.550 GeV are kept

# Also prepare merged Kshort list "K_S0:merged"
# by combining the above with a hand-built one

# Belle goodKshort list "'K_S0:legacyGoodKS" has been prepared. Vertex fit is performed and candidates
# with inv. mass between 0.468 - 0.528 Gev are kept
# and those satisfying "goodBelleKshort" criteria are taken further.


def stdKshorts(path=analysis_main):
    fillParticleList('K_S0:all', '0.3 < M < 0.7', True, path=path)
    vertexKFit('K_S0:all', 0.0, '', '', path)
    applyCuts('K_S0:all', '0.450 < M < 0.550', path)


def mergedKshorts(prioritiseV0=True, path=analysis_main):
    fillParticleList('K_S0:V0', '0.3 < M < 0.7', True, path=path)
    stdPi('all')
    # applyCuts('pi+:all', 'chiProb>0.001') #deprecated
    # prescription from tracking soon?
    reconstructDecay('K_S0:RD -> pi-:all pi+:all', '0.3 < M < 0.7', 1, True, path)
    V0ListMerger('K_S0:V0', 'K_S0:RD', prioritiseV0)  # outputs K_S0:merged
    vertexKFit('K_S0:merged', 0.0, '', '', path)
    applyCuts('K_S0:merged', '0.450 < M < 0.550', path)


def goodBelleKshort(path=analysis_main):
    fillParticleList('K_S0:legacyGoodKS', '0.3 < M < 0.7', True, path=path)
    vertexKFit('K_S0:legacyGoodKS', 0.0, '', '', path)
    applyCuts('K_S0:legacyGoodKS', '0.468 < M < 0.528 and goodBelleKshort==1', path)


def loadStdKS(path=analysis_main):
    stdKshorts(path)
