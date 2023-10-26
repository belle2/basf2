#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import basf2 as b2
import modularAnalysis as ma
from ROOT import Belle2
from variables import variables as vm
import random
import argparse
from grafei import graFEISaverModule
from grafei import FlagBDecayModule

# Random seeds
b2.set_random_seed("Pino")
random.seed(10)

# ------------------------SETTINGS--------------------------------------------------------------------------
# graFEI cuts
priors = [0.068, 0.050, 0.7326, 0.1315, 0.0183, 0.00006]
# f"pidIsMostLikely({','.join(str(p) for p in priors)})" #"pidIsMostLikely()"
most_likely_tracks = f"pidIsMostLikely({','.join(str(p) for p in priors)})"
# most_likely_tracks = "extraInfo(pidIsMostLikelyTempVars)"

cut_charged_graFEI = [
    "mostLikelyTracks>0",
    "nCDCHits>20",
    "thetaInCDCAcceptance",
    "abs(dz)<1.0",
    "dr<0.5",
    "p<5",
    "pt>0.2",
]
cut_photons_graFEI = [
    "beamBackgroundSuppression>0.4",
    "hadronicSplitOffSuppression>0.3",
    "abs(clusterTiming)<100",
    "abs(formula(clusterTiming/clusterErrorTiming))<2.0",
    "[[clusterReg==1 and E>0.09] or [clusterReg==2 and E>0.09] or [clusterReg==3 and E>0.14]]",
]
cut_KL0_graFEI = "PDG==0"  # Reject KL0 for the moment
# graFEI globaltag name
# graFEI_globaltag = "graFEI_mixed_LCAMassProb_v1"

# Additional cuts from stdPhotons tight list are added
cut_photons_graFEI += [
    "inCDCAcceptance",
    "[clusterErrorTiming < 1e6 and [clusterE1E9 > 0.4 or E > 0.075]]",
]

# ----------------------------------------------------------------------------------------------------------

cut_charged_graFEI = " and ".join(cut_charged_graFEI)
cut_photons_graFEI = " and ".join(cut_photons_graFEI)


parser = argparse.ArgumentParser()
parser.add_argument(
    "-m",
    "--mc",
    required=False,
    default=None,
    choices=["Upsilon(4S):MC", "B0:MC", "B+:MC"],
    help="Decide which MC particle to load for truth-matching")
parser.add_argument("-c", "--cfg", type=str, default=None, help="graFEI config file")
parser.add_argument("-w", "--weight", type=str, default=None, help="graFEI weight file")
args = parser.parse_args()
cfg_file = args.cfg
weight_file = args.weight
mc = args.mc

assert not args.mc or args.mc == "Upsilon(4S):MC", "This script actually works only for Upsilon"

# Create path
path = b2.Path()

# Load input data from mdst/udst file
ma.inputMdstList(
    filelist=[""],  # Specify input file in command line with -i
    path=path,
    environmentType="default",
)

# Add globaltags
# b2.conditions.prepend_globaltag(graFEI_globaltag)
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# Alias for pidIsMostLikely depending on chosen priors
vm.addAlias("mostLikelyTracks", most_likely_tracks)

# Load photons MVAs once and for all
ma.fillParticleList(
    "gamma:all",
    "",
    path=path,
)
ma.getBeamBackgroundProbability("gamma:all", "MC15ri", path=path)
ma.getFakePhotonProbability("gamma:all", "MC15ri", path=path)

charged_types = ["e+", "mu+", "pi+", "K+", "p+"]
part_types = charged_types + ["gamma"]

for part in part_types:
    # Fill lists
    cut = cut_photons_graFEI if part == "gamma" else cut_charged_graFEI

    ma.fillParticleList(
        f"{part}:graFEI",
        cut,
        path=path,
    )

    if mc:
        # Create relations to MCParticles
        ma.matchMCTruth(f"{part}:graFEI", path=path)

        # Associate parent B to each FSP
        flag_decay_module = FlagBDecayModule(
            f"{part}:graFEI",
            b_parent_var="BParentGenID",
        )
        path.add_module(flag_decay_module)

# most_likely_module = isMostLikelyTempVars([f"{part}:graFEI" for part in charged_types], priors)
# path.add_module(most_likely_module)

for c in charged_types:
    ma.applyCuts(f"{c}:graFEI", "mostLikelyTracks>0", path=path)

ma.combineAllParticles(
    [f"{part}:graFEI" for part in part_types], "Upsilon(4S):graFEI", path=path
)

if mc:
    ma.fillParticleListFromMC("Upsilon(4S):MC", "", path=path)
    # ma.matchMCTruth("Upsilon(4S):graFEI", path=path)

graFEI = graFEISaverModule(
    "Upsilon(4S):graFEI",
    store_true_info=mc,
    cfg_path=cfg_file,
    param_file=weight_file,
)
path.add_module(graFEI)

for part in part_types:
    ma.cutAndCopyList(
        f"{part}:B_1",
        f"{part}:graFEI",
        cut="extraInfo(graFEI_fromWhichB) == 1",
        writeOut=True,
        path=path,
    )
    ma.cutAndCopyList(
        f"{part}:B_2",
        f"{part}:graFEI",
        cut="extraInfo(graFEI_fromWhichB) == 2",
        writeOut=True,
        path=path,
    )
    ma.cutAndCopyList(
        f"{part}:notB",
        f"{part}:graFEI",
        cut="extraInfo(graFEI_fromWhichB) == -1",
        writeOut=True,
        path=path,
    )

ma.combineAllParticles(
    [f"{part}:B_1" for part in part_types], "B0:B_1_graFEI", path=path
)
ma.combineAllParticles(
    [f"{part}:B_2" for part in part_types], "B0:B_2_graFEI", path=path
)
ma.combineAllParticles(
    [f"{part}:notB" for part in part_types], "B0:notB_graFEI", path=path
)

# # Continuum suppression
# csMask = (
#     "csMask",
#     "nCDCHits > 0 and useCMSFrame(p)<=3.2",
#     "p >= 0.05 and useCMSFrame(p)<=3.2",
# )

# ma.appendROEMasks(list_name="B0:B_1_graFEI", mask_tuples=[csMask], path=path)
# ma.appendROEMasks(list_name="B0:B_2_graFEI", mask_tuples=[csMask], path=path)

# ma.buildContinuumSuppression(list_name="B0:B_1_graFEI", roe_mask="csMask", path=path)
# ma.buildContinuumSuppression(list_name="B0:B_2_graFEI", roe_mask="csMask", path=path)

if mc:
    ma.matchMCTruth("B0:B_1_graFEI", path=path)
    ma.matchMCTruth("B0:B_2_graFEI", path=path)
    ma.matchMCTruth("Upsilon(4S):graFEI", path=path)

# ---------------- Write information to file ---------------------------
input_file_path = Belle2.Environment.Instance().getInputFilesOverride()[0]

# Variables
default_vars = [
    "M",
    "Mbc",
    "deltaE",
    "p",
    "px",
    "py",
    "pz",
    "pt",
    "E",
    "phi",
    "theta",
    # "cosTBz",
    # "cosTBTO",
]
if mc:
    default_vars.extend(
        [
            # "isSignal",
            # "isSignalAcceptMissingNeutrino",
            "mcErrors",
            "genMotherPDG",
            "mcPDG",
        ]
    )

ma.variablesToEventExtraInfo(
    "B0:B_1_graFEI",
    dict((var, f"Btag_{var}") for var in default_vars),
    path=path,
)
ma.variablesToEventExtraInfo(
    "B0:B_2_graFEI",
    dict((var, f"Bsig_{var}") for var in default_vars),
    path=path,
)

# graFEI variables
graFEI_vars = [
    "graFEI_probEdgeProd",
    "graFEI_probEdgeMean",
    "graFEI_probEdgeGeom",
    "graFEI_validTree",
    "graFEI_6inLCAS",
    "graFEI_5inLCAS",
    "graFEI_4inLCAS",
    "graFEI_3inLCAS",
    "graFEI_2inLCAS",
    "graFEI_1inLCAS",
    "graFEI_nIntermediates",
    "graFEI_nFSP",
    "graFEI_nCharged_preFit",
    "graFEI_nElectrons_preFit",
    "graFEI_nMuons_preFit",
    "graFEI_nPions_preFit",
    "graFEI_nKaons_preFit",
    "graFEI_nProtons_preFit",
    "graFEI_nLeptons_preFit",
    "graFEI_nPhotons_preFit",
    "graFEI_nOthers_preFit",
    "graFEI_nCharged_postFit",
    "graFEI_nElectrons_postFit",
    "graFEI_nMuons_postFit",
    "graFEI_nPions_postFit",
    "graFEI_nKaons_postFit",
    "graFEI_nProtons_postFit",
    "graFEI_nLeptons_postFit",
    "graFEI_nPhotons_postFit",
    "graFEI_nOthers_postFit",
    "graFEI_nPredictedUnmatched",
    "graFEI_nPredictedUnmatched_noPhotons",
    "graFEI_depthLCA",
    "graFEI_nBtag_daughters",
    "graFEI_nBsig_daughters",
    "graFEI_notB_daughters",
]
if mc:
    graFEI_vars.extend(
        [
            "graFEI_truth_perfectLCA",
            "graFEI_truth_perfectMasses",
            "graFEI_truth_perfectEvent",
            "graFEI_truth_isSemileptonic",
            "graFEI_truth_nFSP",
            "graFEI_truth_depthLCA",
            "graFEI_truth_nPhotons",
            "graFEI_truth_nElectrons",
            "graFEI_truth_nMuons",
            "graFEI_truth_nPions",
            "graFEI_truth_nKaons",
            "graFEI_truth_nProtons",
            "graFEI_truth_nOthers",
        ]
    )

# Aliases
for var in default_vars:
    vm.addAlias(f"Ups_Bsig_{var}", f"eventExtraInfo(Bsig_{var})")
    vm.addAlias(f"Ups_Btag_{var}", f"eventExtraInfo(Btag_{var})")
for var in graFEI_vars:
    vm.addAlias(f"Ups_{var}", f"extraInfo({var})")

graFEI_vars = (
    [f"Ups_{var}" for var in graFEI_vars]
    + [f"Ups_Bsig_{var}" for var in default_vars]
    + [f"Ups_Btag_{var}" for var in default_vars]
)
graFEI_vars = sorted(list(set(graFEI_vars)))

ma.variablesToNtuple(
    "Upsilon(4S):graFEI",
    graFEI_vars,
    filename=f'graFEI_UpsReco_{input_file_path[input_file_path.rfind("/")+1:]}',
    treename="tree",
    path=path,
)

# Process
b2.process(path)

# print out the summary
print(b2.statistics)
