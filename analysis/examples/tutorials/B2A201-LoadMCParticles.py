#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial demonstrates how to load generated final                 #
# state particles (MCParticle objects) as Particles and                  #
# create ParticleLists for each final state particle type:               #
# gamma/e/mu/pi/K/proton/K_L.                                            #
#                                                                        #
# All analysis modules and tools (e.g. making combinations, ...)         #
# have interfaces for ParticleLists so this step is                      #
# necessary if analysis tools are to be used.                            #
#                                                                        #
# NOTE: If you run into problems with the file                           #
# B2pi0D_D2hh_D2hhh_B2munu.root not being found, then basf2              #
# can't find your examples data dir (or you don't have it).              #
# Please take a look into the readme of /analysis/examples/              #
# for how to debug this.                                                 #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B2pi0D_D2hh_D2hhh_B2munu.root', 'examples', False),
             path=my_path)

# print contents of the DataStore before loading MCParticles
ma.printDataStore(path=my_path)

# create and fill gamma/e/mu/pi/K/p ParticleLists
# second argument are the selection criteria: '' means no cut, take all
photons = ('gamma:gen', '')
electrons = ('e-:gen', '')
muons = ('mu-:gen', '')
pions = ('pi-:gen', '')
kaons = ('K-:gen', '')
protons = ('anti-p-:gen', '')

ma.fillParticleListsFromMC([photons, electrons, muons, pions, kaons, protons], path=my_path)

# print contents of the DataStore after loading MCParticles
# the difference is that DataStore now contains StoreArray<Particle>
# filled with Particles created from generated final state particles
ma.printDataStore(path=my_path)

# print out the contents of each ParticleList
ma.printList(list_name='gamma:gen', full=False, path=my_path)
ma.printList(list_name='e-:gen', full=False, path=my_path)
ma.printList(list_name='mu-:gen', full=False, path=my_path)
ma.printList(list_name='pi-:gen', full=False, path=my_path)
ma.printList(list_name='K-:gen', full=False, path=my_path)
ma.printList(list_name='anti-p-:gen', full=False, path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
