#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import sys
import basf2 as b2
import modularAnalysis as ma
import stdV0s
import variables.collections as vc

# get input file number from the command line
filenumber = sys.argv[1]

# create path
main = b2.Path()

# load input data from mdst/udst file
ma.inputMdstList(
    environmentType="default",
    filelist=[b2.find_file(f"starterkit/2021/1111540100_eph3_BGx0_{filenumber}.root", "examples")],
    path=main,
)

# fill final state particle lists
ma.fillParticleList(
    "e+:uncorrected",
    "electronID > 0.1 and dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance",
    path=main,
)
stdV0s.stdKshorts(path=main)

# combine final state particles to form composite particles
ma.reconstructDecay(
    "J/psi:ee -> e+:uncorrected e-:uncorrected", cut="dM < 0.11", path=main
)

# combine J/psi and KS candidates to form B0 candidates
ma.reconstructDecay(
    "B0 -> J/psi:ee K_S0:merged",
    cut="Mbc > 5.2 and abs(deltaE) < 0.15",
    path=main,
)

# match reconstructed with MC particles
ma.matchMCTruth("B0", path=main)

# Create list of variables to save into the output file
b_vars = []

standard_vars = vc.kinematics + vc.mc_kinematics + vc.mc_truth
b_vars += vc.deltae_mbc
b_vars += standard_vars

# Save variables to an output file (ntuple)
ma.variablesToNtuple(
    "B0",
    variables=b_vars,
    filename="Bd2JpsiKS.root",
    treename="tree",
    path=main,
)

# Start the event loop (actually start processing things)
b2.process(main)

# print out the summary
print(b2.statistics)
