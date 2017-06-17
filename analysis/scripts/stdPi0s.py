#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from stdPhotons import *

# Prepare all standard final state particles


def stdPi0s(listtype='veryLoose', path=analysis_main):
    if listtype == 'veryLoose':
        fillParticleList('gamma:pi0', '', True, path)
        reconstructDecay('pi0:veryLoose -> gamma:pi0 gamma:pi0', '0.09 < M < 0.165', 1, True, path)
        matchMCTruth('pi0:veryLoose', path)
    elif listtype == 'loose':
        fillParticleList('gamma:pi0highE', '', True, path)
        reconstructDecay('pi0:loose -> gamma:pi0highE gamma:pi0highE', '0.1 < M < 0.160', 1, True, path)
        matchMCTruth('pi0:loose', path)
    elif listtype == 'veryLooseFit':
        fillParticleList('gamma:pi0', '', True, path)
        reconstructDecay('pi0:veryLooseFit -> gamma:pi0 gamma:pi0', '0.09 < M < 0.165', 1, True, path)
        massKFit('pi0:veryLooseFit', 0.0, '', path)
        matchMCTruth('pi0:veryLooseFit', path)
    elif listtype == 'looseFit':
        fillParticleList('gamma:pi0highE', '', True, path)
        reconstructDecay('pi0:looseFit -> gamma:pi0highE gamma:pi0highE', '0.1 < M < 0.160', 1, True, path)
        massKFit('pi0:looseFit', 0.0, '', path)
        matchMCTruth('pi0:looseFit', path)
    elif listtype == 'all':
        fillParticleList('gamma:all', '', True, path)
        reconstructDecay('pi0:all -> gamma:all gamma:all', '0.09 < M < 0.165', 1, True, path)
        matchMCTruth('pi0:all', path)
    elif listtype == 'allFit':
        fillParticleList('gamma:all', '', True, path)
        reconstructDecay('pi0:allFit -> gamma:all gamma:all', '0.09 < M < 0.165', 1, True, path)
        massKFit('pi0:allFit', 0.0, '', path)
        matchMCTruth('pi0:allFit', path)
    else:
        fillParticleList('gamma:pi0', '', True, path)
        reconstructDecay('pi0:veryLooseFit -> gamma:pi0 gamma:pi0', '0.09 < M < 0.165', 1, True, path)
        massKFit('pi0:veryLooseFit', 0.0, '', path)
        matchMCTruth('pi0:veryLooseFit', path)


def loadStdPi0(listtype='veryLoose', path=analysis_main):
    stdPi0s(listtype, path)


def loadStdAllPi0(path=analysis_main):
    stdPi0s('all', path)

# not recommended! only here for backwards compatibility


def loadStdLoosePi0(path=analysis_main):
    loadStdPi0(path)
    cutAndCopyList('pi0:loose', 'pi0:all', '-0.6 < extraInfo(BDT) < 1.0',
                   True, path)


def loadStdGoodPi0(path=analysis_main):
    loadStdPi0(path)
    cutAndCopyList('pi0:good', 'pi0:all', '0.5 < extraInfo(BDT) < 1.0', True,
                   path)
