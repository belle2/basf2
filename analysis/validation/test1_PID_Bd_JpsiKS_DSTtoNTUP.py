#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../mdst-jpsiks.root</input>
  <output>../ana-jpsiks.root</output>
  <contact>Francesco Tenchini; francesco.tenchini@unimelb.edu.au</contact>
</header>
"""

#######################################################
#
# Obtain lepton samples for PID performance validation
#
# Contributors: Francesco Tenchini (October 2016)
#
######################################################

from basf2 import *
from vertex import *
from modularAnalysis import *
from reconstruction import *

set_log_level(LogLevel.ERROR)

# load input ROOT file
inputMdst('default', '../mdst-jpsiks.root')

# --------------------------------------------------
# Create and fill final state ParticleLists
# --------------------------------------------------

# reconstruct everything as muons, use truth in analysis
fillParticleList('mu-:all', 'chiProb > 0.001 and abs(d0) < 2 and abs(z0) < 4')
fillParticleList('e-:all', 'chiProb > 0.001 and abs(d0) < 2 and abs(z0) < 4')
fillParticleList('pi-:all', 'chiProb > 0.001')

# --------------------------------------------------
# Reconstruct B->J/psi(->ee,mumu) Ks and truth match
# --------------------------------------------------

reconstructDecay('J/psi:mu -> mu-:all mu+:all', '2.8 < M < 3.3', 1)
vertexRave('J/psi:mu', 0.0)
matchMCTruth('J/psi:mu')
reconstructDecay('J/psi:e  -> e-:all  e+:all', ' 2.8 < M < 3.3', 2)
vertexRave('J/psi:e', 0.0)
matchMCTruth('J/psi:e')
# copyLists('J/psi:lept',['J/psi:mu','J/psi:e'])

reconstructDecay('K_S0 -> pi-:all pi+:all', '0.4 < M < 0.6')
vertexRave('K_S0', 0.0)
matchMCTruth('K_S0')

reconstructDecay('B0:mu -> J/psi:mu K_S0', '5.2 < M < 5.4')
vertexRave('B0:mu', 0.0)
matchMCTruth('B0:mu')
reconstructDecay('B0:e -> J/psi:e K_S0', '5.2 < M < 5.4')
vertexRave('B0:e', 0.0)
matchMCTruth('B0:e')

# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------

# information to be saved to file
mutools = ['EventMetaData', '^B0']
mutools += ['PID', ' B0 -> [ J/psi -> ^mu+ ^mu-][ K_S0 -> ^pi+ ^pi-]']
mutools += ['Kinematics', '^B0 -> [^J/psi -> ^mu+ ^mu-][^K_S0 -> ^pi+ ^pi-]']
mutools += ['InvMass', '^B0 -> [^J/psi ->  mu+  mu-][^K_S0 ->  pi+  pi-]']
mutools += ['MCTruth', '^B0 -> [^J/psi -> ^mu+ ^mu-][^K_S0 -> ^pi+ ^pi-]']
mutools += ['CustomFloats[isSignal]', '^B0 -> [^J/psi -> ^mu+ ^mu-][^K_S0 -> ^pi+ ^pi-]']

etools = ['EventMetaData', '^B0']
etools += ['PID', ' B0 -> [ J/psi -> ^e+ ^e-][ K_S0 -> ^pi+ ^pi-]']
etools += ['Kinematics', '^B0 -> [^J/psi -> ^e+ ^e-][^K_S0 -> ^pi+ ^pi-]']
etools += ['InvMass', '^B0 -> [^J/psi ->  e+  e-][^K_S0 ->  pi+  pi-]']
etools += ['MCTruth', '^B0 -> [^J/psi -> ^e+ ^e-][^K_S0 -> ^pi+ ^pi-]']
etools += ['CustomFloats[isSignal]', '^B0 -> [^J/psi -> ^e+ ^e-][^K_S0 -> ^pi+ ^pi-]']

# write out the flat ntuple
ntupleFile('../ana-jpsiks.root')
ntupleTree('jpsiks_mu', 'B0:mu', mutools)
ntupleTree('jpsiks_e', 'B0:e', etools)

# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------

process(analysis_main)
print(statistics)
