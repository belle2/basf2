#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to perform four momentum constraint
# fit with the KFit. In this example the following decay chain:
#
# Upsilon(4S) -> eta               Upsilon
#                 |                  |
#                 +->gamma gamma     +-> u+ u-
#
# is reconstructed. The four momentum constraint fit is performed on
# all final states, and the total four momentum is set at that of cms
#
# Contributors: Yu Hu (March 2017)
# huyu@ihep.ac.cn
#
####################################################################


#
# Import and mdst loading
#

from basf2 import *
from modularAnalysis import *
from stdPhotons import *

# load input ROOT file
filelistSIG = '/group/belle2/tutorial/release_01-00-00/mdst-Y4setaY.root'

inputMdst('default', filelistSIG)

# Creates a list of good pions and kaons with some PID and IP cut
stdPhotons('loose')
fillParticleList('mu+:pid', 'muonID>0.1')


# Reconstructs eta -> gamma gamma
reconstructDecay("eta:gg -> gamma:loose gamma:loose", "")
# Reconstructs Upsilon -> u+ u-
reconstructDecay("Upsilon:uu -> mu+:pid mu-:pid", "M>2.")

# Reconstructs Upsilon(4S) -> Upsilon eta
reconstructDecay("Upsilon(4S) -> eta:gg Upsilon:uu", "")

# Perform four momentum constraint fit using KFit
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
fourCKFit("Upsilon(4S)", 0.0)

# Perform four momentum constraint fit using KFit and update the Daughters
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
# fourCKFitDaughtersUpdate("Upsilon(4S)", 0.0)

# Associates the MC truth to the reconstructed D0
matchMCTruth('Upsilon(4S)')

# here you can put whatever you like....
toolsD0 = ['EventMetaData', '^Upsilon(4S)']
toolsD0 += ['InvMass', '^Upsilon(4S) -> ^eta ^Upsilon']
toolsD0 += ['Kinematics', '^Upsilon(4S) -> [^eta -> ^gamma ^gamma] [^Upsilon -> ^mu+ ^mu-]']
toolsD0 += ['PID', 'Upsilon(4S) -> eta [Upsilon -> ^mu+ ^mu-]']
toolsD0 += ['MCTruth', '^Upsilon(4S) -> [^eta -> ^gamma ^gamma] [^Upsilon -> ^mu+ ^mu-]']
toolsD0 += ['MCHierarchy', '^Upsilon(4S) -> ^eta ^Upsilon']

# ntupleFile('B2A407-KFit-FourCFit_updateDaughters.root')
ntupleFile('B2A407-KFit-FourCFit.root')
ntupleTree('Upsilon4s', 'Upsilon(4S)', toolsD0)

#
# Process and print statistics
#

# Process the events
process(analysis_main)
# print out the summary
print(statistics)
