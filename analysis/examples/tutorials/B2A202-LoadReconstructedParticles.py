#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################                                                                                     #
# This tutorial demonstrates how to load reconstructed
# final state particles as Particles:
# - Tracks are loaded as e/mu/pi/K/p Particles
# - neutral ECLClusters are loaded as photons
# - neutral KLMClusters are loaded as Klongs
#
# Create ParticleList for each final state
# particle type as well.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdst
from modularAnalysis import loadReconstructedParticles
from modularAnalysis import printDataStore
from modularAnalysis import printList
from modularAnalysis import fillParticleList
from modularAnalysis import analysis_main

# check if the required input file exists (from B2A101 example)
import os.path
import sys
if not os.path.isfile('B2A101-Y4SEventGeneration-gsim-BKGx0.root'):
    sys.exit('Required input file (B2A101-Y4SEventGeneration-gsim-BKGx0.root) does not exist. Please run B2A101-Y4SEventGeneration.py and B2A103-SimulateAndReconstruct-withoutBeamBkg.py tutorial scripts first.'
             )

# load input ROOT file
inputMdst('B2A101-Y4SEventGeneration-gsim-BKGx0.root')

# print contents of the DataStore before loading Particles
printDataStore()

# load all final state Particles
loadReconstructedParticles()

# create and fill gamma/e/mu/pi/K/p ParticleLists
# second argument are the selection criteria: [''] means no cut, take all
fillParticleList('gamma:all', [''])
fillParticleList('e-:all', [''])
fillParticleList('mu-:all', [''])
fillParticleList('pi-:all', [''])
fillParticleList('K-:all', [''])
fillParticleList('anti-p-:all', [''])

# alternatively, we can create and fill final state Particle lists only
# with candidates that pass certain PID requirements
fillParticleList('gamma:highE', ['E 1.0:'])
fillParticleList('e+:good', ['eid 0.1:'])
fillParticleList('mu+:good', ['muid 0.1:'])
fillParticleList('pi+:good', ['piid 0.1:'])
fillParticleList('K+:good', ['Kid 0.1:'])
fillParticleList('p+:good', ['prid 0.1:'])

# print contents of the DataStore after loading Particles
printDataStore()

# print out the contents of each ParticleList
printList('gamma:all', False)
printList('gamma:highE', False)
printList('e-:all', False)
printList('e-:good', False)
printList('mu-:all', False)
printList('mu-:good', False)
printList('pi-:all', False)
printList('pi-:good', False)
printList('K-:all', False)
printList('K-:good', False)
printList('anti-p-:all', False)
printList('anti-p-:good', False)

# Process the events
process(analysis_main)

# print out the summary
print statistics
