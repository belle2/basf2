#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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
# See tutorials/B2A203-LoadV0s.py for the recommended way to load V0-like particles
#
# This is a more advanced example.
# It shows how to create and fill Ks/Lambda0 ParticleLists using V0s as source and
# create a list of particles combined using the analysis ParticleCombiner module.
#
# A combination of this two lists is what is implemented in the standard lists
# provided and recommended to the users (/analysis/scripts/sdtV0s.py)
#
# Contributors: B. Scavino (August 2019)
#
################################################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu
import vertex as vtx

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B2pi0D_D2hh_D2hhh_B2munu.root', 'examples', False),
             path=my_path)

# print contents of the DataStore before loading Particles
ma.printDataStore(path=my_path)

# this is an example of how create and fill Ks/Lambda0 ParticleLists, using V0s as source
# second argument are the selection criteria: '' means no cut, take all
# the decay chain has to be specified (i.e. the two daughters, as well)
# A vertex fit should also be performed
# In this example a cut on the candidates' mass is applied
ma.fillParticleList(decayString='K_S0:v0 -> pi+ pi-', cut='0.3 < M < 0.7', path=my_path)
vtx.treeFit(list_name='K_S0:v0', conf_level=0.0, path=my_path)
ma.fillParticleList(decayString='Lambda0:v0 -> p+ pi-', cut='0.9 < M < 1.3', path=my_path)
vtx.treeFit(list_name='Lambda0:v0', conf_level=0.0, path=my_path)

# alternatively, we can create a list of particles combined
# using the analysis ParticleCombiner module
# Before doing it we need to create and fill ParticleLists
# of the V0s' daughters (anti-particle lists are automatically
# created, too)
# A vertex fit should also be performed
# In this example a cut on the candidates' mass is applied
ma.fillParticleList(decayString='pi-:all', cut='', path=my_path)
ma.fillParticleList(decayString='p+:all', cut='', path=my_path)

ma.reconstructDecay(decayString='K_S0:RD -> pi+:all pi-:all', cut='0.3 < M < 0.7', path=my_path)
vtx.treeFit(list_name='K_S0:RD', conf_level=0.0, path=my_path)
ma.reconstructDecay(decayString='Lambda0:RD -> p+:all pi-:all', cut='0.9 < M < 1.3', path=my_path)
vtx.treeFit(list_name='Lambda0:RD', conf_level=0.0, path=my_path)

# print contents of the DataStore after loading Particles
ma.printDataStore(path=my_path)

# print out the contents of each ParticleList
ma.printList(list_name='K_S0:v0', full=False, path=my_path)
ma.printList(list_name='Lambda0:v0', full=False, path=my_path)

ma.printList(list_name='K_S0:RD', full=False, path=my_path)
ma.printList(list_name='Lambda0:RD', full=False, path=my_path)

# define variables
pi0_vars = vc.kinematics + \
    vc.pid +  \
    vc.track + \
    vc.track_hits + \
    vc.mc_truth

v0_vars = vc.kinematics + \
    vc.inv_mass + \
    vc.vertex +\
    vc.mc_truth + \
    ['chiProb'] +\
    vu.create_daughter_aliases(pi0_vars, 0) +\
    vu.create_daughter_aliases(pi0_vars, 1)


# saving variables to ntuple
rootOutputFile = 'B2A205-LoadV0s_expert.root'

# K_S0 from V0s
ma.variablesToNtuple(treename='kshort_v0',
                     decayString='K_S0:v0',
                     variables=v0_vars,
                     filename=rootOutputFile,
                     path=my_path)

# K_S0 from reconstructDecay
ma.variablesToNtuple(treename='kshort_rd',
                     decayString='K_S0:RD',
                     variables=v0_vars,
                     filename=rootOutputFile,
                     path=my_path)

# Lambda0 from V0s
ma.variablesToNtuple(treename='lambda_v0',
                     decayString='Lambda0:v0',
                     variables=v0_vars,
                     filename=rootOutputFile,
                     path=my_path)

# Lambda0 from reconstructDecay
ma.variablesToNtuple(treename='lambda_rd',
                     decayString='Lambda0:RD',
                     variables=v0_vars,
                     filename=rootOutputFile,
                     path=my_path)

b2.process(my_path)

# print out the summary
print(b2.statistics)
