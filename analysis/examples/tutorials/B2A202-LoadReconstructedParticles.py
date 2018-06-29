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

# define Ntuple tools for charged Particles
toolsTrackPI = ['EventMetaData', 'pi+']
toolsTrackPI += ['RecoStats', 'pi+']
toolsTrackPI += ['Kinematics', '^pi+']
toolsTrackPI += ['Track', '^pi+']
toolsTrackPI += ['PID', '^pi+']
toolsTrackPI += ['MCTruth', '^pi+']
toolsTrackPI += ['MCKinematics', '^pi+']
toolsTrackPI += ['MCHierarchy', '^pi+']

toolsTrackK = ['EventMetaData', 'K+']
toolsTrackK += ['RecoStats', 'K+']
toolsTrackK += ['Kinematics', '^K+']
toolsTrackK += ['Track', '^K+']
toolsTrackK += ['PID', '^K+']
toolsTrackK += ['MCTruth', '^K+']
toolsTrackK += ['MCKinematics', '^K+']
toolsTrackK += ['MCHierarchy', '^K+']

toolsTrackE = ['EventMetaData', 'e+']
toolsTrackE += ['RecoStats', 'e+']
toolsTrackE += ['Kinematics', '^e+']
toolsTrackE += ['Track', '^e+']
toolsTrackE += ['PID', '^e+']
toolsTrackE += ['MCTruth', '^e+']
toolsTrackE += ['MCKinematics', '^e+']
toolsTrackE += ['MCHierarchy', '^e+']

toolsTrackMu = ['EventMetaData', 'mu+']
toolsTrackMu += ['RecoStats', 'mu+']
toolsTrackMu += ['Kinematics', '^mu+']
toolsTrackMu += ['Track', '^mu+']
toolsTrackMu += ['PID', '^mu+']
toolsTrackMu += ['MCTruth', '^mu+']
toolsTrackMu += ['MCKinematics', '^mu+']
toolsTrackMu += ['MCHierarchy', '^mu+']

toolsGamma = ['Kinematics', '^gamma']
toolsGamma += ['MCKinematics', '^gamma']
toolsGamma += ['MCTruth', '^gamma']
toolsGamma += ['Cluster', '^gamma']

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

toolsPI0 = ['MCTruth', '^pi0 -> gamma gamma']
toolsPI0 += ['Kinematics', '^pi0 -> ^gamma ^gamma']
toolsPI0 += ['MassBeforeFit', '^pi0']
toolsPI0 += ['EventMetaData', '^pi0']
toolsPI0 += ['Cluster', 'pi0 -> ^gamma ^gamma']
toolsPI0 += ['CustomFloats[extraInfo(BDT):decayAngle(0)]', '^pi0']
toolsPI0 += ['MCHierarchy[Intermediate]', '^pi0']

ntupleFile('B2A202-LoadReconstructedParticles.root')
ntupleTree('pion', 'pi+:all', toolsTrackPI)
ntupleTree('kaon', 'K+:all', toolsTrackK)
ntupleTree('elec', 'e+:all', toolsTrackE)
ntupleTree('muon', 'mu+:all', toolsTrackMu)
ntupleTree('phot', 'gamma:all', toolsGamma)
ntupleTree('pi0', 'pi0:looseFit', toolsPI0)
ntupleTree('kshort', 'K_S0:all', toolsK0)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
