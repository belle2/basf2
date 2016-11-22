#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *


def stdPhotons(listtype='loose', path=analysis_main):

    if listtype == 'all':
        fillParticleList('gamma:all', '', True, path)
    elif listtype == 'loose':
        stdPhotons('all', path)
        cutAndCopyList('gamma:loose', 'gamma:all', 'clusterErrorTiming < 1e6 and [clusterE1E9 > 0.4 or E > 0.075]', True, path)
    elif listtype == 'tight':
        stdPhotons('loose', path)
        cutAndCopyList(
            'gamma:tight',
            'gamma:loose',
            '[clusterReg == 1 and E > 0.05] or [clusterReg == 2 and E > 0.05] or [clusterReg == 3 and E > 0.075]',
            True,
            path)
    elif listtype == 'pi0':
        stdPhotons('loose', path)
        cutAndCopyList(
            'gamma:pi0',
            'gamma:loose',
            '[clusterReg == 1 and E > 0.05] or [clusterReg == 2 and E > 0.05] or [clusterReg == 3 and E > 0.075]',
            True,
            path)
    elif listtype == 'pi0highE':
        stdPhotons('loose', path)
        cutAndCopyList(
            'gamma:pi0highE',
            'gamma:loose',
            '[clusterReg == 1 and E > 0.2] or [clusterReg == 2 and E > 0.2] or [clusterReg == 3 and E > 0.2]',
            True,
            path)
    else:
        fillParticleList('gamma:all', '', True, path)

# Used in skimming code


def loadStdAllPhoton(path=analysis_main):
    fillParticleList('gamma:all', '', True, path)


def loadStdPhoton(path=analysis_main):
    loadStdAllPhoton(path)


def loadStdGoodPhoton(path=analysis_main):
    loadStdAllPhoton(path)
    cutAndCopyList('gamma:good', 'gamma:all', '0.5 < goodGamma < 1.5', True, path)


def loadStdPhotonE12(path=analysis_main):
    loadStdAllPhoton(path)
    cutAndCopyList('gamma:E12', 'gamma:all', '1.2 < E < 100', True, path)


def loadStdPhotonE15(path=analysis_main):
    loadStdAllPhoton(path)
    cutAndCopyList('gamma:E15', 'gamma:all', '1.5 < E < 100', True, path)
