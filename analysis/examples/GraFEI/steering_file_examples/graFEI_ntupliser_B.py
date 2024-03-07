#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import basf2 as b2
import modularAnalysis as ma
from variables import variables as vm
from stdCharged import stdK, stdPi

from ROOT import Belle2

# Necessary to run argparse
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True  # noqa

import random
import argparse
from pathlib import Path

from grafei import GraFEIModule

# Random seeds
b2.set_random_seed(42)
random.seed(42)


def _parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-c", "--config", type=str, default=None, help="graFEI config file"
    )
    parser.add_argument(
        "-w", "--weight", type=str, default=None, help="graFEI weight file"
    )
    parser.add_argument(
        "-g",
        "--globaltag",
        type=str,
        default="graFEI_mixed_LCAMassProb_v1",
        help="Globaltag containing graFEI model",
    )
    return parser.parse_args()


if __name__ == "__main__":
    # Random seeds
    b2.set_random_seed(42)
    random.seed(42)

    args = _parse_args()

    store_mc_truth = True

    b2.conditions.prepend_globaltag(args.globaltag)
    b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

    input_file = Path(Belle2.Environment.Instance().getInputFilesOverride()[0])

    path = b2.create_path()
    ma.inputMdst(str(input_file), path=path)

    # graFEI cuts
    priors = [0.068, 0.050, 0.7326, 0.1315, 0.0183, 0.00006]

    cut_charged_graFEI = [
        f"pidIsMostLikely({','.join(str(p) for p in priors)})>0",
        "nCDCHits>20",
        "thetaInCDCAcceptance",
        "abs(dz)<1.0",
        "dr<0.5",
        "p<5",
        "pt>0.2",
    ]
    cut_photons_graFEI = [
        "beamBackgroundSuppression>0.4",
        "fakePhotonSuppression>0.3",
        "abs(clusterTiming)<100",
        "abs(formula(clusterTiming/clusterErrorTiming))<2.0",
        "[[clusterReg==1 and E>0.09] or [clusterReg==2 and E>0.09] or [clusterReg==3 and E>0.14]]",
    ]
    cut_KL0_graFEI = "PDG==0"  # Reject KL0 for the moment

    # Additional cuts from stdPhotons tight list are added
    cut_photons_graFEI += [
        "inCDCAcceptance",
        "[clusterErrorTiming < 1e6 and [clusterE1E9 > 0.4 or E > 0.075]]",
    ]

    # How to deal with multiple signal side candidates
    ranking_variable = "abs(cosThetaBetweenParticleAndNominalB)"  # "random"  # abs(cosThetaBetweenParticleAndNominalB)
    # ----------------------------------------------------------------------------------------------------------

    cut_charged_graFEI = " and ".join(cut_charged_graFEI)
    cut_photons_graFEI = " and ".join(cut_photons_graFEI)

    # Load MVA's for all gamma
    ma.fillParticleList(
        "gamma:all",
        "",
        path=path,
    )
    ma.getBeamBackgroundProbability("gamma:all", "MC15ri", path=path)
    ma.getFakePhotonProbability("gamma:all", "MC15ri", path=path)

    # -----------------------Signal side reconstruction------------------------------------

    stdK("loose", path=path)
    stdPi("loose", path=path)

    ma.reconstructDecay(
        "D-:Kpipi -> K+:loose pi-:loose pi-:loose", "1.85 < M < 1.88", path=path
    )
    ma.fillParticleList(
        "mu+:tight",
        "abs(dz)<2 and dr<0.5 and nCDCHits>20 and thetaInCDCAcceptance and pt>0.6 and E<5.5 and muonID_noSVD>0.9",
        path=path,
    )
    ma.reconstructDecay(
        "B0:sig -> D-:Kpipi mu+:tight",
        "cosThetaBetweenParticleAndNominalB>-2 and cosThetaBetweenParticleAndNominalB<2",
        path=path,
    )

    # ROE
    ma.buildRestOfEvent("B0:sig", path=path)

    # Basic cuts to evaluate truth-matching on ROE
    basicMask = (
        "basicMask",
        f"pidIsMostLikely({','.join(str(p) for p in priors)})>0 and nCDCHits>0 and thetaInCDCAcceptance and abs(dz)<4 and dr<2",
        "inCDCAcceptance and clusterErrorTiming<1e6 and [clusterE1E9>0.4 or E>0.075] and [[clusterReg == 1 and E > 0.05]"
        " or [clusterReg == 2 and E > 0.05] or [clusterReg == 3 and E > 0.075]]",
    )

    # Continuum suppression
    csMask = (
        "csMask",
        "nCDCHits > 0 and useCMSFrame(p)<=3.2",
        "p >= 0.05 and useCMSFrame(p)<=3.2",
    )

    ma.appendROEMasks(list_name="B0:sig", mask_tuples=[csMask, basicMask], path=path)

    ma.buildContinuumSuppression(list_name="B0:sig", roe_mask="csMask", path=path)

    ma.applyCuts("B0:sig", "cosTBz<0.9", path=path)

    # Reject events with no signal candidates
    skimfilter = b2.register_module("SkimFilter")
    skimfilter.param("particleLists", ["B0:sig"])
    empty_path = b2.create_path()
    skimfilter.if_value("=0", empty_path, b2.AfterConditionPath.END)
    path.add_module(skimfilter)

    if store_mc_truth:
        ma.matchMCTruth("B0:sig", path=path)

    # perform random candidate selection
    ma.rankByLowest(
        "B0:sig",
        variable=ranking_variable,
        outputVariable="Rank",
        numBest=1,
        path=path,
    )

    if store_mc_truth:
        # Fill particle list from ROE B0:sig to evaluate truth-matching condition
        ma.fillParticleListFromROE(
            "B0:forTM",
            "",
            maskName="basicMask",
            sourceParticleListName="B0:sig",
            path=path,
        )

        ma.matchMCTruth("B0:forTM", path=path)

        ma.variablesToEventExtraInfo("B0:forTM", {"mcPDG": "Bsig_ROE_mcPDG"}, path=path)

    # -----------------------------------------Tag side reconstruction----------------------------------------

    if store_mc_truth:
        # Fill particle list of true B0 from ground-truth information
        ma.fillParticleListFromMC("B0:MC", "", path=path)

    graFEI_roe_mask = (
        "graFEIROE",
        cut_charged_graFEI,
        cut_photons_graFEI,
        cut_KL0_graFEI,
    )

    ma.appendROEMasks("B0:sig", [graFEI_roe_mask], path=path)

    # Fill particle list from ROE B0:sig
    ma.fillParticleListFromROE(
        "B0:tag",
        "",
        maskName="graFEIROE",
        sourceParticleListName="B0:sig",
        path=path,
    )

    if store_mc_truth:
        # Associate parent B to each FSP
        ma.matchMCTruth("B0:tag", path=path)

    # Run graFEI
    graFEI = GraFEIModule(
        "B0:tag",
        cfg_path=args.config,
        param_file=args.weight,
    )
    path.add_module(graFEI)

    # Reconstructin Upsilon(4S)
    ma.reconstructDecay("Upsilon(4S):graFEI -> B0:tag B0:sig", "", path=path)

    # ---------------- Write information to file ---------------------------

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
        "cosTBz",
        # "cosTBTO",
        "cosThetaBetweenParticleAndNominalB",
    ]
    if store_mc_truth:
        default_vars.extend(
            [
                "isSignal",
                "isSignalAcceptMissingNeutrino",
                "mcErrors",
                "genMotherPDG",
                "mcPDG",
            ]
        )

    # graFEI variables
    graFEI_vars = [
        "graFEI_probEdgeProd",
        "graFEI_probEdgeMean",
        "graFEI_probEdgeGeom",
        "graFEI_validTree",
        "graFEI_goodEvent",
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
    ]
    if store_mc_truth:
        graFEI_vars.extend(
            [
                "graFEI_truth_perfectLCA",
                "graFEI_truth_perfectMasses",
                "graFEI_truth_perfectEvent",
                "graFEI_truth_isSemileptonic",
                "graFEI_truth_nFSP",
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
    vm.addAlias("Bsig_Rank", "daughter(1, extraInfo(Rank))")

    for var in default_vars:
        vm.addAlias(f"Btag_{var}", f"daughter(0, {var})")
        vm.addAlias(f"Bsig_{var}", f"daughter(1, {var})")
    for var in graFEI_vars:
        vm.addAlias(f"Btag_{var}", f"daughter(0, extraInfo({var}))")

    graFEI_vars = [f"Btag_{var}" for var in graFEI_vars + default_vars] + \
        ["Btag_Mbc", "Bsig_Rank"] + [f"Bsig_{var}" for var in default_vars]

    vm.addAlias("Bsig_D_M", "daughter(1, daughter(0, M))")
    vm.addAlias("Bsig_D_E", "daughter(1, daughter(0, E))")
    vm.addAlias("Bsig_D_pt", "daughter(1, daughter(0, pt))")
    vm.addAlias("Bsig_D_p", "daughter(1, daughter(0, p))")

    graFEI_vars.extend(["Bsig_D_M", "Bsig_D_E", "Bsig_D_pt", "Bsig_D_p"])

    graFEI_vars = sorted(list(set(graFEI_vars)))

    ma.variablesToNtuple(
        "Upsilon(4S):graFEI",
        graFEI_vars,
        filename='graFEI_onROE_output.root',
        treename="tree",
        path=path,
    )

    # Process
    b2.process(path)

    # print out the summary
    print(b2.statistics)
