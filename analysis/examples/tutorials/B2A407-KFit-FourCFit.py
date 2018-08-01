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

# 6. Dump info to ntuple
from variableCollections import event_variables, kinematic_variables, cluster_variables, \
    track_variables, mc_variables, pid_variables, convert_to_daughter_vars, convert_to_gd_vars,\

from modularAnalysis import variablesToNTuple
rootOutputFile = 'B2A407-KFit-FourCFit.root'
variablesToNTuple(filename=rootOutputFile,
                  decayString='Upsilon(4S)',
                  treename='Upsilon4s',
                  event_variables + kinematic_variables + mc_variables +
                  convert_to_daughter_vars(kinematic_variables + mc_variables, 0) +
                  convert_to_daughter_vars(kinematic_variables + mc_variables, 1) +
                  convert_to_gd_vars(kinematic_variables + mc_variables, 0, 0) +
                  convert_to_gd_vars(kinematic_variables + mc_variables, 0, 1) +
                  convert_to_gd_vars(kinematic_variables + mc_variables + pid_variables, 1, 0) +
                  convert_to_gd_vars(kinematic_variables + mc_variables + pid_variables, 1, 1))


#
# Process and print statistics
#

# Process the events
process(analysis_main)
# print out the summary
print(statistics)
