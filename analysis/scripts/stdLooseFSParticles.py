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
    fillParticleList('pi+:all', '', True, path)


def stdVeryLooseK(path=analysis_main):
    fillParticleList('K+:all', '', True, path)


def stdLoosePi(path=analysis_main):
    fillParticleList('pi+:loose', 'piid > 0.1 and chiProb > 0.001', True, path)


def stdLooseK(path=analysis_main):
    fillParticleList('K+:loose', 'Kid > 0.1 and chiProb > 0.001', True, path)


   # Add a loose PID here when Mu is available

def stdLooseMu(path=analysis_main):
    fillParticleList('mu+:loose', 'chiProb > 0.001', True, path)


def stdVeryLooseMu(path=analysis_main):
    fillParticleList('mu+:all', 'chiProb > 0.001', True, path)
