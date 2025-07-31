#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import fei
import basf2 as b2
import modularAnalysis as ma

# Create path
path = b2.create_path()

# Load input ROOT file
ma.inputMdst(filename=b2.find_file('mdst16.root', 'validation', False),
             path=path)

# Add the necessary database
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# Get FEI default channels.
# Utilise the arguments to toggle on and off certain channels
particles = fei.get_default_channels(removeSLD=True)

# Set up FEI configuration specifying the FEI prefix
configuration = fei.config.FeiConfiguration(prefix='FEIv1_2025_MC16ri_aldebaran_200', training=False, monitor=False)

# Get FEI path
feistate = fei.get_path(particles, configuration)

# Add FEI path to the path to be processed
path.add_path(feistate.path)

# Add MC matching when applying to MC. This is required for variables like isSignal and mcErrors below
path.add_module('MCMatcherParticles', listName='B+:generic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B+:semileptonic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B0:generic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B0:semileptonic', looseMCMatching=True)

commonVariables = ['mcErrors', 'extraInfo(decayModeID)', 'extraInfo(uniqueSignal)', 'extraInfo(SignalProbability)']
genericVariables = ['Mbc', 'deltaE', 'isSignal'] + commonVariables
semiLeptonicVariables = ['cosThetaBetweenParticleAndNominalB', 'isSignalAcceptMissingNeutrino'] + commonVariables

# Store tag-side variables of interest.
ma.variablesToNtuple('B+:generic',
                     genericVariables,
                     treename='Bplus',
                     filename='B_charged_hadronic.root',
                     path=path)
ma.variablesToNtuple('B+:semileptonic',
                     semiLeptonicVariables,
                     treename='BplusSL',
                     filename='B_charged_semileptonic.root',
                     path=path)

ma.variablesToNtuple('B0:generic',
                     genericVariables,
                     treename='B0',
                     filename='B_mixed_hadronic.root',
                     path=path)
ma.variablesToNtuple('B0:semileptonic',
                     semiLeptonicVariables,
                     treename='B0SL',
                     filename='B_mixed_semileptonic.root',
                     path=path)

# Process 100 events
b2.process(path, max_event=100, calculateStatistics=True)
print(b2.statistics)
