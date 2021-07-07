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
# using Rave and how to use TagV and save Delta t.                       #
# The following  decay chain:                                            #
#                                                                        #
# B0 -> J/psi Ks                                                         #
#        |    |                                                          #
#        |    +-> pi+ pi-                                                #
#        |                                                               #
#        +-> mu+ mu-                                                     #
#                                                                        #
# is reconstructed B0 vertex is fitted using the two m                   #
# the side vertex is fitted and Delta t (in ps.) is                      #
# calculated                                                             #
#                                                                        #
##########################################################################

import basf2 as b2
from modularAnalysis import inputMdst
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from vertex import raveFit
from modularAnalysis import buildRestOfEvent
from modularAnalysis import fillParticleList
from vertex import TagV
from stdCharged import stdMu
import variables.collections as vc
import variables.utils as vu
from modularAnalysis import variablesToNtuple

# Add signal MC files for release 9.
# 150000 events!
# Consider using -n flag in command line to limit number of events, e.g.:
# > basf2 B2A410-TagVertex.py -n 1000

# create path
my_path = b2.create_path()

# load input ROOT file
inputMdst(environmentType='default',
          filename=b2.find_file('B02JpsiKs_Jpsi2mumu_Ks2pipi.root', 'examples', False),
          path=my_path)

# use standard final state particle lists
#
# creates "mu+:loose" ParticleList (and c.c.)
stdMu('loose', path=my_path)

# create Ks -> pi+ pi- list from V0
# keep only candidates with 0.4 < M(pipi) < 0.6 GeV
fillParticleList('K_S0:pipi -> pi+ pi-', '0.4 < M < 0.6', path=my_path)

# reconstruct J/psi -> mu+ mu- decay
# keep only candidates with 3.0 < M(mumu) < 3.2 GeV
reconstructDecay('J/psi:mumu -> mu+:loose mu-:loose', '3.0 < M < 3.2', path=my_path)

# reconstruct B0 -> J/psi Ks decay
# keep only candidates with 5.2 < M(J/PsiKs) < 5.4 GeV
reconstructDecay('B0:jpsiks -> J/psi:mumu K_S0:pipi', '5.2 < M < 5.4', path=my_path)

# perform MC matching (MC truth association). Always before TagV
matchMCTruth('B0:jpsiks', path=my_path)

# perform B0 kinematic vertex fit using only the mu+ mu-
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
raveFit('B0:jpsiks', 0.0, decay_string='B0 -> [J/psi -> ^mu+ ^mu-] K_S0', path=my_path)

# build the rest of the event associated to the B0
buildRestOfEvent('B0:jpsiks', path=my_path)

# calculate the Tag Vertex and Delta t (in ps)
# breco: type of MC association.
TagV('B0:jpsiks', 'breco', path=my_path)

# Select variables that we want to store to ntuple

fshars = vc.pid + vc.track + vc.mc_truth
jpsiandk0svars = vc.inv_mass + vc.vertex + vc.mc_vertex + vc.mc_truth
bvars = vc.inv_mass + vc.deltae_mbc + \
    vc.vertex + vc.mc_vertex + vc.mc_truth + vc.tag_vertex + vc.tag_vertex + \
    vc.mc_tag_vertex + \
    vu.create_aliases_for_selected(fshars, 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]') + \
    vu.create_aliases_for_selected(jpsiandk0svars, 'B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]')


# Saving variables to ntuple
output_file = 'B2A410-TagVertex.root'
variablesToNtuple('B0:jpsiks', bvars,
                  filename=output_file, treename='B0tree', path=my_path)


# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
