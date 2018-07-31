#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to reconstruct the
# following  decay chain (and c.c. decay chain):
#
# D*+ -> D0 pi+
#        |
#        +-> K- pi+
#
# Contributors: A. Zupanc (June 2014)
#               I. Komarov (Demeber 2017)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from stdCharged import *


# Add 10 signal MC files (each containing 1000 generated events)
filelistSIG = \
    ['/group/belle2/tutorial/release_01-00-00/mdst-dstars.root'
     ]

inputMdstList('default', filelistSIG)

# use standard final state particle lists
#
# creates "pi+:all" ParticleList (and c.c.)
stdPi('all')
# creates "pi+:loose" ParticleList (and c.c.)
stdLoosePi()
# creates "K+:loose" ParticleList (and c.c.)
stdLooseK()

# reconstruct D0 -> K- pi+ decay
# keep only candidates with 1.8 < M(Kpi) < 1.9 GeV
reconstructDecay('D0:kpi -> K-:loose pi+:loose', '1.8 < M < 1.9')

# reconstruct D*+ -> D0 pi+ decay
# keep only candidates with Q = M(D0pi) - M(D0) - M(pi) < 20 MeV
# and D* CMS momentum > 2.5 GeV
reconstructDecay('D*+ -> D0:kpi pi+:all', '0.0 < Q < 0.020 and 2.5 < useCMSFrame(p) < 5.5')

# perform MC matching (MC truth asociation)
matchMCTruth('D*+')

from groups_of_varuables import event_variables, kinematic_variables, cluster_variables, \
    track_variables, mc_variables, pid_variables, convert_to_daughter_vars, convert_to_gd_vars

charged_particle_variables = kinematic_variables + track_variables + mc_variables + pid_variables

from modularAnalysis import variablesToNTuple
output_file = 'B2A301-Dstar2D0Pi-Reconstruction.root'
variablesToNTuple(filename=output_file,
                  decayString='D*+',
                  treename='dsttree',
                  event_variables +
                  kinematic_variables +
                  mc_variables +
                  convert_to_daughter_vars(charged_particle_variables, 1) +
                  convert_to_daughter_vars(kinematic_variables + mc_variables, 0) +
                  convert_to_gd_vars(charged_particle_variables, 0, 0) +
                  convert_to_gd_vars(charged_particle_variables, 0, 1))

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
