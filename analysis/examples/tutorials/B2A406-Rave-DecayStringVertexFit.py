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

# Add 10 signal MC files (each containing 1000 generated events)
filelistSIG = \
    ['/hsm/belle2/bdata/MC/signal/cc2dstar/mcprod1405/BGx1/mc35_cc2dstar_BGx1_s00/cc2dstar_e0001r001*_s00_BGx1.mdst.root'
     ]

inputMdstList('MC5', filelistSIG)

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
toolsDST = ['EventMetaData', '^D*+']
toolsDST += ['InvMass[BeforeFit]', '^D*+ -> ^D0 pi+']
toolsDST += ['CMSKinematics', '^D*+']
toolsDST += ['Kinematics', 'D*+ -> [^D0 -> ^K- ^pi+] pi+']
toolsDST += ['Vertex', '^D*+ -> ^D0 pi+']
toolsDST += ['MCVertex', '^D*+ -> ^D0 pi+']
toolsDST += ['PID', 'D*+ -> [D0 -> ^K- ^pi+] ^pi+']
toolsDST += ['Track', 'D*+ -> [D0 -> ^K- ^pi+] ^pi+']
toolsDST += ['MCTruth', '^D*+ -> ^D0 ^pi+']
toolsDST += ['MCFlightInfo', '^D*+ -> [^D0 -> K- pi+] pi+']
toolsDST += ['FlightInfo', '^D*+ -> [^D0 -> K- pi+] pi+']

# create flat Ntuple for D:st (single track vertex fit)
toolsDSTst = ['EventMetaData', '^D0']
toolsDSTst = ['Kinematics', '^D0 -> ^K- ^pi+']
toolsDSTst += ['Vertex', '^D0']
toolsDSTst += ['MCVertex', '^D0']
toolsDSTst += ['FlightInfo', '^D0 -> K- pi+']

# create flat Ntuple for D:du (daughters update)
toolsDSTdu = ['Kinematics', '^D0 -> ^K- ^pi+']

# write out the flat ntuples
ntupleFile('B2A406-Rave-DecayStringVertexFit.root')
ntupleTree('dsttree1', 'D*+:1', toolsDST)
ntupleTree('dsttree2', 'D*+:2', toolsDST)
ntupleTree('dsttree3', 'D*+:3', toolsDST)
ntupleTree('d0tree1', 'D0:st', toolsDSTst)
ntupleTree('d0tree2', 'D0:du', toolsDSTdu)


# Process the events
process(analysis_main)

# print out the summary
print(statistics)
