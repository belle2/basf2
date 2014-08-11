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
    stdPhoton(path)

  # reconstruct standard pi0
    stdPi0(path)

  # Egamma > 1.5 GeV
    stdHighEPhoton(path)


def stdPi(path=analysis_main):
    fillParticleList('pi+:std', 'piid > 0.5 and chiProb > 0.001', True, path)


def stdK(path=analysis_main):
    fillParticleList('K+:std', 'Kid > 0.5 and chiProb > 0.001', True, path)


def stdPi0(path=analysis_main):
    goodPhoton(path)
    reconDecay('pi0:all -> gamma:good gamma:good', '0.11 < M < 0.16', 1,
               True, path)
    massKFit('pi0:all', 0.0, '', path)
    matchMCTruth('pi0:all', path)

    expert = register_module('TMVAExpert')
    expert.param('prefix', 'PI0-MC3.5')
    expert.param('workingDirectory', os.environ['BELLE2_LOCAL_DIR']
                 + '/analysis/data/TMVA/StandardPi0/')
    expert.param('method', 'BDTG')
    expert.param('listNames', ['pi0:all'])
    expert.param('signalProbabilityName', 'BDT')
    path.add_module(expert)

    cutAndCopyList('pi0:loose', 'pi0:all', '-0.6 < getExtraInfo(BDT) < 1.0',
                   True, path)
    cutAndCopyList('pi0:good', 'pi0:all', '0.5 < getExtraInfo(BDT) < 1.0',
                   True, path)


def loosePi0(path=analysis_main):
    stdPi0(path)
    cutAndCopyList('pi0:loose', 'pi0:all', '-0.6 < getExtraInfo(BDT) < 1.0',
                   True, path)


def goodPi0(path=analysis_main):
    stdPi0(path)
    cutAndCopyList('pi0:good', 'pi0:all', '0.5 < getExtraInfo(BDT) < 1.0',
                   True, path)


def stdPhoton(path=analysis_main):
    fillParticleList('gamma:all', '', True, path)
    calibratePhotonEnergy('gamma:all', path)


def goodPhoton(path=analysis_main):
    stdPhoton(path)
    cutAndCopyList('gamma:good', 'gamma:all', '0.5 < goodGamma < 1.5', True,
                   path)


def stdHighEPhoton(path=analysis_main):
    stdPhoton(path)
    cutAndCopyList('gamma:highE', 'gamma:all', '1.5 < E < 100', True, path)
