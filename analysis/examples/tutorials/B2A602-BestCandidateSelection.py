#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial exemplifies how a best-candidate selection
# can be performed using rankByLowest()/rankByHighest() for
# different variables.
# The decay channel D0 -> K- pi+ (+ c.c.) is reconstructed,
# a vertex fit performed and variables dM and chiProb are then
# used to rank the candidates and saved via the CustomFloats
# ntuple tool.
#
# To look at the results, one might use:
# ntuple->Scan("D0_dM:D0_chiProb:D0_dM_rank:D0_chiProb_rank:D0_mcErrors")
#
#
# based on B2A403-KFit-VertexFit.py
#
# Contributors: C. Pulvermacher
#               I. Komarov (Demeber 2017)
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *

# Add MC9 signal samples
filelistSIG = [('/group/belle2/tutorial/release_01-00-00/\
mdst_000001_prod00002171_task00000001.root')]


inputMdstList('default', filelistSIG)

# use standard final state particle lists
#
# creates "pi+:all" ParticleList (and c.c.)
stdPi('all')
# rank all pions of the event by momentum magnitude
# variable stored to extraInfo as pi_p_rank
rankByLowest('pi+:all', 'p', outputVariable='pi_p_rank')
variables.addAlias('pi_p_rank', 'extraInfo(pi_p_rank)')
# creates "K+:loose" ParticleList (and c.c.)
stdLooseK()

# keep only candidates with 1.8 < M(Kpi) < 1.9 GeV
reconstructDecay('D0 -> K-:loose pi+:all', '1.8 < M < 1.9')

# perform D0 vertex fit
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vertexKFit('D0', 0.0)

# smaller |M_rec - M| is better, add here a different output variable name, due to parentheses
rankByLowest('D0', 'abs(dM)', outputVariable='abs_dM_rank')

# maybe not the best idea, but might cut away candidates with failed fits
rankByHighest('D0', 'chiProb')

# Now let's do mixed ranking:
# First, we want to rank D candiadtes by the momentum of the pions
# Second, we want to rank those D candidates that were built with the highest-p by the vertex Chi2
# This doesn't have any sense, but shows how to work with consequetive rankings
#
# Let's add alias for the momentum rank of pions in D
variables.addAlias('D1_pi_p_rank', 'daughter(1,pi_p_rank)')
# Ranking D candidates by this variable.
# Candidates built with the same pion get the same rank (allowMultiRank=True).
rankByHighest('D0', 'D1_pi_p_rank', allowMultiRank=True, outputVariable="first_D_rank")
variables.addAlias('first_D_rank', 'extraInfo(first_D_rank)')
# Now let's rank by chiPrhob only those candiadtes that are built with the highest momentum pi
# Other canidadites will get this rank equal to -1
rankByHighest("D0", "chiProb", cut="first_D_rank == 1", outputVariable="second_D_rank")
variables.addAlias('second_D_rank', 'extraInfo(second_D_rank)')


# add rank variable aliases for easier use
variables.addAlias('dM_rank', 'extraInfo(abs_dM_rank)')
variables.addAlias('chiProb_rank', 'extraInfo(chiProb_rank)')

# perform MC matching (MC truth asociation)
matchMCTruth('D0')

# create and fill flat Ntuple with MCTruth and kinematic information
from groups_of_varuables import event_variables, kinematic_variables, cluster_variables, \
    track_variables, mc_variables, pid_variables, convert_to_daughter_vars, convert_to_gd_vars,\
    flight_info, mc_flight_info, vertex, mc_vertex

charged_particle_variables = kinematic_variables + track_variables + mc_variables + pid_variables

from modularAnalysis import variablesToNTuple
output_file = 'B2A602-BestCandidateSelection.root'
variablesToNTuple(filename=output_file,
                  decayString='D0',
                  treename='ntuple',
                  ['dM', 'chiProb', 'dM_rank', 'chiProb_rank', 'D1_pi_p_rank', 'first_D_rank', 'second_D_rank'] +
                  event_variables + kinematic_variables + mc_variables + vertex + mc_vertex +
                  convert_to_daughter_vars(mc_variables, 1) +
                  convert_to_daughter_vars(mc_variables, 0))


# Process the events
process(analysis_main)

# print out the summary
print(statistics)
