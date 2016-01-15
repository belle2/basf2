#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from stdPhotons import *

# Prepare all standard final state particles


def loadStdPi0(path=analysis_main):
    loadStdAllPi0(path)


def loadStdAllPi0(path=analysis_main):
    loadStdGoodPhoton(path)
    reconstructDecay('pi0:all -> gamma:good gamma:good', '0.11 < M < 0.16', 1,
                     True, path)
    massKFit('pi0:all', 0.0, '', path)
    matchMCTruth('pi0:all', path)


def loadStdLoosePi0(path=analysis_main):
    loadStdPi0(path)
    cutAndCopyList('pi0:loose', 'pi0:all', '-0.6 < extraInfo(BDT) < 1.0',
                   True, path)


def loadStdGoodPi0(path=analysis_main):
    loadStdPi0(path)
    cutAndCopyList('pi0:good', 'pi0:all', '0.5 < extraInfo(BDT) < 1.0', True,
                   path)
