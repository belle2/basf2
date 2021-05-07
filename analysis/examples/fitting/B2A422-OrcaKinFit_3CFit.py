#!/usr/bin/env python3

#############################################################
#
# This tutorial demonstrates how to perform 3C fit with
# the orcakinfit. In this example the following decay chain:
# Y4S ->  gamma Z0
#               |
#               +-> mu+ mu-
# is reconstructed. For the gamma, we just use the position
# imformation.
#
# Contributors: Torben Ferber (2017)
#
#############################################################

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

b2.set_log_level(b2.LogLevel.WARNING)

# Input file(s).
inputMdst(environmentType='default',
          filename=b2.find_file('darkphotonmumu_mdst.root', 'examples', False),
          path=mypath)

# Creates a list of photons
stdPhotons('loose', path=mypath)

# use standard final state particle lists for muons
fillParticleList('mu-:z0', 'chiProb > 0.001 and p > 1.0', path=mypath)

# reconstruct Z -> mu+ mu-
reconstructDecay('Z0:mm -> mu+:z0 mu-:z0', '', path=mypath)


pdg.add_particle('beam', 9000009, 999., 999., 0, 0)  # name, PDG, mass, width, charge, spin
reconstructDecay("beam:rec -> gamma:loose Z0:mm", "", path=mypath)
reconstructDecay("beam:kinfit -> gamma:loose Z0:mm", "", path=mypath)


# MC truth matching
matchMCTruth('beam:rec', path=mypath)
matchMCTruth('beam:kinfit', path=mypath)

# kinematic 3C hard fit
fitKinematic3C('beam:kinfit', path=mypath)

# Select variables that we want to store to ntuple

mugvars = vc.inv_mass + vc.kinematics + vc.mc_truth + vc.mc_kinematics + vc.momentum_uncertainty
z0vars = vc.inv_mass + vc.kinematics + vc.mc_kinematics + vc.mc_truth + \
    vu.create_aliases_for_selected(mugvars, 'beam -> ^gamma [^Z0 -> ^mu+ ^mu-]')

z0uvars = z0vars + \
    vu.create_aliases(['OrcaKinFitProb',
                       'OrcaKinFitChi2',
                       'OrcaKinFitErrorCode'], 'extraInfo({variable})', "")

# Saving variables to ntuple
output_file = 'B2A422-Orcakinfit_3CFit.root'
variablesToNtuple('beam:rec', z0vars,
                  filename=output_file, treename='Z0_mm_rec', path=mypath)
variablesToNtuple('beam:kinfit', z0uvars,
                  filename=output_file, treename='Z0_mm_kinfit', path=mypath)

# Process the events
b2.process(mypath)

# print out the summary
print(b2.statistics)
