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
#               I. Komarov (September 2018)
#
################################################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu
import vertex as vx
import stdCharged as stdc
import variables as va
from stdPi0s import stdPi0s

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B2pi0D_D2hh_D2hhh_B2munu.root', 'examples', False),
             path=my_path)

# use standard final state particle lists
#
# creates "pi+:all" ParticleList (and c.c.)
stdc.stdPi('all', path=my_path)
# rank all pions of the event by momentum magnitude
# variable stored to extraInfo as pi_p_rank
ma.rankByLowest(particleList='pi+:all',
                variable='p',
                outputVariable='pi_p_rank',
                path=my_path)

va.variables.addAlias('pi_p_rank', 'extraInfo(pi_p_rank)')

# creates "K+:loose" ParticleList (and c.c.)
stdc.stdK(listtype='loose', path=my_path)

# keep only candidates with 1.8 < M(Kpi) < 1.9 GeV
ma.reconstructDecay(decayString='D0 -> K-:loose pi+:all',
                    cut='1.8 < M < 1.9',
                    path=my_path)

# perform D0 vertex fit
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vx.vertexTree(list_name='D0',
              conf_level=-1,  # keep all cadidates, 0:keep only fit survivors, optimise this cut for your need
              ipConstraint=True,
              # pins the B0 PRODUCTION vertex to the IP (increases SIG and BKG rejection) use for better vertex resolution
              updateAllDaughters=True,  # update momenta off ALL particles
              path=my_path
              )

# smaller |M_rec - M| is better, add here a different output variable name, due to parentheses
ma.rankByLowest(particleList='D0',
                variable='abs(dM)',
                outputVariable='abs_dM_rank',
                path=my_path)

# maybe not the best idea, but might cut away candidates with failed fits
ma.rankByHighest(particleList='D0',
                 variable='chiProb',
                 path=my_path)

# Now let's do mixed ranking:
# First, we want to rank D candiadtes by the momentum of the pions
# Second, we want to rank those D candidates that were built with the highest-p by the vertex Chi2
# This doesn't have any sense, but shows how to work with consequetive rankings
#
# Let's add alias for the momentum rank of pions in D
va.variables.addAlias('D1_pi_p_rank', 'daughter(1,pi_p_rank)')
# Ranking D candidates by this variable.
# Candidates built with the same pion get the same rank (allowMultiRank=True).
ma.rankByHighest(particleList='D0',
                 variable='D1_pi_p_rank',
                 allowMultiRank=True,
                 outputVariable="first_D_rank",
                 path=my_path)
va.variables.addAlias('first_D_rank', 'extraInfo(first_D_rank)')
# Now let's rank by chiPrhob only those candiadtes that are built with the highest momentum pi
# Other canidadites will get this rank equal to -1
ma.rankByHighest(particleList="D0",
                 variable="chiProb",
                 cut="first_D_rank == 1",
                 outputVariable="second_D_rank",
                 path=my_path)
va.variables.addAlias('second_D_rank', 'extraInfo(second_D_rank)')


# add rank variable aliases for easier use
va.variables.addAlias('dM_rank', 'extraInfo(abs_dM_rank)')
va.variables.addAlias('chiProb_rank', 'extraInfo(chiProb_rank)')

# perform MC matching (MC truth asociation)
ma.matchMCTruth(list_name='D0', path=my_path)


# Select variables that we want to store to ntuple
fs_hadron_vars = vu.create_aliases_for_selected(list_of_variables=vc.mc_truth, decay_string='D0 -> ^K- ^pi+')

d0_vars = vc.vertex + \
    vc.mc_vertex + \
    vc.mc_truth + \
    fs_hadron_vars + \
    ['dM', 'chiProb', 'dM_rank', 'chiProb_rank', 'D1_pi_p_rank', 'first_D_rank', 'second_D_rank']


# Saving variables to ntuple
output_file = 'B2A602-BestCandidateSelection.root'
ma.variablesToNtuple(decayString='D0',
                     variables=d0_vars,
                     filename=output_file,
                     treename='D0',
                     path=my_path)


# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
