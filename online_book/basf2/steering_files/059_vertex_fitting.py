#!/usr/bin/env python3

import sys
import basf2 as b2
import modularAnalysis as ma
import stdV0s
import flavorTagger as ft
from variables import variables
import variables.collections as vc
import variables.utils as vu
import vertex

# get input file number from the command line
filenumber = sys.argv[1]

# set analysis global tag (needed for flavor tagging)
b2.use_central_database("analysis_tools_release-04-02")

# create path
main = b2.Path()

# load input data from mdst/udst file
filedirectory = "/group/belle2/users/tenchini/prerelease-05-00-00a/1111540100/"
ma.inputMdstList(
    environmentType="default",
    filelist=[f"{filedirectory}/1111540100_eph3_BGx0_{filenumber}.root"],
    path=main,
)

# fill final state particle lists
ma.fillParticleList(
    "e+:uncorrected",
    "electronID > 0.1 and dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance",
    path=main,
)
stdV0s.stdKshorts(path=main)

# apply Bremsstrahlung correction to electrons
variables.addAlias(
    "goodFWDGamma", "passesCut(clusterReg == 1 and clusterE > 0.075)"
)
variables.addAlias(
    "goodBRLGamma", "passesCut(clusterReg == 2 and clusterE > 0.05)"
)
variables.addAlias(
    "goodBWDGamma", "passesCut(clusterReg == 3 and clusterE > 0.1)"
)
variables.addAlias(
    "goodGamma", "passesCut(goodFWDGamma or goodBRLGamma or goodBWDGamma)"
)
ma.fillParticleList("gamma:brems", "goodGamma", path=main)
ma.correctBrems("e+:corrected", "e+:uncorrected", "gamma:brems", path=main)
variables.addAlias("isBremsCorrected", "extraInfo(bremsCorrected)")

# combine final state particles to form composite particles
ma.reconstructDecay(
    "J/psi:ee -> e+:corrected e-:corrected ?addbrems",
    cut="dM < 0.11",
    path=main,
)

# perform vertex fit of J/psi candidates
vertex.KFit("J/psi:ee", conf_level=0.0, path=main)

# combine J/psi and KS candidates to form B0 candidates
ma.reconstructDecay(
    "B0 -> J/psi:ee K_S0:merged",
    cut="Mbc > 5.2 and abs(deltaE) < 0.15",
    path=main,
)

# match reconstructed with MC particles
ma.matchMCTruth("B0", path=main)

# build the rest of the event
ma.buildRestOfEvent("B0", fillWithMostLikely=True, path=main)
track_based_cuts = "thetaInCDCAcceptance and pt > 0.075"
ecl_based_cuts = "thetaInCDCAcceptance and E > 0.05"
roe_mask = ("my_mask", track_based_cuts, ecl_based_cuts)
ma.appendROEMasks("B0", [roe_mask], path=main)

# call flavor tagging
ft.flavorTagger("B0", path=main)

# remove B0 candidates without a valid flavor information
ma.applyCuts("B0", "qrOutput(FBDT) > -2", path=main)

# fit B vertex on the tag-side
vertex.TagV("B0", constraintType="tube", fitAlgorithm="Rave", path=main)

# perform best candidate selection
b2.set_random_seed("Belle II StarterKit")
ma.rankByHighest("B0", variable="random", numBest=1, path=main)

# Create list of variables to save into the output file
b_vars = []

standard_vars = vc.kinematics + vc.mc_kinematics + vc.mc_truth
b_vars += vc.deltae_mbc
b_vars += standard_vars
b_vars += vc.roe_kinematics + vc.roe_multiplicities
# Let's also add a version of the ROE variables that includes the mask:
for roe_variable in vc.roe_kinematics + vc.roe_multiplicities:
    # e.g. instead of 'roeE()' (no mask) we want 'roeE(my_mask)'
    roe_variable_with_mask = roe_variable.replace("()", "(my_mask)")
    b_vars.append(roe_variable_with_mask)
b_vars += ft.flavor_tagging
b_vars += vc.vertex + vc.mc_vertex

# Variables for final states (electrons, positrons, pions)
fs_vars = vc.pid + vc.track + vc.track_hits + standard_vars
b_vars += vu.create_aliases_for_selected(
    fs_vars + ["isBremsCorrected"],
    "B0 -> [J/psi -> ^e+ ^e-] K_S0",
    prefix=["ep", "em"],
)
b_vars += vu.create_aliases_for_selected(
    fs_vars, "B0 -> J/psi [K_S0 -> ^pi+ ^pi-]", prefix=["pip", "pim"]
)
# Variables for J/Psi, KS
jpsi_ks_vars = vc.inv_mass + standard_vars
jpsi_ks_vars += vc.vertex + vc.mc_vertex
b_vars += vu.create_aliases_for_selected(jpsi_ks_vars, "B0 -> ^J/psi ^K_S0")
# Also add kinematic variables boosted to the center of mass frame (CMS)
# for all particles
cmskinematics = vu.create_aliases(
    vc.kinematics, "useCMSFrame({variable})", "CMS"
)
b_vars += vu.create_aliases_for_selected(
    cmskinematics, "^B0 -> [^J/psi -> ^e+ ^e-] [^K_S0 -> ^pi+ ^pi-]"
)

variables.addAlias(
    "withBremsCorrection",
    "passesCut(passesCut(ep_isBremsCorrected == 1) or passesCut(em_isBremsCorrected == 1))",
)
b_vars += ["withBremsCorrection"]

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
