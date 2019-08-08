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
#               I. Komarov (September 2018)
#
################################################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu
import stdV0s as stdv

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B2pi0D_D2hh_D2hhh_B2munu.root', 'examples', False),
             path=my_path)

# print contents of the DataStore before loading Particles
ma.printDataStore(path=my_path)

# create and fill Ks/Lambda0 ParticleLists, using V0s as source
# second argument are the selection criteria: '' means no cut, take all
# the decay chain has to be specified (i.e. the two daughters, as well)
# A vertex fit should also be performed
# In this example a cut on the candidates mass is applied
ma.fillParticleList(decayString='K_S0:V0 -> pi+ pi-', cut='0.3 < M < 0.7', path=my_path)
ma.vertexKFit(list_name='K_S0:V0', conf_level=0.0, path=my_path)
ma.fillParticleList(decayString='Lambda0:V0 -> p+ pi-', cut='0.9 < M < 1.3', path=my_path)
ma.vertexKFit(list_name='Lambda0:V0', conf_level=0.0, path=my_path)

# alternatively, we can create a list of particles combined
# using the analysis ParticleCombiner module
# Before doing it we need to create and fill ParticleLists
# of the V0s' daughters (anti-particle lists are automatically
# created, too)
# A vertex fit should also be performed
# In this example a cut on the candidates mass is applied
ma.fillParticleList(decayString='pi-:all', cut='', path=my_path)
ma.fillParticleList(decayString='p+:all', cut='', path=my_path)

ma.reconstructDecay(decayString='K_S0:RD -> pi+:all pi-:all', cut='0.3 < M < 0.7', path=my_path)
ma.vertexKFit(list_name='K_S0:RD', conf_level=0.0, path=my_path)
ma.reconstructDecay(decayString='Lambda0:RD -> p+:all pi-:all', cut='0.9 < M < 1.3', path=my_path)
ma.vertexKFit(list_name='Lambda0:RD', conf_level=0.0, path=my_path)

# another possibility is to use default functions
# for V0s they are defined in analysis/scripts/stdV0s.py
# e.g. stdKshorts():
# - takes all V0 candidates, performs vertex fit, and fills 'K_S0:all' ParticleList
#   a cut on the candidates mass is applied, too
stdv.stdKshorts(path=my_path)
stdv.stdLambdas(path=my_path)

# print contents of the DataStore after loading Particles
ma.printDataStore(path=my_path)

# print out the contents of each ParticleList
ma.printList(list_name='K_S0:V0', full=False, path=my_path)
ma.printList(list_name='Lambda0:V0', full=False, path=my_path)

ma.printList(list_name='K_S0:RD', full=False, path=my_path)
ma.printList(list_name='Lambda0:RD', full=False, path=my_path)

ma.printList(list_name='K_S0:all', full=False, path=my_path)
ma.printList(list_name='Lambda0:all', full=False, path=my_path)

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
rootOutputFile = 'B2A203-LoadV0s.root'

# K_S0 from V0s
ma.variablesToNtuple(treename='kshort_v0',
                     decayString='K_S0:V0',
                     variables=v0_vars,
                     filename=rootOutputFile,
                     path=my_path)

# K_S0 from reconstructDecay
ma.variablesToNtuple(treename='kshort_rd',
                     decayString='K_S0:RD',
                     variables=v0_vars,
                     filename=rootOutputFile,
                     path=my_path)

# K_S0 from standard list (=V0s, see stdV0s.py)
ma.variablesToNtuple(treename='kshort_std',
                     decayString='K_S0:all',
                     variables=v0_vars,
                     filename=rootOutputFile,
                     path=my_path)

# Lambda0 from V0s
ma.variablesToNtuple(treename='lambda_v0',
                     decayString='Lambda0:V0',
                     variables=v0_vars,
                     filename=rootOutputFile,
                     path=my_path)

# Lambda0 from reconstructDecay
ma.variablesToNtuple(treename='lambda_rd',
                     decayString='Lambda0:RD',
                     variables=v0_vars,
                     filename=rootOutputFile,
                     path=my_path)

# Lambda0 from standard list (=V0s, see stdV0s.py)
ma.variablesToNtuple(treename='lambda_std',
                     decayString='Lambda0:all',
                     variables=v0_vars,
                     filename=rootOutputFile,
                     path=my_path)


b2.process(my_path)

# print out the summary
print(b2.statistics)
