#!/usr/bin/env python3

import sys
import basf2 as b2
import modularAnalysis as ma
import stdV0s
import variables.collections as vc
import variables.utils as vu

# get input file number from the command line
filenumber = sys.argv[1]

# create path
main = b2.Path()

# load input data from mdst/udst file
ma.inputMdstList(
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

# build the rest of the event [S10|S30|S40]
ma.buildRestOfEvent("B0", fillWithMostLikely=True, path=main)  # [E10]
track_based_cuts = "thetaInCDCAcceptance and pt > 0.075 and dr < 5 and abs(dz) < 10"
ecl_based_cuts = "thetaInCDCAcceptance and E > 0.05"  # [E30]
roe_mask = ("my_mask", track_based_cuts, ecl_based_cuts)
ma.appendROEMasks("B0", [roe_mask], path=main)  # [E40]

# Create list of variables to save into the output file
b_vars = []

standard_vars = vc.kinematics + vc.mc_kinematics + vc.mc_truth
b_vars += vc.deltae_mbc
b_vars += standard_vars

# ROE variables [S20]
roe_kinematics = ["roeE()", "roeM()", "roeP()", "roeMbc()", "roeDeltae()"]  # [S50]
roe_multiplicities = [
    "nROE_Charged()",
    "nROE_Photons()",
    "nROE_NeutralHadrons()",
]
b_vars += roe_kinematics + roe_multiplicities  # [E20]
# Let's also add a version of the ROE variables that includes the mask:
for roe_variable in roe_kinematics + roe_multiplicities:
    # e.g. instead of 'roeE()' (no mask) we want 'roeE(my_mask)'
    roe_variable_with_mask = roe_variable.replace("()", "(my_mask)")
    b_vars.append(roe_variable_with_mask)  # [E50]

# Variables for final states (electrons, positrons, pions)
fs_vars = vc.pid + vc.track + vc.track_hits + standard_vars
b_vars += vu.create_aliases_for_selected(
    fs_vars,
    "B0 -> [J/psi -> ^e+ ^e-] [K_S0 -> ^pi+ ^pi-]",
    prefix=["ep", "em", "pip", "pim"],
)
# Variables for J/Psi, KS
jpsi_ks_vars = vc.inv_mass + standard_vars
b_vars += vu.create_aliases_for_selected(jpsi_ks_vars, "B0 -> ^J/psi ^K_S0")
# Also add kinematic variables boosted to the center of mass frame (CMS)
# for all particles
cmskinematics = vu.create_aliases(
    vc.kinematics, "useCMSFrame({variable})", "CMS"
)
b_vars += vu.create_aliases_for_selected(
    cmskinematics, "^B0 -> [^J/psi -> ^e+ ^e-] [^K_S0 -> ^pi+ ^pi-]"
)

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
