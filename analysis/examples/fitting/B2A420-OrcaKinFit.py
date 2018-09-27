#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
#
# This tutorial demonstrates how to perform 4C fit with
# the orcakinfit. In this example the following decay chain:
#
# e+ e- ->  mu+ mu-
#
# is reconstructed. The 4C kinematic fit is performed on
# the non-radiative muon pairs
#
# Contributors: Torben Ferber (2017)
#
#############################################################

from basf2 import *
from modularAnalysis import add_beamparameters
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import fitKinematic4C
from modularAnalysis import variablesToNtuple
import sys
from beamparameters import add_beamparameters
import variableCollections as vc

if not os.path.isfile('B2A101-Y4SEventGeneration-evtgen.root'):
    sys.exit('Required input file (B2A424-SimulateMuonPairs.root) does not exist. '
             'Please run B2A424-SimulateMuonPairs.py tutorial script first.')

set_log_level(LogLevel.WARNING)

# not necessary, but set it explicitly
beamparameters = add_beamparameters(analysis_main, "Y4S")
print_params(beamparameters)

# Input file(s).
inputMdst('default', 'B2A424-SimulateMuonPairs.root')

# use standard final state particle lists for muons
fillParticleList('mu-:z0', 'chiProb > 0.001 and p > 1.0')
fillParticleList('mu-:z0fit', 'chiProb > 0.001 and p > 1.0')

# reconstruct Z -> mu+ mu-
reconstructDecay('Z0:mm_rec -> mu+:z0 mu-:z0', '9.0 < M < 11.0')
reconstructDecay('Z0:mm_kinfit -> mu+:z0fit mu-:z0fit', '9.0 < M < 11.0')

# MC truth matching
matchMCTruth('Z0:mm_rec')
matchMCTruth('Z0:mm_kinfit')

# kinematic 4C hard fit
fitKinematic4C('Z0:mm_kinfit')

# Select variables that we want to store to ntuple
muvars = vc.kinematics + vc.mc_truth + vc.mc_kinematics + vc.momentum_uncertainty
z0vars = vc.event_meta_data + vc.inv_mass + vc.kinematics + vc.mc_kinematics + vc.mc_truth + \
    vc.convert_to_all_selected_vars(muvars, 'Z0 -> ^mu+ ^mu-')

z0uvars = z0vars + \
    vc.wrap_list(['OrcaKinFitProb', 'OrcaKinFitChi2', 'OrcaKinFitErrorCode'], 'extraInfo(variable)', "")


# Saving variables to ntuple
output_file = 'B2A420-OrcaKinFit.root'
variablesToNtuple('Z0:mm_rec', z0vars,
                  filename=output_file, treename='Z0_mm_rec')
variablesToNtuple('Z0:mm_kinfit', z0uvars,
                  filename=output_file, treename='Z0_mm_kinfit')


# Process the events
process(analysis_main)

# print out the summary
print(statistics)
