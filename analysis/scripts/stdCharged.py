#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

# Prepare all standard final state particles
# Loose FSParticles are used for skimming


def loadStdCharged(path=analysis_main):

    # No PID
    loadStdAllK(path)
    loadStdAllPi(path)
    loadStdAllE(path)
    loadStdAllMu(path)
    loadStdAllPr(path)

    # Loose PID
    loadStdLooseK(path)
    loadStdLoosePi(path)
    loadStdLooseE(path)
    loadStdLooseMu(path)
    loadStdLoosePr(path)

# No PID


def loadStdAllPi(path=analysis_main):
    fillParticleList('pi+:all', '', True, path)


def loadStdAllK(path=analysis_main):
    fillParticleList('K+:all', '', True, path)


def loadStdAllE(path=analysis_main):
    fillParticleList('e+:all', '', True, path)


def loadStdAllMu(path=analysis_main):
    fillParticleList('mu+:all', '', True, path)


def loadStdAllPr(path=analysis_main):
    fillParticleList('p+:all', '', True, path)

# Loose PID / To be refined


def loadStdLoosePi(path=analysis_main):
    fillParticleList('pi+:loose', 'piid > 0.1 and chiProb > 0.001', True, path)


def loadStdLooseK(path=analysis_main):
    fillParticleList('K+:loose', 'Kid > 0.1 and chiProb > 0.001', True, path)


def loadStdLooseMu(path=analysis_main):
    fillParticleList('mu+:loose', 'muid > 0.1 and chiProb > 0.001', True, path)


def loadStdLooseE(path=analysis_main):
    fillParticleList('e+:loose', 'eid > 0.1 and chiProb > 0.001', True, path)


def loadStdLoosePr(path=analysis_main):
    fillParticleList('p+:loose', 'prid > 0.1 and chiProb > 0.001', True, path)
