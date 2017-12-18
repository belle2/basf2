#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Example to kinematically fit non-radiative muon pairs
#
# Contributors: Torben Ferber (2017)
#
######################################################

from basf2 import *
from modularAnalysis import *
import sys
from beamparameters import add_beamparameters

inputfile = sys.argv[1]
outputfile = sys.argv[2]

set_log_level(LogLevel.WARNING)
# set_log_level(LogLevel.DEBUG)

# not necessary, but set it explicitly
beamparameters = add_beamparameters(analysis_main, "Y4S")
print_params(beamparameters)

# Input file(s).
filelistSIG = [str(inputfile)]
inputMdstList('default', filelistSIG)

# use standard final state particle lists for muons
# stdMu('all')
fillParticleList('mu-:all', 'chiProb > 0.001 and p > 1.0')
fillParticleList('mu-:allup', 'chiProb > 0.001 and p > 1.0')

# reconstruct Z -> mu+ mu-
reconstructDecay('Z0:mm_rec -> mu+:all mu-:all', '9.0 < M < 11.0')
reconstructDecay('Z0:mm_kinfit -> mu+:allup mu-:allup', '9.0 < M < 11.0')

# MC truth matching
matchMCTruth('Z0:mm_rec')
matchMCTruth('Z0:mm_kinfit')

# kinematic 4C hard fit
# fitKinematic4C('Z0:mm_kinfit')
fitKinematic1CUnmeasured('Z0:mm_kinfit')

# create and fill flat Ntuple with MCTruth and kinematic information
toolsZ0 = ['EventMetaData', '^Z0']
toolsZ0 += ['InvMass', '^Z0 -> mu+ mu-']
toolsZ0 += ['Kinematics', '^Z0 -> ^mu+ ^mu-']
toolsZ0 += ['MCTruth', '^Z0 -> ^mu+ ^mu-']
toolsZ0 += ['MCKinematics', '^Z0 -> ^mu+ ^mu-']
toolsZ0 += ['MomentumUncertainty', 'Z0 -> ^mu+ ^mu-']

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
ntupleFile(str(outputfile))
ntupleTree('Z0_mm_rec', 'Z0:mm_rec', toolsZ0)
ntupleTree('Z0_mm_kinfit', 'Z0:mm_kinfit', toolsZ0u)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
