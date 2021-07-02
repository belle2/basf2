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
# This tutorial demonstrates how to write out the                        #
# decay hash. This allows to store information on                        #
# the reconstructed and original decay and use it                        #
# in the offline analysis steps.                                         #
#                                                                        #
#   J/psi                                                                #
#    |                                                                   #
#    +-> e- e+                                                           #
#                                                                        #
# Notes:                                                                 #
#  1) No guarantee for collisions!                                       #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('JPsi2ee_e2egamma.root', 'examples', False),
             path=my_path)

# reconstruct the decay
ma.fillParticleList(decayString='e+',
                    cut='electronID > 0.2 and d0 < 2 and abs(z0) < 4',
                    writeOut=False,
                    path=my_path)
ma.fillParticleList(decayString='gamma',
                    cut='',
                    writeOut=False,
                    path=my_path)
ma.reconstructDecay(decayString='J/psi -> e+ e-',
                    cut='',
                    path=my_path)

# generate the decay string
my_path.add_module('ParticleMCDecayString', listName='J/psi', fileName='hashmap_Jpsi_from_B2A502.root')


# write out ntuples
var = ['M',
       'p',
       'E',
       'x', 'y', 'z',
       'extraInfo(DecayHash)',
       'extraInfo(DecayHashExtended)',
       ]

ma.variablesToNtuple(decayString='J/psi',
                     variables=var,
                     filename='Jpsi_from_B2A502.root',
                     path=my_path)

# process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
