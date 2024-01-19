#!/usr/bin/env python3

import sys
import basf2 as b2
import modularAnalysis as ma
import stdV0s
from variables import variables as vm  # shorthand for VariableManager
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

# apply Bremsstrahlung correction to electrons [S10|S20]
vm.addAlias(
    "goodFWDGamma", "passesCut(clusterReg == 1 and clusterE > 0.075)"
)  # [E10]
vm.addAlias(
    "goodBRLGamma", "passesCut(clusterReg == 2 and clusterE > 0.05)"
)
vm.addAlias(
    "goodBWDGamma", "passesCut(clusterReg == 3 and clusterE > 0.1)"
)
vm.addAlias(
    "goodGamma", "passesCut(goodFWDGamma or goodBRLGamma or goodBWDGamma)"
)  # [E20]
ma.fillParticleList("gamma:brems", "goodGamma", path=main)
ma.correctBrems("e+:corrected", "e+:uncorrected", "gamma:brems", path=main)  # [S30]
vm.addAlias("isBremsCorrected", "extraInfo(bremsCorrected)")  # [E30]

# combine final state particles to form composite particles [S40]
ma.reconstructDecay(
    "J/psi:ee -> e+:corrected e-:corrected ?addbrems",
    cut="abs(dM) < 0.11",
    path=main,
)  # [E40]

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
track_based_cuts = "thetaInCDCAcceptance and pt > 0.075 and dr < 5 and abs(dz) < 10"
ecl_based_cuts = "thetaInCDCAcceptance and E > 0.05"
roe_mask = ("my_mask", track_based_cuts, ecl_based_cuts)
ma.appendROEMasks("B0", [roe_mask], path=main)

# perform best candidate selection [S60]
b2.set_random_seed("Belle II StarterKit")
ma.rankByHighest("B0", variable="random", numBest=1, path=main)  # [E60]

# Create list of variables to save into the output file
b_vars = []

standard_vars = vc.kinematics + vc.mc_kinematics + vc.mc_truth
b_vars += vc.deltae_mbc
b_vars += standard_vars

# ROE variables
roe_kinematics = ["roeE()", "roeM()", "roeP()", "roeMbc()", "roeDeltae()"]
roe_multiplicities = [
    "nROE_Charged()",
    "nROE_Photons()",
    "nROE_NeutralHadrons()",
]
b_vars += roe_kinematics + roe_multiplicities
# Let's also add a version of the ROE variables that includes the mask:
for roe_variable in roe_kinematics + roe_multiplicities:
    # e.g. instead of 'roeE()' (no mask) we want 'roeE(my_mask)'
    roe_variable_with_mask = roe_variable.replace("()", "(my_mask)")
    b_vars.append(roe_variable_with_mask)

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
b_vars += vu.create_aliases_for_selected(jpsi_ks_vars, "B0 -> ^J/psi ^K_S0")
# Add the J/Psi mass calculated with uncorrected electrons:
vm.addAlias(  # [S50]
    "Jpsi_M_uncorrected", "daughter(0, daughterCombination(M,0:0,1:0))"
)  # [E50]
b_vars += ["Jpsi_M_uncorrected"]
# Also add kinematic variables boosted to the center of mass frame (CMS)
# for all particles
cmskinematics = vu.create_aliases(
    vc.kinematics, "useCMSFrame({variable})", "CMS"
)
b_vars += vu.create_aliases_for_selected(
    cmskinematics, "^B0 -> [^J/psi -> ^e+ ^e-] [^K_S0 -> ^pi+ ^pi-]"
)

vm.addAlias(
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
