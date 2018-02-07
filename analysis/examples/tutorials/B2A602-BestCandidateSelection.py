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
# ntuple->Scan("D0__dM:D0__chiProb:D0__absdM_rank:D0__chiProb_rank:D0_mcErrors")
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
filelistSIG = [('/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002171\
/e0000/4S/r00000/ccbar/sub00/mdst_000001_prod00002171_task00000001.root')]


inputMdstList('default', filelistSIG)

# use standard final state particle lists
#
# creates "pi+:all" ParticleList (and c.c.)
stdPi('all')
# creates "K+:loose" ParticleList (and c.c.)
stdLooseK()

# keep only candidates with 1.8 < M(Kpi) < 1.9 GeV
reconstructDecay('D0 -> K-:loose pi+:all', '1.8 < M < 1.9')

# perform D0 vertex fit
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vertexKFit('D0', 0.0)

# smaller |M_rec - M| is better
rankByLowest('D0', 'abs(dM)')

# maybe not the best idea, but might cut away candidates with failed fits
rankByHighest('D0', 'chiProb')

# perform MC matching (MC truth asociation)
matchMCTruth('D0')


# create and fill flat Ntuple with MCTruth and kinematic information
toolsDST = ['EventMetaData', '^D0']
toolsDST += ['CMSKinematics', '^D0']
# save ranks and associated variables
toolsDST += ['CustomFloats[dM:chiProb:extraInfo(abs(dM)_rank):extraInfo(chiProb_rank)]', '^D0']
toolsDST += ['Vertex', '^D0']
toolsDST += ['MCVertex', '^D0']
toolsDST += ['MCTruth', '^D0 -> ^K- ^pi+']

# write out the flat ntuple
ntupleFile('B2A602-BestCandidateSelection.root')
ntupleTree('ntuple', 'D0', toolsDST)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
