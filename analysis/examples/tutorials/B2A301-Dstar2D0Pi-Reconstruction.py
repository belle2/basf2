#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to reconstruct the
# following  decay chain (and c.c. decay chain):
#
# D*+ -> D0 pi+
#        |
#        +-> K- pi+
#
#
# Note: This example uses the signal MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from stdLooseFSParticles import stdVeryLoosePi
from stdLooseFSParticles import stdLoosePi
from stdLooseFSParticles import stdLooseK

# Add 10 signal MC files (each containing 1000 generated events)
filelistSIG = \
    ['/hsm/belle2/bdata/MC/signal/cc2dstar/mcprod1405/BGx1/mc35_cc2dstar_BGx1_s00/cc2dstar_e0001r001*_s00_BGx1.mdst.root'
     ]

inputMdstList(filelistSIG)

# use standard final state particle lists
#
# creates "pi+:all" ParticleList (and c.c.)
stdVeryLoosePi()
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

# create and fill flat Ntuple with MCTruth and kinematic information
toolsDST = ['EventMetaData', '^D*+']
toolsDST += ['InvMass', '^D*+ -> ^D0 pi+']
toolsDST += ['CMSKinematics', '^D*+']
toolsDST += ['PID', 'D*+ -> [D0 -> ^K- ^pi+] ^pi+']
toolsDST += ['Track', 'D*+ -> [D0 -> ^K- ^pi+] ^pi+']
toolsDST += ['MCTruth', '^D*+ -> ^D0 ^pi+']

# write out the flat ntuple
ntupleFile('B2A301-Dstar2D0Pi-Reconstruction.root')
ntupleTree('dsttree', 'D*+', toolsDST)

# Process the events
process(analysis_main)

# print out the summary
print statistics
