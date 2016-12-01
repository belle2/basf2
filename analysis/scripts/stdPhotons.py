#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
# Prepare all standard final state particles


def loadStdAllPhoton(path=analysis_main):
    fillParticleList('gamma:all', '', True, path)
    calibratePhotonEnergy('gamma:all', 0.030, path)


def loadStdPhoton(path=analysis_main):
    loadStdAllPhoton(path)


def loadStdGoodPhoton(path=analysis_main):
    loadStdAllPhoton(path)
    cutAndCopyList('gamma:good', 'gamma:all', '0.5 < goodGamma < 1.5', True, path)


def loadStdGoodBellePhoton(path=analysis_main):
    loadStdAllPhoton(path)
    cutAndCopyList('gamma:goodBelle', 'gamma:all', '0.5 < goodBelleGamma < 1.5', True, path)


def loadStdPhotonE12(path=analysis_main):
    loadStdAllPhoton(path)
    cutAndCopyList('gamma:E12', 'gamma:all', '1.2 < E < 100', True, path)


def loadStdPhotonE15(path=analysis_main):
    loadStdAllPhoton(path)
    cutAndCopyList('gamma:E15', 'gamma:all', '1.5 < E < 100', True, path)
