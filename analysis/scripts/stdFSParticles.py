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
  # reconstruct standard Ks
    stdKs()
  # Egamma > 1.5 GeV
    stdHighEPhoton(path)


def stdPi(path=analysis_main):
    fillParticleList('pi+:std', 'piid > 0.5 and chiProb > 0.001', True, path)


def stdK(path=analysis_main):
    fillParticleList('K+:std', 'Kid > 0.5 and chiProb > 0.001', True, path)


def stdPi0(path=analysis_main):
    goodPhoton(path)
    reconstructDecay('pi0:all -> gamma:good gamma:good', '0.11 < M < 0.16', 1,
                     True, path)
    massKFit('pi0:all', 0.0, '', path)
    matchMCTruth('pi0:all', path)

    expert = register_module('TMVAExpert')
    expert.param('prefix', 'PI0-MC3.5')
    expert.param('workingDirectory', os.environ['BELLE2_LOCAL_DIR']
                 + '/analysis/data/TMVA/StandardPi0/')
    expert.param('method', 'BDTG')
    expert.param('listNames', ['pi0:all'])
    expert.param('expertOutputName', 'BDT')
    path.add_module(expert)

    cutAndCopyList('pi0:loose', 'pi0:all', '-0.6 < extraInfo(BDT) < 1.0',
                   True, path)
    cutAndCopyList('pi0:good', 'pi0:all', '0.5 < extraInfo(BDT) < 1.0', True,
                   path)


def loosePi0(path=analysis_main):
    stdPi0(path)
    cutAndCopyList('pi0:loose', 'pi0:all', '-0.6 < extraInfo(BDT) < 1.0',
                   True, path)


def goodPi0(path=analysis_main):
    stdPi0(path)
    cutAndCopyList('pi0:good', 'pi0:all', '0.5 < extraInfo(BDT) < 1.0', True,
                   path)


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


def stdKs(path=analysis_main):

    stdPi()
    reconstructDecay('K_S0:all -> pi-:all pi+:all', '0.4 < M < 0.6', 1, True,
                     path)
    vertexKFit('K_S0:all', 0.0)
    applyCuts('K_S0:all', '0.477614<M<0.517614')
    matchMCTruth('K_S0:all')

    expert_v0 = register_module('TMVAExpert')
    expert_v0.param('prefix', 'v0_TMVA')
    expert_v0.param('workingDirectory',
                    '/home/belle2/pjaeger/public/StandardKs/')
    expert_v0.param('method', 'FastBDT')
    expert_v0.param('listNames', ['K_S0:all'])
    expert_v0.param('expertOutputName', 'BDT_v0')
    path.add_module(expert_v0)

    expert_l = register_module('TMVAExpert')
    expert_l.param('prefix', 'lambda_TMVA')
    expert_l.param('workingDirectory',
                   '/home/belle2/pjaeger/public/StandardKs/')
    expert_l.param('method', 'FastBDT')
    expert_l.param('listNames', ['K_S0:all'])
    expert_l.param('expertOutputName', 'BDT_l')
    path.add_module(expert_l)

    cutAndCopyList('K_S0:good', 'K_S0:all',
                   '0.51 < extraInfo(BDT_v0) < 1.0 and 0.91 < extraInfo(BDT_l) < 1.0', True, path)
