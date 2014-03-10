#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

# Prepare all standard final state particles


def stdFSParticles(path=analysis_main):

  # Nominal PID
    stdK(path)
    stdPi(path)

  # MDST standard
    stdPi0(path)
    stdPhoton(path)

  # Egamma > 1.5 GeV
    stdHighEPhoton(path)


def stdPi(path=analysis_main):
    selectParticle('StdPi+', 211, ['piid 0.5:', 'chiProb 0.001:'], True, path)
    selectParticle('StdPi-', -211, ['piid 0.5:', 'chiProb 0.001:'], True, path)


def stdK(path=analysis_main):
    selectParticle('StdK-', -321, ['Kid 0.5:', 'chiProb 0.001:'], True, path)
    selectParticle('StdK+', 321, ['Kid 0.5:', 'chiProb 0.001:'], True, path)


def stdPi0(path=analysis_main):
    selectParticle('StdPi0', 111, [''], True, path)


def stdPhoton(path=analysis_main):
    selectParticle('StdPhoton', 22, [''], True, path)


def stdHighEPhoton(path=analysis_main):
    selectParticle('StdHighEPhoton', 22, ['p 1.5:'], True, path)


