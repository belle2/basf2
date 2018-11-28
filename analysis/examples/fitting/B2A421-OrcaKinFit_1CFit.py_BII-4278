#!/usr/bin/env python3
# -*- coding: utf-8 -*-

############################################################
#
# This tutorial demonstrates how to perform 1C fit with
# the orcakinfit. In this example the following decay chain:
# Y4S ->  gamma Z0
#               |
#               +-> mu+ mu-
# with missing gamma is reconstructed.
#
# Contributors: Torben Ferber (2017)
#
############################################################

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
import variables.collections as vc

set_log_level(LogLevel.WARNING)
# set_log_level(LogLevel.DEBUG)

# not necessary, but set it explicitly
beamparameters = add_beamparameters(analysis_main, "Y4S")
print_params(beamparameters)

# Input file(s).
filelistSIG = ['/gpfs/group/belle2/tutorial/orcakinfit/out-1.root']
inputMdstList('MC9', filelistSIG)

# use standard final state particle lists for muons
fillParticleList('mu-:z0', 'chiProb > 0.001 and p > 1.0')
fillParticleList('mu-:z0fit', 'chiProb > 0.001 and p > 1.0')

# reconstruct Z -> mu+ mu-
reconstructDecay('Z0:mm_rec -> mu+:z0 mu-:z0', '9.0 < M < 11.0')
reconstructDecay('Z0:mm_kinfit -> mu+:z0fit mu-:z0fit', '9.0 < M < 11.0')

# MC truth matching
matchMCTruth('Z0:mm_rec')
matchMCTruth('Z0:mm_kinfit')

# kinematic 1C hard fit
UnmeasuredfitKinematic1C('Z0:mm_kinfit')

# Select variables that we want to store to ntuple

muvars = vc.kinematics + vc.mc_truth + vc.mc_kinematics + vc.momentum_uncertainty
z0vars = vc.event_meta_data + vc.inv_mass + vc.kinematics + vc.mc_kinematics + vc.mc_truth + \
    vc.create_aliases_for_selected(muvars, 'Z0 -> ^mu+ ^mu-')

z0uvars = z0vars + \
    vc.create_aliases(['OrcaKinFitProb',
                       'OrcaKinFitChi2',
                       'OrcaKinFitErrorCode',
                       'OrcaKinFitUnmeasuredTheta',
                       'OrcaKinFitUnmeasuredPhi',
                       'OrcaKinFitUnmeasuredE'], 'extraInfo(variable)', "")

# Saving variables to ntuple
output_file = 'B2A421-Orcakinfit_1CFit.root'
variablesToNtuple('Z0:mm_rec', z0vars,
                  filename=output_file, treename='Z0_mm_rec')
variablesToNtuple('Z0:mm_kinfit', z0uvars,
                  filename=output_file, treename='Z0_mm_kinfit')


# Process the events
process(analysis_main)

# print out the summary
print(statistics)
