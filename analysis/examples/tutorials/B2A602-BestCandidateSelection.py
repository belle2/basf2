#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
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
# ntuple->Scan("D0__dM:D0__chiProb:D0__absdM_rank:D0__chiProb_rank:D0_mcStatus")
#
#
# Note: This example uses the signal MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers,
# or by specifying other input files via the -i argument to basf2.
#
# based on B2A403-KFit-VertexFit.py
#
# Contributors: C. Pulvermacher
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdLooseFSParticles import stdVeryLoosePi
from stdLooseFSParticles import stdLoosePi
from stdLooseFSParticles import stdLooseK

# Add 10 signal MC files (each containing 1000 generated events)
filelistSIG = \
    ['/group/belle2/MC/signal/cc2dstar/mcprod1405/BGx1/mc35_cc2dstar_BGx1_s00/cc2dstar_e0001r001*_s00_BGx1.mdst.root'
     ]

inputMdstList(filelistSIG)

# load all final state Particles
loadReconstructedParticles()

# use standard final state particle lists
#
# creates "pi+:all" ParticleList (and c.c.)
stdVeryLoosePi()
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
print statistics
