#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial demonstrates how to perform four momentum constraint     #
# fit with the KFit. In this example the following decay chain:          #
#                                                                        #
# Upsilon(4S) -> eta               Upsilon                               #
#                 |                  |                                   #
#                 +->gamma gamma     +-> u+ u-                           #
#                                                                        #
# is reconstructed. The four momentum constraint fit is performed on     #
# all final states, and the total four momentum is set at that of cms    #
#                                                                        #
##########################################################################


#
# Import and mdst loading
#

import basf2 as b2
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import reconstructDecay
from vertex import kFit
from modularAnalysis import matchMCTruth
from modularAnalysis import variablesToNtuple
from stdPhotons import stdPhotons
import variables.collections as vc
import variables.utils as vu

# create path
my_path = b2.create_path()

# load input ROOT file
inputMdst(filename=b2.find_file('Y4SEventToetaY1S-evtgen_100.root', 'examples', False),
          path=my_path)


# Creates a list of good pions and kaons with some PID and IP cut
stdPhotons('loose', path=my_path)
fillParticleList('mu+:pid', 'muonID>0.1', path=my_path)


# Reconstructs eta -> gamma gamma
reconstructDecay("eta:gg -> gamma:loose gamma:loose", "", path=my_path)
# Reconstructs Upsilon -> u+ u-
reconstructDecay("Upsilon:uu -> mu+:pid mu-:pid", "M>2.", path=my_path)

# Reconstructs Upsilon(4S) -> Upsilon eta
reconstructDecay("Upsilon(4S) -> eta:gg Upsilon:uu", "", path=my_path)

# Associates the MC truth to the reconstructed particles
matchMCTruth('Upsilon(4S)', path=my_path)

# Perform four momentum constraint fit using KFit.
# Reject the candidates with failed fit.
kFit("Upsilon(4S)", 0.0, 'fourC', path=my_path)

# Perform four momentum constraint fit using KFit and update the Daughters
# Reject the candidates with failed fit.
# kFit("Upsilon(4S)", 0.0, 'fourC', daughtersUpdate=True, path=my_path)

# Select variables that we want to store to ntuple
muvars = vc.mc_truth + vc.pid + vc.kinematics
gvars = vc.kinematics + vc.mc_truth
etaanduvars = vc.inv_mass + vc.kinematics + vc.mc_truth
u4svars = vc.inv_mass + vc.kinematics + vc.mc_truth + \
    vu.create_aliases(['FourCFitProb', 'FourCFitChi2'], 'extraInfo({variable})', "") + \
    vu.create_aliases_for_selected(etaanduvars, 'Upsilon(4S) -> ^eta ^Upsilon') + \
    vu.create_aliases_for_selected(muvars, 'Upsilon(4S) -> eta [Upsilon -> ^mu+ ^mu-]') + \
    vu.create_aliases_for_selected(gvars, 'Upsilon(4S) -> [eta -> ^gamma ^gamma] Upsilon')

# Saving variables to ntuple
output_file = 'B2A407-KFit-FourCFit.root'
variablesToNtuple('Upsilon(4S)', u4svars,
                  filename=output_file, treename='Upsilon4s', path=my_path)

#
# Process and print statistics
#

# Process the events
b2.process(my_path)
# print out the summary
print(b2.statistics)
