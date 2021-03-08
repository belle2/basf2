#!/usr/bin/env python3

# William Sutcliffe 2019

import os

import fei
import basf2 as b2
import modularAnalysis as ma

import b2biiConversion

# To properly read the Belle database the user name is set to g0db
os.environ['PGUSER'] = 'g0db'
# Add the necessary global tag
# b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())
b2.conditions.prepend_globaltag('analysis_tools_light-2012-minos')

# Create path
path = b2.create_path()

# Run conversion on input file
b2biiConversion.convertBelleMdstToBelleIIMdst(
    b2.find_file(
        'analysis/mdstBelle1_exp65_charged.root',
        'validation',
        False),
    applySkim=True,
    # Actually, the KS finder should be set to True.
    # However, here it's set to False because the necessary library is only present on kekcc and not on the build server.
    enableNisKsFinder=False,
    enableLocalDB=False,
    path=path)
ma.setAnalysisConfigParams({'mcMatchingVersion': 'Belle'}, path)

# Get FEI default channels for a converted training
# Utilise the arguments to toggle on and off certain channels
particles = fei.get_default_channels()

# Set up FEI configuration specifying the FEI prefix of the Belle legacy training
configuration = fei.config.FeiConfiguration(prefix='FEI_B2BII_light-2012-minos',
                                            training=False, monitor=False, cache=0)

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
