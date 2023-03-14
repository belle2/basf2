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
# This tutorial demonstrates how to perform 1C fit with                  #
# the orcakinfit. In this example the following decay chain:             #
# Y4S ->  gamma Z0                                                       #
#               |                                                        #
#               +-> mu+ mu-                                              #
# with missing gamma is reconstructed.                                   #
#                                                                        #
##########################################################################

import basf2 as b2
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from kinfit import UnmeasuredfitKinematic1C
from modularAnalysis import variablesToNtuple
import variables.collections as vc
import variables.utils as vu

# create path
mypath = b2.create_path()

b2.set_log_level(b2.LogLevel.WARNING)

# Input file(s).
inputMdst(filename=b2.find_file('darkphotonmumu_mdst.root', 'examples', False),
          path=mypath)

# use standard final state particle lists for muons
fillParticleList('mu-:z0', 'chiProb > 0.001 and p > 1.0', path=mypath)
fillParticleList('mu-:z0fit', 'chiProb > 0.001 and p > 1.0', path=mypath)

# reconstruct Z -> mu+ mu-
reconstructDecay('Z0:mm_rec -> mu+:z0 mu-:z0', '9.0 < M < 11.0', path=mypath)
reconstructDecay('Z0:mm_kinfit -> mu+:z0fit mu-:z0fit', '9.0 < M < 11.0', path=mypath)

# MC truth matching
matchMCTruth('Z0:mm_rec', path=mypath)
matchMCTruth('Z0:mm_kinfit', path=mypath)

# kinematic 1C hard fit
UnmeasuredfitKinematic1C('Z0:mm_kinfit', path=mypath)

# Select variables that we want to store to ntuple

muvars = vc.kinematics + vc.mc_truth + vc.mc_kinematics + vc.momentum_uncertainty
z0vars = vc.inv_mass + vc.kinematics + vc.mc_kinematics + vc.mc_truth + \
    vu.create_aliases_for_selected(muvars, 'Z0 -> ^mu+ ^mu-')

z0uvars = z0vars + \
    ['OrcaKinFitProb',
     'OrcaKinFitChi2',
     'OrcaKinFitErrorCode',
     'OrcaKinFitUnmeasuredTheta',
     'OrcaKinFitUnmeasuredPhi',
     'OrcaKinFitUnmeasuredE'
     ]

# Saving variables to ntuple
output_file = 'B2A421-Orcakinfit_1CFit.root'
variablesToNtuple('Z0:mm_rec', z0vars,
                  filename=output_file, treename='Z0_mm_rec', path=mypath)
variablesToNtuple('Z0:mm_kinfit', z0uvars,
                  filename=output_file, treename='Z0_mm_kinfit', path=mypath)


# Process the events
b2.process(mypath)

# print out the summary
print(b2.statistics)
