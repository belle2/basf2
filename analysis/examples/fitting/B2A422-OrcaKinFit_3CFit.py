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
# This tutorial demonstrates how to perform 3C fit with                  #
# the orcakinfit. In this example the following decay chain:             #
# Y4S ->  gamma Z0                                                       #
#               |                                                        #
#               +-> mu+ mu-                                              #
# is reconstructed. For the gamma, we just use the position              #
# information.                                                           #
#                                                                        #
##########################################################################

import basf2 as b2
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from kinfit import fitKinematic3C
from modularAnalysis import variablesToNtuple
import variables.collections as vc
import variables.utils as vu
from stdPhotons import stdPhotons
import pdg

# create path
mypath = b2.create_path()

# Input file(s).
inputMdst(filename=b2.find_file('darkphotonmumu_mdst.root', 'examples', False),
          path=mypath)

# Creates a list of photons
stdPhotons('loose', path=mypath)

# use standard final state particle lists for muons
fillParticleList('mu-:A', 'chiProb > 0.001 and p > 1.0', path=mypath)

# reconstruct A -> mu+ mu-
pdg.add_particle('A', 9000008, 999., 999., 0, 0)  # name, PDG, mass, width, charge, spin
reconstructDecay('A:mm -> mu+:A mu-:A', '', path=mypath)

pdg.add_particle('beam', 9000009, 999., 999., 0, 0)  # name, PDG, mass, width, charge, spin
reconstructDecay("beam:rec -> gamma:loose A:mm", "", path=mypath)
reconstructDecay("beam:kinfit -> gamma:loose A:mm", "", path=mypath)

# MC truth matching
matchMCTruth('beam:rec', path=mypath)
matchMCTruth('beam:kinfit', path=mypath)

# kinematic 3C hard fit
fitKinematic3C('beam:kinfit', path=mypath)

# Select variables that we want to store to ntuple

mugvars = vc.inv_mass + vc.kinematics + vc.mc_truth + vc.mc_kinematics + vc.momentum_uncertainty
Avars = vc.inv_mass + vc.kinematics + vc.mc_kinematics + vc.mc_truth + \
    vu.create_aliases_for_selected(mugvars, 'beam -> ^gamma [^A -> ^mu+ ^mu-]')

Auvars = Avars + ['OrcaKinFitProb', 'OrcaKinFitChi2', 'OrcaKinFitErrorCode']

# Saving variables to ntuple
output_file = 'B2A422-Orcakinfit_3CFit.root'
variablesToNtuple('beam:rec', Avars,
                  filename=output_file, treename='A_mm_rec', path=mypath)
variablesToNtuple('beam:kinfit', Auvars,
                  filename=output_file, treename='A_mm_kinfit', path=mypath)

# Process the events
b2.process(mypath, calculateStatistics=True)

# print out the summary
print(b2.statistics)
