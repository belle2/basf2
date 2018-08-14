#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to reconstruct the
# following  decay chain (and c.c. decay chain):
#
# D*+ -> D0 pi+
#        |
#        +-> K- pi+
#
# Contributors: A. Zupanc (June 2014)
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
from stdCharged import *
from beamparameters import add_beamparameters

beamparameters = add_beamparameters(analysis_main, "Y4S")

# Add 10 signal MC files (each containing 1000 generated events)
filelistSIG = \
    ['/group/belle2/tutorial/release_01-00-00/mdst-dstars.root'
     ]

inputMdstList('MC9', filelistSIG)

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

# reconstruct D*+ -> D0 pi+ decay
# keep only candidates with Q = M(D0pi) - M(D0) - M(pi) < 20 MeV
# and D* CMS momentum > 2.5 GeV
reconstructDecay('D*+ -> D0:kpi pi+:all', '0.0 < Q < 0.020 and 2.5 < useCMSFrame(p) < 5.5')

# perform MC matching (MC truth asociation)
matchMCTruth('D*+')

# Select variables that we want to store to ntuple
import variableCollections as vc

dstar_vars = vc.event_meta_data + vc.inv_mass + vc.ckm_kinematics + vc.mc_truth

fs_hadron_vars = vc.convert_to_all_selected_vars(
    vc.pid + vc.track + vc.mc_truth,
    'D*+ -> [D0 -> ^K- ^pi+] ^pi+')

d0_vars = vc.convert_to_one_selected_vars(
    vc.inv_mass + vc.mc_truth,
    'D*+ -> ^D0 pi+', 'D0')


# Saving variables to ntuple
from modularAnalysis import variablesToNtuple
output_file = 'B2A301-Dstar2D0Pi-Reconstruction.root'
variablesToNtuple('D*+', dstar_vars + d0_vars + fs_hadron_vars,
                  filename=output_file, treename='dsttree')

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
