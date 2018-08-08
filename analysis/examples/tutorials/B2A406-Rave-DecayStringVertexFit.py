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

# Select variables that we want to store to ntuple
from variableCollections import *

dstar_vars = event_meta_data + inv_mass + ckm_kinematics + mc_truth + \
    mc_flight_info + flight_info + vertex

fs_hadron_vars = convert_to_all_selected_vars(
    pid + track + mc_truth,
    'D*+ -> [D0 -> ^K- ^pi+] ^pi+')

d0_vars = convert_to_one_selected_vars(
    inv_mass + mc_truth + vertex,
    'D*+ -> ^D0 pi+', 'D0')

dstt = event_meta_data + kinematics + vertex + mc_vertex + flight_info + \
    convert_to_all_selected_vars(
        kinematics,
        '^D0 -> ^K- ^pi+')

dstu = kinematics + convert_to_all_selected_vars(
    kinematics,
    '^D0 -> ^K- ^pi+')

# Saving variables to ntuple
from modularAnalysis import variablesToNtuple
output_file = 'B2A406-Rave-DecayStringVertexFit.root'
variablesToNtuple('D*+:1', dstar_vars + d0_vars + fs_hadron_vars,
                  filename=output_file, treename='dsttree1')
variablesToNtuple('D*+:2', dstar_vars + d0_vars + fs_hadron_vars,
                  filename=output_file, treename='dsttree2')
variablesToNtuple('D*+:3', dstar_vars + d0_vars + fs_hadron_vars,
                  filename=output_file, treename='dsttree3')
variablesToNtuple('D0:st', dstt,
                  filename=output_file, treename='d0tree1')
variablesToNtuple('D0:du', dstu,
                  filename=output_file, treename='d0tree2')


# Process the events
process(analysis_main)

# print out the summary
print(statistics)
