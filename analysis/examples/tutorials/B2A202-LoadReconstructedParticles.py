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
#               I.Komarov(Sep 2018)
#
######################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu
from stdV0s import stdKshorts
from stdPi0s import stdPi0s

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B2pi0D_D2hh_D2hhh_B2munu.root', 'examples', False),
             path=my_path)

# print contents of the DataStore before loading Particles
ma.printDataStore()

# create and fill gamma/e/mu/pi/K/p ParticleLists
# second argument are the selection criteria: '' means no cut, take all
ma.fillParticleList(decayString='gamma:all', cut='', path=my_path)
ma.fillParticleList(decayString='e-:all', cut='', path=my_path)
ma.fillParticleList(decayString='mu-:all', cut='', path=my_path)
ma.fillParticleList(decayString='pi-:all', cut='', path=my_path)
ma.fillParticleList(decayString='K-:all', cut='', path=my_path)
ma.fillParticleList(decayString='anti-p-:all', cut='', path=my_path)

# alternatively, we can create and fill final state Particle lists only
# with candidates that pass certain PID requirements
ma.fillParticleList(decayString='gamma:highE', cut='E > 1.0', path=my_path)
ma.fillParticleList(decayString='e+:good', cut='electronID > 0.1', path=my_path)
ma.fillParticleList(decayString='mu+:good', cut='muonID > 0.1', path=my_path)
ma.fillParticleList(decayString='pi+:good', cut='protonID > 0.1', path=my_path)
ma.fillParticleList(decayString='K+:good', cut='kaonID > 0.1', path=my_path)
ma.fillParticleList(decayString='p+:good', cut='protonID > 0.1', path=my_path)

# another possibility is to use default functions
# for example stdKshorts() from stdV0s.py that:
# - takes all V0 candidates, performs vertex fit, and fills 'K_S0:all' ParticleList
# (-> for more details about V0s have a look at B2A203-LoadV0s.py)
# or for example stdPi0s() from stdPi0s.py:
stdKshorts(path=my_path)
stdPi0s(listtype='looseFit', path=my_path)

# print contents of the DataStore after loading Particles
ma.printDataStore()

# print out the contents of each ParticleList
ma.printList('gamma:all', False, path=my_path)
ma.printList('gamma:highE', False, path=my_path)
ma.printList('e-:all', False, path=my_path)
ma.printList('e-:good', False, path=my_path)
ma.printList('mu-:all', False, path=my_path)
ma.printList('mu-:good', False, path=my_path)
ma.printList('pi-:all', False, path=my_path)
ma.printList('pi-:good', False, path=my_path)
ma.printList('K-:all', False, path=my_path)
ma.printList('K-:good', False, path=my_path)
ma.printList('anti-p-:all', False, path=my_path)
ma.printList('anti-p-:good', False, path=my_path)
ma.printList('K_S0:all', False, path=my_path)
ma.printList('pi0:looseFit', False, path=my_path)


# Select variables that we want to store to ntuple
# You can either use preselected variable groups from variableCollections:
# Or use your own lists. Both options are shown here.

charged_particle_variables = vc.reco_stats + \
    vc.kinematics + \
    vc.track + \
    vc.track_hits + \
    vc.pid + \
    vc.mc_truth + \
    vc.mc_kinematics

gamma_variables = vc.kinematics + \
    vc.mc_kinematics + \
    vc.cluster

K0s_variables = vc.kinematics + \
    vc.inv_mass + \
    vc.vertex + \
    vc.mc_vertex + \
    vc.pid + \
    vc.mc_truth + \
    ['dr', 'dz', 'isSignal', 'chiProb']

pi0_variables = vc.mc_truth + \
    vc.kinematics + \
    ['extraInfo(BDT)', 'decayAngle(0)']

# Saving variables to ntuple
output_file = 'B2A202-LoadReconstructedParticles.root'
ma.variablesToNtuple(decayString='pi+:all',
                     variables=charged_particle_variables,
                     treename='pion',
                     filename=output_file,
                     path=my_path)
ma.variablesToNtuple(decayString='K+:all',
                     variables=charged_particle_variables,
                     treename='kaon',
                     filename=output_file,
                     path=my_path)
ma.variablesToNtuple(decayString='e+:all',
                     variables=charged_particle_variables,
                     treename='elec',
                     filename=output_file,
                     path=my_path)
ma.variablesToNtuple(decayString='mu+:all',
                     variables=charged_particle_variables,
                     treename='muon',
                     filename=output_file,
                     path=my_path)
ma.variablesToNtuple(decayString='gamma:all',
                     variables=gamma_variables,
                     treename='phot',
                     filename=output_file,
                     path=my_path)

# Note here, that since we want to get info about gammas from pi0,
# we convert names of te variables from the gamma list in the way that they will
# correspond to given gammas.
ma.variablesToNtuple(decayString='pi0:looseFit',
                     variables=pi0_variables + vu.create_aliases_for_selected(gamma_variables, 'pi0 -> ^gamma ^gamma'),
                     filename=output_file,
                     treename='pi0',
                     path=my_path)

# Here for pions from K0s we do the same thing, but here we add custom aliases
# (see ntuples to see the difference)
ma.variablesToNtuple(decayString='K_S0:all',
                     variables=K0s_variables +
                     vu.create_aliases_for_selected(charged_particle_variables, 'K_S0 -> ^pi+ pi-', 'pip') +
                     vu.create_aliases_for_selected(charged_particle_variables, 'K_S0 -> pi+ ^pi-', 'pim'),
                     filename=output_file,
                     treename='kshort',
                     path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
