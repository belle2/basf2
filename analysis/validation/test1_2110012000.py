#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../2110012000.dst.root</input>
  <output>../2110012000.ntup.root</output>
  <contact>Jake Bennett; jvbennett@cmu.edu</contact>
</header>
"""

#######################################################
#
# D+ sample for charm validation.
#
# ccbar -> D+ -> pi+ pi0
#
# Contributors: Giulia Casarosa (April 2017)
#
######################################################

from basf2 import *
from vertex import *
from modularAnalysis import *
from reconstruction import *
from stdPi0s import *

# load input ROOT file
inputMdst('default', '../2110012000.dst.root')

# --------------------------------------------------
# Create and fill final state ParticleLists
# --------------------------------------------------

# creates "pi+:loose" ParticleList (and c.c.)
fillParticleList('pi+:all', 'chiProb > 0.001 and abs(d0) < 2 and abs(z0) < 4')
copyList('pi+:pt', 'pi+:all')
applyCuts('pi+:pt', 'pt > 0.1')

stdPi0s('veryLoose')
# reconstruct D+:kpi and perform a mass constrained vertex fit
reconstructDecay('D+:sig -> pi+:pt pi0:veryLoose', '1.81 < M < 1.91')
massVertexRave('D+:sig', 0.001)

# truth matching
matchMCTruth('D+:sig')

# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------

# information to be saved to file
toolsDZ = ['EventMetaData', '^D*+']
toolsDZ += ['InvMass', '^D+ -> pi+ ^pi0']
toolsDZ += ['InvMass[BeforeFit]', '^D+ -> pi+ ^pi0']
toolsDZ += ['CustomFloats[isSignal]', '^D+ -> pi+ pi0']
toolsDZ += ['Kinematics', '^D+ -> ^pi+ ^pi0']
toolsDZ += ['CMSKinematics', '^D+ -> ^pi+ ^pi0']
toolsDZ += ['Vertex', '^D+ -> pi+ pi0']
toolsDZ += ['Track', 'D+ -> ^pi+ pi0']
toolsDZ += ['TrackHits', 'D+ -> ^pi+ pi0']

toolsDZ += ['MCKinematics', '^D+ -> ^pi+ ^pi0']
toolsDZ += ['MCVertex', '^D+ -> pi+ pi0']
toolsDZ += ['MCTruth', '^D+ -> pi+ pi0']

# write out the flat ntuple
ntupleFile('../2110012000.ntup.root')
ntupleTree('dp', 'D+:sig', toolsDZ)

summaryOfLists(['D+:sig', 'pi+:pt', 'pi0:veryLoose'])
# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------

analysis_main.add_module('Progress')

process(analysis_main)

print(statistics)
