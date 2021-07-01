#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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

import basf2 as b2
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from kinfit import fitKinematic4C
from modularAnalysis import variablesToNtuple
import variables.collections as vc
import variables.utils as vu
from stdPhotons import stdPhotons

# create path
mypath = b2.create_path()

# load input ROOT file
inputMdst(environmentType='default',
          filename=b2.find_file('Y4SEventToetaY1S-evtgen_100.root', 'examples', False),
          path=mypath)

# Creates a list of good photon and mu
stdPhotons('loose', path=mypath)
fillParticleList('mu+:pid', 'chiProb > 0.001 and p > 1.0', path=mypath)

# Reconstructs eta -> gamma gamma
reconstructDecay("eta:gg -> gamma:loose gamma:loose", "", path=mypath)
# Reconstructs Upsilon -> u+ u-
reconstructDecay("Upsilon:uu -> mu+:pid mu-:pid", "M>2.", path=mypath)

# Reconstructs Upsilon(4S) -> Upsilon eta
reconstructDecay("Upsilon(4S) -> eta:gg Upsilon:uu", "", path=mypath)
reconstructDecay("Upsilon(4S):4c -> eta:gg Upsilon:uu", "", path=mypath)

# Associates the MC truth to the reconstructed Upsilon(4S)
matchMCTruth('Upsilon(4S)', path=mypath)
matchMCTruth('Upsilon(4S):4c', path=mypath)

# Perform four momentum constraint fit using OrcaKinFit
fitKinematic4C("Upsilon(4S):4c", path=mypath)

# Select variables that we want to store to ntuple
muvars = vc.mc_truth + vc.pid + vc.kinematics
gvars = vc.kinematics + vc.mc_truth + vc.inv_mass
etaanduvars = vc.inv_mass + vc.kinematics + vc.mc_truth
u4svars = vc.inv_mass + vc.kinematics + vc.mc_truth +\
    vu.create_aliases(['FourCFitProb', 'FourCFitChi2'], 'extraInfo({variable})', "") + \
    vu.create_aliases_for_selected(etaanduvars, 'Upsilon(4S) -> ^eta ^Upsilon') + \
    vu.create_aliases_for_selected(muvars, 'Upsilon(4S) -> eta [Upsilon -> ^mu+ ^mu-]') + \
    vu.create_aliases_for_selected(gvars, 'Upsilon(4S) -> [eta -> ^gamma ^gamma] Upsilon')

u4svars_4c = u4svars + vu.create_aliases(['OrcaKinFitProb',
                                          'OrcaKinFitChi2',
                                          'OrcaKinFitErrorCode'], 'extraInfo({variable})', "")

u4svars_def = u4svars + vu.create_aliases(['chiProb'], 'extraInfo({variable})', "")


# Saving variables to ntuple
output_file = 'B2A423-Orcakinfit_4CFit.root'
variablesToNtuple('Upsilon(4S)', u4svars_def,
                  filename=output_file, treename='Upsilon4s', path=mypath)
variablesToNtuple('Upsilon(4S):4c', u4svars_4c,
                  filename=output_file, treename='Upsilon4s_4c', path=mypath)


# Process the events
b2.process(mypath)
# print out the summary
print(b2.statistics)
