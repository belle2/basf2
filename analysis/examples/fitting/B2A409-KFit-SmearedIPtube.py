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
# This tutorial demonstrates how to perform Vertex fits                  #
# using KFit with smeared IP tube constraint.                            #
# The following  decay chain (and c.c. decay chain):                     #
#                                                                        #
# B0 -> J/psi K_S0                                                       #
#        |                                                               #
#        +-> mu- mu+                                                     #
#                                                                        #
# is reconstructed and the B0 decay vertex is fitted                     #
#                                                                        #
# Note: This example is build upon                                       #
# B2A403-KFit-VertexFit.py                                               #
#                                                                        #
##########################################################################

import basf2 as b2
from modularAnalysis import inputMdst
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from vertex import kFit
from stdCharged import stdMu
from stdV0s import stdKshorts
from modularAnalysis import variablesToNtuple
import variables.collections as vc

# create path
my_path = b2.create_path()

# load input ROOT file
inputMdst(environmentType='default',
          filename=b2.find_file('B02JpsiKs_Jpsi2mumu_Ks2pipi.root', 'examples', False),
          path=my_path)

# creates "mu+:all" ParticleList (and c.c.)
stdMu('all', path=my_path)
# creates "K_S0:merged" ParticleList
stdKshorts(path=my_path)

# reconstruct J/psi -> mu- mu+ decay
reconstructDecay('J/psi:mm -> mu-:all mu+:all', cut='3.05<M<3.15', path=my_path)

# reconstruct B0 -> J/psi K_S0 decay
# create two lists for comparison
reconstructDecay('B0:iptube -> J/psi:mm K_S0:merged', cut='5.27<Mbc<5.29 and abs(deltaE)<0.1', path=my_path)
reconstructDecay('B0:iptube20um -> J/psi:mm K_S0:merged', cut='5.27<Mbc<5.29 and abs(deltaE)<0.1', path=my_path)

# perform B0 vertex fit using only muons from J/psi
# fit one list with iptube constraint, the other with smeared iptube constraint.
kFit('B0:iptube', 0.0, 'vertex', constraint='iptube', decay_string='B0 -> [J/psi -> ^mu- ^mu+] K_S0', smearing=0.0, path=my_path)
kFit(
    'B0:iptube20um',
    0.0,
    'vertex',
    constraint='iptube',
    decay_string='B0 -> [J/psi -> ^mu- ^mu+] K_S0',
    smearing=0.002,
    path=my_path)

# perform MC matching (MC truth asociation)
matchMCTruth('B0:iptube', path=my_path)
matchMCTruth('B0:iptube20um', path=my_path)

# Select variables that we want to store to ntuple
B0_vars = vc.mc_truth + vc.vertex + vc.mc_vertex

# Saving variables to ntuple
output_file = 'B2A409-KFit-SmearedIPtube.root'
variablesToNtuple('B0:iptube', B0_vars,
                  filename=output_file, treename='B0tree_noSmear', path=my_path)
variablesToNtuple('B0:iptube20um', B0_vars,
                  filename=output_file, treename='B0tree_smear20um', path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
