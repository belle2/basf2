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
# This tutorial demonstrates how to load reconstructed                   #
# final state particles as Particles in case of V0-like particles:       #
# - K_S0                                                                 #
# - Lambda0                                                              #
# - (anti-Lambda0)                                                       #
#                                                                        #
# The recommended way to do this is to use the standard                  #
# lists provided in the script: analysis/scripts/stdV0s.py:              #
#                                                                        #
#    stdKshorts(prioritiseV0, fitter, path)                              #
#    stdLambdas(prioritiseV0, fitter, path)                              #
#                                                                        #
##########################################################################

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

# The recommended way to create and fill Ks/Lambda0 ParticleLists is to use
# the default functions provided in analysis/scripts/stdV0s.py
# These functions load a combined list of V0 objects merged with
# a list of particles combined using the analysis ParticleCombiner module.
# See the script (analysis/scripts/stdV0s.py) for more details.
stdv.stdKshorts(prioritiseV0=True, path=my_path)
stdv.stdLambdas(prioritiseV0=True, path=my_path)

# print contents of the DataStore after loading Particles
ma.printDataStore(path=my_path)

# print out the contents of each ParticleList
ma.printList(list_name='K_S0:merged', full=False, path=my_path)
ma.printList(list_name='Lambda0:merged', full=False, path=my_path)

# define variables
pi0_vars = vc.kinematics + \
    vc.pid +  \
    vc.track + \
    vc.track_hits + \
    vc.mc_truth

v0_vars = vc.kinematics + \
    vc.inv_mass + \
    vc.vertex + \
    vc.mc_truth + \
    ['chiProb'] + \
    vu.create_daughter_aliases(pi0_vars, 0) +\
    vu.create_daughter_aliases(pi0_vars, 1)


# saving variables to ntuple
rootOutputFile = 'B2A203-LoadV0s.root'

# K_S0 from standard list (merged list, see stdV0s.py)
ma.variablesToNtuple(treename='kshort_std',
                     decayString='K_S0:merged',
                     variables=v0_vars,
                     filename=rootOutputFile,
                     path=my_path)

# Lambda0 from standard list (merged list, see stdV0s.py)
ma.variablesToNtuple(treename='lambda_std',
                     decayString='Lambda0:merged',
                     variables=v0_vars,
                     filename=rootOutputFile,
                     path=my_path)


b2.process(my_path)

# print out the summary
print(b2.statistics)
