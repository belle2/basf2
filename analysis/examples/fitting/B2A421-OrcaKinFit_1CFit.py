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
# e+e- ->  gamma A                                                       #
#                |                                                       #
#                +-> mu+ mu-                                             #
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
import pdg

# create path
mypath = b2.create_path()

# Input file(s).
inputMdst(filename=b2.find_file('darkphotonmumu_mdst.root', 'examples', False),
          path=mypath)

# use standard final state particle lists for muons
fillParticleList('mu-', 'chiProb > 0.001 and p > 1.0', path=mypath)
fillParticleList('mu-:kinfit', 'chiProb > 0.001 and p > 1.0', path=mypath)

# reconstruct A -> mu+ mu-
pdg.add_particle('A', 9000008, 999., 999., 0, 0)  # name, PDG, mass, width, charge, spin
reconstructDecay('A:mm_rec -> mu+ mu-', '', path=mypath)
reconstructDecay('A:mm_kinfit -> mu+:kinfit mu-:kinfit', '', path=mypath)

# MC truth matching
matchMCTruth('A:mm_rec', path=mypath)
matchMCTruth('A:mm_kinfit', path=mypath)

# kinematic 1C hard fit
UnmeasuredfitKinematic1C('A:mm_kinfit', path=mypath)

# Select variables that we want to store to ntuple

muvars = vc.kinematics + vc.mc_truth + vc.mc_kinematics + vc.momentum_uncertainty
Avars = vc.inv_mass + vc.kinematics + vc.mc_kinematics + vc.mc_truth + \
    vu.create_aliases_for_selected(muvars, 'A -> ^mu+ ^mu-')

Auvars = Avars + \
    ['OrcaKinFitProb',
     'OrcaKinFitChi2',
     'OrcaKinFitErrorCode',
     'OrcaKinFitUnmeasuredTheta',
     'OrcaKinFitUnmeasuredPhi',
     'OrcaKinFitUnmeasuredE'
     ]

# Saving variables to ntuple
output_file = 'B2A421-Orcakinfit_1CFit.root'
variablesToNtuple('A:mm_rec', Avars,
                  filename=output_file, treename='A_mm_rec', path=mypath)
variablesToNtuple('A:mm_kinfit', Auvars,
                  filename=output_file, treename='A_mm_kinfit', path=mypath)


# Process the events
b2.process(mypath)
