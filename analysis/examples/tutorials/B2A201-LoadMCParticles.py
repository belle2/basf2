#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to load generated final
# state particles (MCParticle objects) as Particles and
# create ParticleLists for each final state particle
# type: gamma/e/mu/pi/K/proton/K_L.
#
# All analysis modules and tools (e.g. making combinations, ...)
# have interfaces for ParticleLists so this step is
# neccessary if analysis tools are to be used.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdst
from modularAnalysis import printDataStore
from modularAnalysis import printList
from modularAnalysis import fillParticleListsFromMC
from modularAnalysis import analysis_main

# check if the required input file exists (from B2A101 example)
import os.path
import sys
if not os.path.isfile('B2A101-Y4SEventGeneration-evtgen.root'):
    sys.exit('Required input file (B2A101-Y4SEventGeneration-evtgen.root) does not exist. '
             'Please run B2A101-Y4SEventGeneration.py tutorial script first.')

# load input ROOT file
inputMdst('None', 'B2A101-Y4SEventGeneration-evtgen.root')

# print contents of the DataStore before loading MCParticles
printDataStore()

# create and fill gamma/e/mu/pi/K/p ParticleLists
# second argument are the selection criteria: '' means no cut, take all
photons = ('gamma:gen', '')
electrons = ('e-:gen', '')
muons = ('mu-:gen', '')
pions = ('pi-:gen', '')
kaons = ('K-:gen', '')
protons = ('anti-p-:gen', '')

fillParticleListsFromMC([photons, electrons, muons, pions, kaons, protons])

# print contents of the DataStore after loading MCParticles
# the difference is that DataStore now contains StoreArray<Particle>
# filled with Particles created from generated final state particles
printDataStore()

# print out the contents of each ParticleList
printList('gamma:gen', False)
printList('e-:gen', False)
printList('mu-:gen', False)
printList('pi-:gen', False)
printList('K-:gen', False)
printList('anti-p-:gen', False)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
