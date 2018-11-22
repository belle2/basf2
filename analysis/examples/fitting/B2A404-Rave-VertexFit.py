#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
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
from modularAnalysis import vertexRave
from stdCharged import stdPi, stdLoosePi, stdLooseK
from modularAnalysis import variablesToNtuple
import variableCollections as vc

# Add MC9 signal samples
filelistSIG = [('/group/belle2/tutorial/release_01-00-00/\
mdst_000001_prod00002171_task00000001.root')]


inputMdstList('default', filelistSIG)

# use standard final state particle lists
#
# creates "pi+:all" ParticleList (and c.c.)
stdPi('all')
# creates "pi+:loose" ParticleList (and c.c.)
stdPi('loose')
# creates "K+:loose" ParticleList (and c.c.)
stdK('loose')

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

# Select variables that we want to store to ntuple

dstar_vars = vc.event_meta_data + vc.inv_mass + vc.ckm_kinematics + vc.mc_truth + \
    vc.mc_flight_info + vc.flight_info

fs_hadron_vars = vc.create_aliases_for_selected(
    vc.pid + vc.track + vc.mc_truth,
    'D*+ -> [D0 -> ^K- ^pi+] ^pi+')

d0_vars = vc.create_aliases_for_selected(
    vc.inv_mass + vc.mc_truth,
    'D*+ -> ^D0 pi+', 'D0')


# Saving variables to ntuple
output_file = 'B2A404-Rave-VertexFit.root'
variablesToNtuple('D*+', dstar_vars + d0_vars + fs_hadron_vars,
                  filename=output_file, treename='dsttree')


# Process the events
process(analysis_main)

# print out the summary
print(statistics)
