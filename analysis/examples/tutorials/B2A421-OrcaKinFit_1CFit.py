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
from modularAnalysis import *
import sys
from beamparameters import add_beamparameters


set_log_level(LogLevel.WARNING)
# set_log_level(LogLevel.DEBUG)

# not necessary, but set it explicitly
beamparameters = add_beamparameters(analysis_main, "Y4S")
print_params(beamparameters)

# Input file(s).
filelistSIG = ['/gpfs/group/belle2/tutorial/orcakinfit/out-1.root']
inputMdstList('default', filelistSIG)

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

# create and fill flat Ntuple with MCTruth and kinematic information for z0 without no kinfit
toolsZ0 = ['EventMetaData', '^Z0']
toolsZ0 += ['InvMass', '^Z0 -> mu+ mu-']
toolsZ0 += ['Kinematics', '^Z0 -> ^mu+ ^mu-']
toolsZ0 += ['MCTruth', '^Z0 -> ^mu+ ^mu-']
toolsZ0 += ['MCKinematics', '^Z0 -> ^mu+ ^mu-']
toolsZ0 += ['MomentumUncertainty', 'Z0 -> ^mu+ ^mu-']

# create and fill flat Ntuple with MCTruth and kinematic information for z0 with kinfit
toolsZ0u = ['EventMetaData', '^Z0']
toolsZ0u += ['InvMass', '^Z0 -> mu+ mu-']
toolsZ0u += ['Kinematics', '^Z0 -> ^mu+ ^mu-']
toolsZ0u += ['MCTruth', '^Z0 -> ^mu+ ^mu-']
toolsZ0u += ['MCKinematics', '^Z0 -> ^mu+ ^mu-']
toolsZ0u += ['MomentumUncertainty', 'Z0 -> ^mu+ ^mu-']
toolsZ0u += ['CustomFloats[extraInfo(OrcaKinFitProb)]', '^Z0']
toolsZ0u += ['CustomFloats[extraInfo(OrcaKinFitChi2)]', '^Z0']
toolsZ0u += ['CustomFloats[extraInfo(OrcaKinFitErrorCode)]', '^Z0']
toolsZ0u += ['CustomFloats[extraInfo(OrcaKinFitUnmeasuredTheta)]', '^Z0']
toolsZ0u += ['CustomFloats[extraInfo(OrcaKinFitUnmeasuredPhi)]', '^Z0']
toolsZ0u += ['CustomFloats[extraInfo(OrcaKinFitUnmeasuredE)]', '^Z0']

# write out the flat ntuple
ntupleFile('B2A421-Orcakinfit_1CFit.root')
ntupleTree('Z0_mm_rec', 'Z0:mm_rec', toolsZ0)
ntupleTree('Z0_mm_kinfit', 'Z0:mm_kinfit', toolsZ0u)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
