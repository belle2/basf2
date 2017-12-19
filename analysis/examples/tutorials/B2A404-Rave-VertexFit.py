#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
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
# Note: This example uses the signal MC sample created with
# release-01-00-00
#
# Contributors: L. Li Gioi (October 2014)
#               I. Komarov (December 2017)
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
from stdCharged import *

# Add 10 signal MC files (each containing 1000 generated events)
filelistSIG = \
    ['/hsm/belle/bdata2/users/ikomarov/tutorial_samples/mdst-dstars.root'
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

# perform D0 vertex fit
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vertexRave('D0:kpi', 0.0)

# reconstruct D*+ -> D0 pi+ decay
# keep only candidates with Q = M(D0pi) - M(D0) - M(pi) < 20 MeV
# and D* CMS momentum > 2.5 GeV
reconstructDecay('D*+ -> D0:kpi pi+:all',
                 '0.0 <= Q < 0.02 and 2.5 < useCMSFrame(p) < 5.5')

# perform D*+ kinematic vertex fit using the D0 and the pi+
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vertexRave('D*+', 0.0)

# perform MC matching (MC truth asociation)
matchMCTruth('D*+')

# create and fill flat Ntuple with MCTruth, kinematic information and D0 FlightInfo
toolsDST = ['EventMetaData', '^D*+']
toolsDST += ['InvMass[BeforeFit]', '^D*+ -> ^D0 pi+']
toolsDST += ['CMSKinematics', '^D*+']
toolsDST += ['Vertex', '^D*+ -> ^D0 pi+']
toolsDST += ['MCVertex', '^D*+ -> ^D0 pi+']
toolsDST += ['PID', 'D*+ -> [D0 -> ^K- ^pi+] ^pi+']
toolsDST += ['Track', 'D*+ -> [D0 -> ^K- ^pi+] ^pi+']
toolsDST += ['MCTruth', '^D*+ -> ^D0 ^pi+']
toolsDST += ['MCFlightInfo', '^D*+ -> [^D0 -> K- pi+] pi+']
toolsDST += ['FlightInfo', '^D*+ -> [^D0 -> K- pi+] pi+']

# write out the flat ntuples
ntupleFile('B2A404-Rave-VertexFit.root')
ntupleTree('dsttree', 'D*+', toolsDST)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
