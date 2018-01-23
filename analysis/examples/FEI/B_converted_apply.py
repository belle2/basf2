#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

from basf2 import *
from modularAnalysis import *

import b2biiConversion
import ROOT
from ROOT import Belle2
ROOT.Belle2.BFieldManager.getInstance().setConstantOverride(0, 0, 1.5 * ROOT.Belle2.Unit.T)

# In case you have problems with the conditions database you can use the localdb of the FEI directly
# use_local_database('/home/belle2/tkeck/feiv4/Belle1_2017_convertedMC_Track14_2/localdb/database.txt',
#                     '/home/belle2/tkeck/feiv4/Belle1_2017_convertedMC_Track14_2/localdb/', True, LogLevel.WARNING)

path = create_path()
b2biiConversion.convertBelleMdstToBelleIIMdst(None, applyHadronBJSkim=True, path=path)
setAnalysisConfigParams({'mcMatchingVersion': 'Belle'}, path)

import fei
particles = fei.get_default_channels(convertedFromBelle=True)
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2017_MCConverted_Track14_2', b2bii=True, training=False, monitor=False)
feistate = fei.get_path(particles, configuration)

path.add_path(feistate.path)

path.add_module('MCMatcherParticles', listName='B+:generic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B+:semileptonic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B0:generic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B0:semileptonic', looseMCMatching=True)

variablesToNTuple('B+:generic', ['uniqueEventID', 'Mbc', 'deltaE', 'mcErrors', 'extraInfo(decayModeID)',
                                 'extraInfo(uniqueSignal)', 'extraInfo(SignalProbability)', 'isSignal'],
                  filename='B_charged_hadronic.root', path=path)
variablesToNTuple('B+:semileptonic',
                  ['uniqueEventID',
                   'cosThetaBetweenParticleAndTrueB',
                   'mcErrors',
                   'extraInfo(decayModeID)',
                   'extraInfo(uniqueSignal)',
                   'extraInfo(SignalProbability)',
                   'isSignalAcceptMissingNeutrino'],
                  filename='B_charged_semileptonic.root',
                  path=path)

variablesToNTuple('B0:generic',
                  ['uniqueEventID',
                   'Mbc',
                   'deltaE',
                   'mcErrors',
                   'extraInfo(decayModeID)',
                   'extraInfo(uniqueSignal)',
                   'extraInfo(SignalProbability)',
                   'isSignal'],
                  filename='B_mixed_hadronic.root',
                  path=path)
variablesToNTuple('B0:semileptonic',
                  ['uniqueEventID',
                   'cosThetaBetweenParticleAndTrueB',
                   'mcErrors',
                   'extraInfo(decayModeID)',
                   'extraInfo(uniqueSignal)',
                   'extraInfo(SignalProbability)',
                   'isSignalAcceptMissingNeutrino'],
                  filename='B_mixed_semileptonic.root',
                  path=path)

print(path)
process(path)
print(statistics)
