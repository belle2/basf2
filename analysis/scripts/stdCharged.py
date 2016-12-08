#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

# Prepare all standard final state particles

# Note: hadron efficiencies in labels are calculated for 1<p<4 GeV/c


def stdPi(listtype='95eff', path=analysis_main):
    if listtype == 'all':
        fillParticleList('pi+:all', '', True, path)
    elif listtype == '99eff':
        fillParticleList('pi+:99eff', 'piid > 0.010 and chiProb > 0.001', True, path)
    elif listtype == '95eff':
        fillParticleList('pi+:95eff', 'piid > 0.437 and chiProb > 0.001', True, path)
    elif listtype == '90eff':
        fillParticleList('pi+:90eff', 'piid > 0.722 and chiProb > 0.001', True, path)
    elif listtype == '85eff':
        fillParticleList('pi+:85eff', 'piid > 0.914 and chiProb > 0.001', True, path)
    else:
        fillParticleList('pi+:all', '', True, path)

###


def stdK(listtype='95eff', path=analysis_main):
    if listtype == 'all':
        fillParticleList('K+:all', '', True, path)
    elif listtype == '99eff':
        fillParticleList('K+:99eff', 'Kid > 0.001 and chiProb > 0.001', True, path)
    elif listtype == '95eff':
        fillParticleList('K+:95eff', 'Kid > 0.293 and chiProb > 0.001', True, path)
    elif listtype == '90eff':
        fillParticleList('K+:90eff', 'Kid > 0.611 and chiProb > 0.001', True, path)
    elif listtype == '85eff':
        fillParticleList('K+:85eff', 'Kid > 0.830 and chiProb > 0.001', True, path)
    else:
        fillParticleList('K+:all', '', True, path)

###


def stdPr(listtype='95eff', path=analysis_main):
    if listtype == 'all':
        fillParticleList('p+:all', '', True, path)
    elif listtype == '95eff':
        fillParticleList('p+:95eff', 'prid > 0.079 and chiProb > 0.001', True, path)
    elif listtype == '90eff':
        fillParticleList('p+:90eff', 'prid > 0.872 and chiProb > 0.001', True, path)
    else:
        fillParticleList('p+:all', '', True, path)

# Benchmarks for electrons are missing, default is 'all' instead of '95eff'


def stdE(listtype='all', path=analysis_main):
    if listtype == 'all':
        fillParticleList('e+:all', '', True, path)
    else:
        fillParticleList('e+:all', '', True, path)

# Benchmarks for muons are missing, default is 'all' instead of '95eff'


def stdMu(listtype='all', path=analysis_main):
    if listtype == 'all':
        fillParticleList('mu+:all', '', True, path)
    else:
        fillParticleList('mu+:all', '', True, path)


def loadStdCharged(path=analysis_main):

    # No PID
    stdK('all', path)
    stdPi('all', path)
    stdPr('all', path)
    stdE('all', path)
    stdMu('all', path)

    # Loose PID
    stdLooseK(path)
    stdLoosePi(path)
    stdLooseE(path)
    stdLooseMu(path)
    stdLoosePr(path)

###

# Loose FSParticles used for skimming


def stdLoosePi(path=analysis_main):
    fillParticleList('pi+:loose', 'piid > 0.1 and chiProb > 0.001', True, path)


def stdLooseK(path=analysis_main):
    fillParticleList('K+:loose', 'Kid > 0.1 and chiProb > 0.001', True, path)


def stdLooseMu(path=analysis_main):
    fillParticleList('mu+:loose', 'muid > 0.1 and chiProb > 0.001', True, path)


def stdLooseE(path=analysis_main):
    fillParticleList('e+:loose', 'eid > 0.1 and chiProb > 0.001', True, path)


def stdLoosePr(path=analysis_main):
    fillParticleList('p+:loose', 'prid > 0.1 and chiProb > 0.001', True, path)
