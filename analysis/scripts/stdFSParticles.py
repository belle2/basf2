#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from stdLooseFSParticles import stdVeryLoosePi

# neutrals lists
from stdPhotons import *
from stdPi0s import *

# Prepare all standard final state particles


def stdFSParticles(path=analysis_main):

    # Nominal PID
    stdK(path)
    stdPi(path)

    # standard photons
    stdPhotons('loose', path)

    # reconstruct standard pi0s including a mass constraint fit
    stdPi0s('veryLooseFit', path)

    # reconstruct standard Ks
    stdKs()

    # Egamma > 1.5 GeV
#    stdHighEPhoton(path)


def stdPi(path=analysis_main):
    fillParticleList('pi+:std', 'piid > 0.5 and chiProb > 0.001', True, path)


def stdK(path=analysis_main):
    fillParticleList('K+:std', 'Kid > 0.5 and chiProb > 0.001', True, path)


def stdKs(path=analysis_main):

    stdVeryLoosePi()
    reconstructDecay('K_S0:all -> pi-:all pi+:all', '0.4 < M < 0.6', 1, True,
                     path)
    vertexKFit('K_S0:all', 0.0)
    applyCuts('K_S0:all', '0.477614<M<0.517614')
    matchMCTruth('K_S0:all')
