#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to load reconstructed
# final state particles as Particles in case of V0-like particles:
# - K_S0
# - Lambda0
# - (anti-Lambda0)
#
# There are 2 possibilities of doing it:
# - fill the ParticleLists using V0s an input MDST
# - use the reconstructDecay() function
# Some default function are already implemented
# and can be directly used
#
# (location: analysis/scripts/stdV0s.py)
#
# Contributors: B. Scavino (August 2018)
#
################################################################################

from basf2 import *
from modularAnalysis import inputMdst
from modularAnalysis import printDataStore
from modularAnalysis import printList
from modularAnalysis import fillParticleList
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import analysis_main

from stdV0s import *
# check if the required input file exists (from B2A101 example)
import os.path
import sys
if not os.path.isfile('B2A101-Y4SEventGeneration-gsim-BKGx0.root'):
    sys.exit('Required input file (B2A101-Y4SEventGeneration-gsim-BKGx0.root) does not exist. '
             'Please run B2A101-Y4SEventGeneration.py and B2A103-SimulateAndReconstruct-withoutBeamBkg.py '
             'tutorial scripts first.')

# load input ROOT file
inputMdst('default', 'B2A101-Y4SEventGeneration-gsim-BKGx0.root')

# print contents of the DataStore before loading Particles
printDataStore()

# create and fill Ks/Lambda0 ParticleLists, using V0s as source
# second argument are the selection criteria: '' means no cut, take all
# the decay chain has to be specified (i.e. the two daughters, as well)
# A vertex fit should also be performed
# In this example a cut on the candidates mass is applied
fillParticleList('K_S0:V0 -> pi+ pi-', '0.3 < M < 0.7')
vertexKFit('K_S0:V0', 0.0)
fillParticleList('Lambda0:V0 -> p+ pi-', '0.9 < M < 1.3')
vertexKFit('Lambda0:V0', 0.0)

# alternatively, we can create a list of particles combined
# using the analysis ParticleCombiner module
# Before doing it we need to create and fill ParticleLists
# of the V0s' daughters (anti-particle lists are automatically
# created, too)
# A vertex fit should also be performed
# In this example a cut on the candidates mass is applied
fillParticleList('pi-:all', '')
fillParticleList('p+:all', '')

reconstructDecay('K_S0:RD -> pi+:all pi-:all', '0.3 < M < 0.7')
vertexKFit('K_S0:RD', 0.0)
reconstructDecay('Lambda0:RD -> p+:all pi-:all', '0.9 < M < 1.3')
vertexKFit('Lambda0:RD', 0.0)

# another possibility is to use default functions
# for V0s they are defined in analysis/scripts/stdV0s.py
# e.g. stdKshorts():
# - takes all V0 candidates, performs vertex fit, and fills 'K_S0:all' ParticleList
#   a cut on the candidates mass is applied, too
stdKshorts()
stdLambdas()

# print contents of the DataStore after loading Particles
printDataStore()

# print out the contents of each ParticleList
printList('K_S0:V0', False)
printList('Lambda0:V0', False)

printList('K_S0:RD', False)
printList('Lambda0:RD', False)

printList('K_S0:all', False)
printList('Lambda0:all', False)

# define Ntuple tools for V0s:
toolsK0 = ['EventMetaData', '^K_S0']
toolsK0 += ['Kinematics', '^K_S0 -> ^pi+ ^pi-']
toolsK0 += ['InvMass', '^K_S0']
toolsK0 += ['Vertex', '^K_S0']
toolsK0 += ['MCVertex', '^K_S0']
toolsK0 += ['PID', 'K_S0 -> ^pi+ ^pi-']
toolsK0 += ['Track', 'K_S0 -> ^pi+ ^pi-']
toolsK0 += ['TrackHits', 'K_S0 -> ^pi+ ^pi-']
toolsK0 += ['MCTruth', '^K_S0 -> ^pi+ ^pi-']
toolsK0 += ['CustomFloats[dr:dz:isSignal:chiProb]', '^K_S0']
toolsK0 += ['MCHierarchy[Intermediate]', '^K_S0']

toolsLambda0 = ['EventMetaData', '^Lambda0']
toolsLambda0 += ['Kinematics', '^Lambda0 -> ^p+ ^pi-']
toolsLambda0 += ['InvMass', '^Lambda0']
toolsLambda0 += ['Vertex', '^Lambda0']
toolsLambda0 += ['MCVertex', '^Lambda0']
toolsLambda0 += ['PID', 'Lambda0 -> ^p+ ^pi-']
toolsLambda0 += ['Track', 'Lambda0 -> ^p+ ^pi-']
toolsLambda0 += ['TrackHits', 'Lambda0 -> ^p+ ^pi-']
toolsLambda0 += ['MCTruth', '^Lambda0 -> ^p+ ^pi-']
toolsLambda0 += ['CustomFloats[dr:dz:isSignal:chiProb]', '^Lambda0']
toolsLambda0 += ['MCHierarchy[Intermediate]', '^Lambda0']

ntupleFile('B2A203-LoadV0s.root')
ntupleTree('kshort_v0', 'K_S0:V0', toolsK0)  # K_S0 from V0s
ntupleTree('kshort_rd', 'K_S0:RD', toolsK0)  # K_S0 from reconstructDecay
ntupleTree('kshort_std', 'K_S0:all', toolsK0)  # K_S0 from standard list (=V0s, see stdV0s.py)

ntupleTree('lambda_v0', 'Lambda0:V0', toolsLambda0)  # Lambda0 from V0s
ntupleTree('lambda_rd', 'Lambda0:RD', toolsLambda0)  # Lambda0 from reconstructDecay
ntupleTree('lambda_std', 'Lambda0:all', toolsLambda0)  # Lambda0 from standard list (=V0s, see stdV0s.py)
# Process the events
process(analysis_main)

# print out the summary
print(statistics)
