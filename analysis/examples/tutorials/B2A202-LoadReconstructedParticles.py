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


# Select variables that we want to store to ntuple
# You can either use preselected variable groups from variableCollections:
import variableCollections as vc
# Or use your own lists. Both options are shown here.

charged_particle_variables = vc.reco_stats + \
    vc.event_meta_data + \
    vc.kinematics + \
    vc.track + \
    track_hits + \
    vc.pid + \
    vc.mc_truth + \
    mc_vc.kinematics + \
    vc.mc_hierarchy


gamma_variables = vc.kinematics + \
    mc_vc.kinematics + \
    vc.cluster

K0s_variables = vc.event_meta_data + \
    vc.kinematics + \
    vc.inv_mass + \
    vc.vertex + \
    mc_vc.vertex + \
    vc.pid + \
    vc.mc_truth + \
    vc.mc_hierarchy + \
    ['dr', 'dz', 'isSignal', 'chiProb']

pi0_variables = vc.mc_truth + \
    vc.kinematics + \
    vc.mass_before_fit + \
    vc.event_meta_data + \
    ['extraInfo(BDT)', 'decayAngle(0)'] + \
    vc.mc_hierarchy

# Saving variables to ntuple
from modularAnalysis import variablesToNtuple
output_file = 'B2A202-LoadReconstructedParticles.root'
variablesToNtuple('pi+:all', charged_particle_variables, treename='pion', filename=output_file)
variablesToNtuple('K+:all', charged_particle_variables, treename='kaon', filename=output_file)
variablesToNtuple('e+:all', charged_particle_variables, treename='elec', filename=output_file)
variablesToNtuple('mu+:all', charged_particle_variables, treename='muon', filename=output_file)
variablesToNtuple('gamma:all', gamma_variables, treename='phot', filename=output_file)

# Note here, that since we want to get info about gammas from pi0,
# we convert names of te variables from the gamma list in the way that they will
# correspond to given gammas.
variablesToNtuple('pi0:looseFit',
                  pi0_variables + vc.convert_to_all_selected_vars(gamma_variables, 'pi0 -> ^gamma ^gamma'),
                  filename=output_file, treename='pi0')

# Here for pions from K0s we do the same thing, but here we add custom aliases
# (see ntuples to see the difference)
variablesToNtuple('K_S0:all', K0s_variables +
                  vc.convert_to_one_selected_vars(charged_particle_variables, 'K_S0 -> ^pi+ pi-', 'pip') +
                  vc.convert_to_one_selected_vars(charged_particle_variables, 'K_S0 -> pi+ ^pi-', 'pim'),
                  filename=output_file, treename='kshort')

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
