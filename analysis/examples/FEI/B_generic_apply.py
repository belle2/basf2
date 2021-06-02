#!/usr/bin/env python3

# William Sutcliffe 2019

import fei
import basf2 as b2
import modularAnalysis as ma

# Create path
path = b2.create_path()

# Load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('mdst14.root', 'validation', False),
             path=path)

# Add the necessary database
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# Get FEI default channels.
# Utilise the arguments to toggle on and off certain channels
particles = fei.get_default_channels()

# Set up FEI configuration specifying the FEI prefix
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2021_MC14_release_05_01_12', training=False, monitor=False, cache=0)

# Get FEI path
feistate = fei.get_path(particles, configuration)

# Add FEI path to the path to be processed
path.add_path(feistate.path)

# Add MC matching when applying to MC. This is required for variables like isSignal and mcErrors below
path.add_module('MCMatcherParticles', listName='B+:generic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B+:semileptonic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B0:generic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B0:semileptonic', looseMCMatching=True)

# Store tag-side variables of interest.
ma.variablesToNtuple('B+:generic',
                     ['Mbc',
                      'deltaE',
                      'mcErrors',
                      'extraInfo(decayModeID)',
                      'extraInfo(uniqueSignal)',
                      'extraInfo(SignalProbability)',
                      'isSignal'],
                     filename='B_charged_hadronic.root',
                     path=path)
ma.variablesToNtuple('B+:semileptonic',
                     ['cosThetaBetweenParticleAndNominalB',
                      'mcErrors',
                      'extraInfo(decayModeID)',
                      'extraInfo(uniqueSignal)',
                      'extraInfo(SignalProbability)',
                      'isSignalAcceptMissingNeutrino'],
                     filename='B_charged_semileptonic.root',
                     path=path)

ma.variablesToNtuple('B0:generic',
                     ['Mbc',
                      'deltaE',
                      'mcErrors',
                      'extraInfo(decayModeID)',
                      'extraInfo(uniqueSignal)',
                      'extraInfo(SignalProbability)',
                      'isSignal'],
                     filename='B_mixed_hadronic.root',
                     path=path)
ma.variablesToNtuple('B0:semileptonic',
                     ['cosThetaBetweenParticleAndNominalB',
                      'mcErrors',
                      'extraInfo(decayModeID)',
                      'extraInfo(uniqueSignal)',
                      'extraInfo(SignalProbability)',
                      'isSignalAcceptMissingNeutrino'],
                     filename='B_mixed_semileptonic.root',
                     path=path)

# Process 100 events
b2.process(path, max_event=100)
print(b2.statistics)
