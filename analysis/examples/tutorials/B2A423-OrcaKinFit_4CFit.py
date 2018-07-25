#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###################################################################
# This tutorial demonstrates how to perform four momentum constraint
# fit with the OrcaKinFit. In this example the following decay chain:
#
# Upsilon(4S) -> eta               Upsilon
#                 |                  |
#                 +->gamma gamma     +-> u+ u-
#
# is reconstructed. The four momentum constraint fit is performed on
# all final states, and the total four momentum is set at that of cms
#
# Contributors: Yu Hu (March 2017)
# yu.hu@desy.de
#
####################################################################


#
# Import and mdst loading
#

from basf2 import *
from modularAnalysis import *
from stdPhotons import *
import sys
from beamparameters import add_beamparameters

# load input ROOT file
inputMdst('default', '/gpfs/group/belle2/tutorial/orcakinfit/Y4SEventGeneration-gsim-BKGx0_eta_100.root')

# Creates a list of good photon and mu
stdPhotons('loose')
fillParticleList('mu+:pid', 'chiProb > 0.001 and p > 1.0')

# Reconstructs eta -> gamma gamma
reconstructDecay("eta:gg -> gamma:loose gamma:loose", "")
# Reconstructs Upsilon -> u+ u-
reconstructDecay("Upsilon:uu -> mu+:pid mu-:pid", "M>2.")

# Reconstructs Upsilon(4S) -> Upsilon eta
reconstructDecay("Upsilon(4S) -> eta:gg Upsilon:uu", "")
reconstructDecay("Upsilon(4S):4c -> eta:gg Upsilon:uu", "")


# Perform four momentum constraint fit using OrcaKinFit
fitKinematic4C("Upsilon(4S):4c")

# Associates the MC truth to the reconstructed Upsilon(4S)
matchMCTruth('Upsilon(4S)')
matchMCTruth('Upsilon(4S):4c')

# here you can put whatever you like....
toolsD04c = ['EventMetaData', '^Upsilon(4S):4c']
toolsD04c += ['InvMass', '^Upsilon(4S):4c -> ^eta ^Upsilon']
toolsD04c += ['Kinematics', '^Upsilon(4S):4c -> [^eta -> ^gamma ^gamma] [^Upsilon -> ^mu+ ^mu-]']
toolsD04c += ['PID', 'Upsilon(4S):4c -> eta [Upsilon -> ^mu+ ^mu-]']
toolsD04c += ['MCTruth', '^Upsilon(4S):4c -> [^eta -> ^gamma ^gamma] [^Upsilon -> ^mu+ ^mu-]']
toolsD04c += ['MCHierarchy', '^Upsilon(4S):4c -> ^eta ^Upsilon']
toolsD04c += ['CustomFloats[extraInfo(OrcaKinFitProb)]', '^Upsilon(4S):4c']
toolsD04c += ['CustomFloats[extraInfo(OrcaKinFitChi2)]', '^Upsilon(4S):4c']
toolsD04c += ['CustomFloats[extraInfo(OrcaKinFitErrorCode)]', '^Upsilon(4S):4c']

toolsD0 = ['EventMetaData', '^Upsilon(4S)']
toolsD0 += ['InvMass', '^Upsilon(4S) -> ^eta ^Upsilon']
toolsD0 += ['Kinematics', '^Upsilon(4S) -> [^eta -> ^gamma ^gamma] [^Upsilon -> ^mu+ ^mu-]']
toolsD0 += ['PID', 'Upsilon(4S) -> eta [Upsilon -> ^mu+ ^mu-]']
toolsD0 += ['MCTruth', '^Upsilon(4S) -> [^eta -> ^gamma ^gamma] [^Upsilon -> ^mu+ ^mu-]']
toolsD0 += ['MCHierarchy', '^Upsilon(4S) -> ^eta ^Upsilon']
toolsD0 += ['CustomFloats[chiProb]', '^Upsilon(4S)']

ntupleFile('B2A423-Orcakinfit_4CFit.root')
ntupleTree('Upsilon4s_4c', 'Upsilon(4S):4c', toolsD04c)
ntupleTree('Upsilon4s', 'Upsilon(4S)', toolsD0)

#
# Process and print statistics
#

# Process the events
process(analysis_main)
# print out the summary
print(statistics)
