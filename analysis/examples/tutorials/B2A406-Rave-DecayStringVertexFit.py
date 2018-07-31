#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to perform Vertex fits
# using Rave. The following  decay chain (and c.c. decay
# chain):
#
# D*+ -> D0 pi+
#        |
#        +-> K- pi+
#
# is reconstructed and the D0 and D*+ decay vertices are
# fitted.
#
# Note: This example is build upon
# B2A301-Dstar2D0Pi-Reconstruction.py
#
# Note: This example uses the signal MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
#
# Contributors: L. Li Gioi (October 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import vertexRave
from modularAnalysis import massVertexRave
from modularAnalysis import vertexRaveDaughtersUpdate
from stdCharged import *


# Add MC9 signal samples
filelistSIG = [('/group/belle2/tutorial/release_01-00-00/\
mdst_000001_prod00002171_task00000001.root')]

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
reconstructDecay('D0:st -> K-:loose pi+:loose', '1.8 < M < 1.9')
reconstructDecay('D0:du -> K-:loose pi+:loose', '1.8 < M < 1.9')

# perform D0 vertex fit
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
massVertexRave('D0:kpi', 0.0)

# perform D0 single track fit (production vertex)
# D0 vertex and covariance matrix must be defined
vertexRave('D0:st', 0.0)
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vertexRave('D0:st', 0.0, '^D0 -> K- pi+', 'ipprofile')

# perform D0 vertex fit updating daughters
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vertexRaveDaughtersUpdate('D0:du', 0.0)

# reconstruct 3 times the D*+ -> D0 pi+ decay
# keep only candidates with Q = M(D0pi) - M(D0) - M(pi) < 20 MeV
# and D* CMS momentum > 2.5 GeV
reconstructDecay('D*+:1 -> D0:kpi pi+:all',
                 '0.0 <= Q < 0.02 and 2.5 < useCMSFrame(p) < 5.5')
reconstructDecay('D*+:2 -> D0:kpi pi+:all',
                 '0.0 <= Q < 0.02 and 2.5 < useCMSFrame(p) < 5.5')
reconstructDecay('D*+:3 -> D0:kpi pi+:all',
                 '0.0 <= Q < 0.02 and 2.5 < useCMSFrame(p) < 5.5')

# perform D*+ kinematic vertex fit using the D0 and the pi+
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vertexRave('D*+:1', 0.0)

# perform D*+ kinematic beam spot constrined vertex fit using the D0 and the pi+
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vertexRave('D*+:2', 0.0, '', 'ipprofile')

# perform D*+ kinematic beam spot constrined vertex fit using only the pi+
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vertexRave('D*+:3', 0.0, 'D*+ -> D0 ^pi+', 'ipprofile')

# perform MC matching (MC truth asociation)
matchMCTruth('D*+:1')
matchMCTruth('D*+:2')
matchMCTruth('D*+:3')

# create and fill flat Ntuple with MCTruth, kinematic information and D0 FlightInfo
from groups_of_varuables import event_variables, kinematic_variables, cluster_variables, \
    track_variables, mc_variables, pid_variables, convert_to_daughter_vars, convert_to_gd_vars,\
    flight_info, mc_flight_info, vertex, mc_vertex

charged_particle_variables = kinematic_variables + track_variables + mc_variables + pid_variables

from modularAnalysis import variablesToNTuple
output_file = 'B2A406-Rave-DecayStringVertexFit.root'
variablesToNTuple(filename=output_file,
                  decayString='D*+:1',
                  treename='dsttree1',
                  ['chiProb'] + vertex + mc_vertex + event_variables +
                  kinematic_variables + mc_variables + flight_info + mc_flight_info +
                  convert_to_daughter_vars(charged_particle_variables + mc_variables, 1) +
                  convert_to_daughter_vars(vertex + mc_vertex + flight_info + mc_flight_info +
                                           kinematic_variables + mc_variables, 0) +
                  convert_to_gd_vars(charged_particle_variables, 0, 0) +
                  convert_to_gd_vars(charged_particle_variables, 0, 1))
variablesToNTuple(filename=output_file,
                  decayString='D*+:2',
                  treename='dsttree2',
                  ['chiProb'] + vertex + mc_vertex + event_variables +
                  kinematic_variables + mc_variables + flight_info + mc_flight_info +
                  convert_to_daughter_vars(charged_particle_variables + mc_variables, 1) +
                  convert_to_daughter_vars(vertex + mc_vertex + flight_info + mc_flight_info +
                                           kinematic_variables + mc_variables, 0) +
                  convert_to_gd_vars(charged_particle_variables, 0, 0) +
                  convert_to_gd_vars(charged_particle_variables, 0, 1))
variablesToNTuple(filename=output_file,
                  decayString='D*+:3',
                  treename='dsttree3',
                  ['chiProb'] + vertex + mc_vertex + event_variables +
                  kinematic_variables + mc_variables + flight_info + mc_flight_info +
                  convert_to_daughter_vars(charged_particle_variables + mc_variables, 1) +
                  convert_to_daughter_vars(vertex + mc_vertex + flight_info + mc_flight_info +
                                           kinematic_variables + mc_variables, 0) +
                  convert_to_gd_vars(charged_particle_variables, 0, 0) +
                  convert_to_gd_vars(charged_particle_variables, 0, 1))
variablesToNTuple(filename=output_file,
                  decayString='D0:st',
                  treename='d0tree1',
                  ['chiProb'] + vertex + mc_vertex + event_variables +
                  kinematic_variables + mc_variables + flight_info +
                  convert_to_daughter_vars(kinematic_variables, 0) +
                  convert_to_daughter_vars(kinematic_variables, 1))
variablesToNTuple(filename=output_file,
                  decayString='D0:du',
                  treename='d0tree2',
                  kinematic_variables +
                  convert_to_daughter_vars(kinematic_variables, 0) +
                  convert_to_daughter_vars(kinematic_variables, 1))

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
