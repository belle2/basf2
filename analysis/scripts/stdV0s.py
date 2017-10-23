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


def markDuplicate(
    particleList,
    prioritiseV0,
    #    extraInfoName
    path=analysis_main
):
    markdup = register_module('DuplicateVertexMarker')
    markdup.param('particleList', particleList)
    markdup.param('prioritiseV0', prioritiseV0)
    path.add_module(markdup)


def stdKshorts(path=analysis_main):
    fillParticleList('K_S0:all', '0.3 < M < 0.7', True, path)
    vertexKFit('K_S0:all', 0.0, '', '', path)
    applyCuts('K_S0:all', '0.450 < M < 0.550', path)


def combinedKshorts(prioritiseV0=False, path=analysis_main):
    fillParticleList('K_S0:V0', '0.3 < M < 0.7', True, path)
    stdPi('all')
    applyCuts('pi+:all', 'chiProb>0.001')
    reconstructDecay('K_S0:RD -> pi-:all pi+:all', '0.3 < M < 0.7', 1, True, path)
    copyLists('K_S0:combined', ['K_S0:V0', 'K_S0:RD'], False, path)
    vertexKFit('K_S0:combined', 0.0, '', '', path)
    applyCuts('K_S0:combined', '0.450 < M < 0.550', path)  # mass window applied here in might reject one
    markDuplicate('K_S0:combined', prioritiseV0)          # otherwise mark best duplicates
    applyCuts('K_S0:combined', 'extraInfo(highQualityVertex)')


def loadStdKS(path=analysis_main):
    stdKshorts(path)

# def loadStdAllKS(path=analysis_main):
#    stdPi('all',path)
#    reconstructDecay('K_S0:all -> pi-:all pi+:all', '0.4 < M < 0.6', 1, True,
#                     path)
#    vertexKFit('K_S0:all', 0.0)
#    applyCuts('K_S0:all', '0.477614<M<0.517614')
#    matchMCTruth('K_S0:all')
