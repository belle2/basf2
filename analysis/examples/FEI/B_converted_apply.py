#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# William Sutcliffe 2019

import fei
import basf2 as b2
import modularAnalysis as ma

import b2biiConversion
import ROOT
from ROOT import Belle2


# Add the necessary database
# You can use the command b2conditionsdb-recommend
b2.conditions.globaltags = ['B2BII_MC']

# Create path
path = b2.create_path()

# Run conversion on input file
b2biiConversion.convertBelleMdstToBelleIIMdst(None, applyHadronBJSkim=True, path=path)
ma.setAnalysisConfigParams({'mcMatchingVersion': 'Belle'}, path)

# Get FEI default channels for a converted training
# Utilise the arguments to toggle on and off certain channels
particles = fei.get_default_channels(convertedFromBelle=True)

# Set up FEI configuration specifying the FEI prefix of the Belle legacy training
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2017_MCConverted_Track14_2', b2bii=True, training=False, monitor=False)

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
