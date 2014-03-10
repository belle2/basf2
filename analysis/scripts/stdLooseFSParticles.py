#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

# Prepare all standard final state particles


def stdLooseFSParticles(path=analysis_main):

  # No PID
    stdVeryLooseK(path)
    stdVeryLoosePi(path)

  # Loose PID
    stdLooseK(path)
    stdLoosePi(path)
    stdLooseMu(path)
    stdVeryLooseMu(path)


def stdVeryLoosePi(path=analysis_main):
    selectParticle('StdVeryLoosePi+', 211, [''], True, path)
    selectParticle('StdVeryLoosePi-', -211, [''], True, path)


def stdVeryLooseK(path=analysis_main):
    selectParticle('StdVeryLooseK+', 321, [''], True, path)
    selectParticle('StdVeryLooseK-', -321, [''], True, path)


def stdLoosePi(path=analysis_main):
    selectParticle('StdLoosePi+', 211, ['piid 0.1:', 'chiProb 0.001:'], True,
                   path)
    selectParticle('StdLoosePi-', -211, ['piid 0.1:', 'chiProb 0.001:'], True,
                   path)


def stdLooseK(path=analysis_main):
    selectParticle('StdLooseK-', -321, ['Kid 0.1:', 'chiProb 0.001:'], True,
                   path)
    selectParticle('StdLooseK+', 321, ['Kid 0.1:', 'chiProb 0.001:'], True,
                   path)


   # Add a loose PID here when Mu is available


def stdLooseMu(path=analysis_main):
    selectParticle('StdLooseMu+', 13, ['chiProb 0.001:'], True, path)
    selectParticle('StdLooseMu-', -13, ['chiProb 0.001:'], True, path)


def stdVeryLooseMu(path=analysis_main):
    selectParticle('StdVeryLooseMu+', 13, ['chiProb 0.001:'], True, path)
    selectParticle('StdVeryLooseMu-', -13, ['chiProb 0.001:'], True, path)


