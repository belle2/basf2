#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
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
#               Vishal (Oct2017) "Intermediate" option in MCHierarchy for Ks,pi0
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

from stdV0s import stdKshorts
from stdPi0s import *
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

# create and fill gamma/e/mu/pi/K/p ParticleLists
# second argument are the selection criteria: '' means no cut, take all
fillParticleList('gamma:all', '')
fillParticleList('e-:all', '')
fillParticleList('mu-:all', '')
fillParticleList('pi-:all', '')
fillParticleList('K-:all', '')
fillParticleList('anti-p-:all', '')

# alternatively, we can create and fill final state Particle lists only
# with candidates that pass certain PID requirements
fillParticleList('gamma:highE', 'E > 1.0')
fillParticleList('e+:good', 'electronID > 0.1')
fillParticleList('mu+:good', 'muonID > 0.1')
fillParticleList('pi+:good', 'protonID > 0.1')
fillParticleList('K+:good', 'kaonID > 0.1')
fillParticleList('p+:good', 'protonID > 0.1')

# another possibility is to use default functions
# for example stdKshorts() from stdV0s.py that:
# - takes all V0 candidates, performs vertex fit, and fills 'K_S0:all' ParticleList
# or for example stdPi0s() from stdPi0s.py:
stdKshorts()
stdPi0s('looseFit')

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
printList('K_S0:all', False)
printList('pi0:looseFit', False)


from variableCollections import event_variables, kinematic_variables, cluster_variables, \
    track_variables, pid_variables, convert_to_daughter_vars

gamma_variables = kinematic_variables + cluster_variables
charged_particle_variables = kinematic_variables + cluster_variables + track_variables + pid_variables


from modularAnalysis import variablesToNTuple
output_file = 'B2A202-LoadReconstructedParticles.root'
variablesToNTuple(filename=output_file, decayString='pi+:all', treename='pion',
                  event_variables +
                  charged_particle_variables)
variablesToNTuple(filename=output_file, decayString='K+:all', treename='kaon',
                  event_variables +
                  charged_particle_variables)
variablesToNTuple(filename=output_file, decayString='e+:all', treename='elec',
                  event_variables +
                  charged_particle_variables)
variablesToNTuple(filename=output_file, decayString='mu+:all', treename='muon',
                  event_variables +
                  charged_particle_variables)
variablesToNTuple(filename=output_file, decayString='gamma:all', treename='phot',
                  event_variables +
                  gamma_variables)
variablesToNTuple(filename=output_file, decayString='pi0:looseFit', treename='pi0',
                  event_variables +
                  kinematic_variables +
                  convert_to_daughter_vars(gamma_variables, 0) +
                  convert_to_daughter_vars(gamma_variables, 1))
variablesToNTuple(filename=output_file, decayString='K_S0:all', treename='kshort',
                  event_variables +
                  kinematic_variables +
                  convert_to_daughter_vars(charged_particle_variables, 0) +
                  convert_to_daughter_vars(charged_particle_variables, 1))

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
