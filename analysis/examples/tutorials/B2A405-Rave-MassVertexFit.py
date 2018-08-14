#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to perform mass constraine
# Vertex fits using Rave. The following  decay chain
# (and c.c. decay chain):
#
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
# Contributors: L. Li Gioi (October 2014)
#               I. Komarov (Demeber 2017)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import vc.vertexRave
from modularAnalysis import massVertexRave
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
reconstructDecay('D0:kpi_mass -> K-:loose pi+:loose', '1.8 < M < 1.9')

# perform D0 vc.vertex fit
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vc.vertexRave('D0:kpi', 0.0)

# perform mass constrained D0 vc.vertex fit
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
massVertexRave('D0:kpi_mass', 0.0)

# reconstruct two D*+ -> D0 pi+ decay
# keep only candidates with Q = M(D0pi) - M(D0) - M(pi) < 20 MeV
# and D* CMS momentum > 2.5 GeV
reconstructDecay('D*+:1 -> D0:kpi pi+:all',
                 '0.0 <= Q < 0.02 and 2.5 < useCMSFrame(p) < 5.5')
reconstructDecay('D*+:2 -> D0:kpi_mass pi+:all',
                 '0.0 <= Q < 0.02 and 2.5 < useCMSFrame(p) < 5.5')

# perform D*+ kinematic vc.vertex fit using the D0 and the pi+
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vc.vertexRave('D*+:1', 0.0)

# perform D*+ kinematic beam spot constrined vc.vertex fit using the D0 and the pi+
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vc.vertexRave('D*+:2', 0.0, '', 'ipprofile')

# perform MC matching (MC truth asociation)
matchMCTruth('D*+:1')
matchMCTruth('D*+:2')

# Select variables that we want to store to ntuple
import variableCollections as vc

dstar_vars = vc.event_meta_data + vc.inv_mass + ckm_vc.kinematics + vc.mc_truth + \
    mc_vc.flight_info + vc.flight_info

fs_hadron_vars = vc.convert_to_all_selected_vars(
    vc.pid + vc.track + vc.mc_truth,
    'D*+ -> [D0 -> ^K- ^pi+] ^pi+')

d0_vars = vc.convert_to_one_selected_vars(
    vc.inv_mass + vc.mc_truth,
    'D*+ -> ^D0 pi+', 'D0')


# Saving variables to ntuple
from modularAnalysis import variablesToNtuple
output_file = 'B2A405-Rave-MassVertexFit.root'
variablesToNtuple('D*+:1', dstar_vars + d0_vars + fs_hadron_vars,
                  filename=output_file, treename='dsttree1')
variablesToNtuple('D*+:2', dstar_vars + d0_vars + fs_hadron_vars,
                  filename=output_file, treename='dsttree2')


# Process the events
process(analysis_main)

# print out the summary
print(statistics)
