#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import basf2 as b2
import modularAnalysis as ma
from variables import variables as vm
import stdPhotons

# Necessary to run argparse
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True  # noqa

import random
import argparse

from grafei import graFEI


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
        default="analysis_tools_light-2403-persian",
        help="Globaltag containing graFEI model",
    )
    parser.add_argument(
        "-l",
        "--lcas",
        type=list,
        default=[[0]],
        help="Choose LCAS matrix of signal side",
    )
    parser.add_argument(
        "-m",
        "--masses",
        type=list,
        default=["k"],
        help="Choose mass hypotheses of signal side",
    )
    parser.add_argument(
        "-n",
        "--no_mc_truth",
        action="store_true",
        help="Choose not to store MC-truth information",
    )
    return parser.parse_args()


if __name__ == "__main__":
    args = _parse_args()

    store_mc_truth = not args.no_mc_truth

    path = b2.create_path()
    ma.inputMdst(filename=b2.find_file('mdst16.root', 'validation', False), path=path)

    b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())
    if args.globaltag:
        b2.conditions.prepend_globaltag(args.globaltag)

    #################################################################################################
    # GraFEI requirements and reconstruction
    # 1) Cuts on charged tracks and photons
    # 2) Keep only "good events" i.e. validTree + signal side matches chosen LCAS and mass hypotheses
    #################################################################################################

    # 1) Cuts on charged tracks and photons
    # These priors were obtained by counting truth-matched tracks in BB mixed MC
    # It could be modified by the user if needed
    priors = [0.068, 0.050, 0.7326, 0.1315, 0.0183, 0.00006]

    charged_cuts = [
        f"pidIsMostLikely({','.join(str(p) for p in priors)})>0",
        "nCDCHits>20",
        "thetaInCDCAcceptance",
        "abs(dz)<1.0",
        "dr<0.5",
        "p<5",
        "pt>0.2",
    ]

    photon_cuts = [
        "beamBackgroundSuppression>0.4",
        "fakePhotonSuppression>0.3",
        "abs(clusterTiming)<100",
        "abs(formula(clusterTiming/clusterErrorTiming))<2.0",
        "[[clusterReg==1 and E>0.09] or [clusterReg==2 and E>0.09] or [clusterReg==3 and E>0.14]]",
    ]

    evt_cut = "n_gamma_in_evt<20 and n_charged_in_evt<20"

    # Fill particle list with optimized cuts
    charged_lists = [f"{c}:final" for c in ["p+", "e+", "pi+", "mu+", "K+"]]

    ma.fillParticleLists(
        [(c, " and ".join(charged_cuts)) for c in charged_lists],
        writeOut=True,
        path=path,
    )

    stdPhotons.stdPhotons(
        listtype="tight",
        beamBackgroundMVAWeight="MC15ri",
        fakePhotonMVAWeight="MC15ri",
        path=path,
    )

    ma.cutAndCopyList(
        "gamma:final",
        "gamma:tight",
        " and ".join(photon_cuts),
        writeOut=True,
        path=path,
    )

    # Add requirements on total number of photons and charged in event
    vm.addAlias("n_gamma_in_evt", "nParticlesInList(gamma:final)")
    vm.addAlias("n_p_in_evt", "nParticlesInList(p+:final)")
    vm.addAlias("n_e_in_evt", "nParticlesInList(e+:final)")
    vm.addAlias("n_mu_in_evt", "nParticlesInList(mu+:final)")
    vm.addAlias("n_pi_in_evt", "nParticlesInList(pi+:final)")
    vm.addAlias("n_K_in_evt", "nParticlesInList(K+:final)")
    vm.addAlias(
        "n_charged_in_evt",
        "formula(n_p_in_evt+n_e_in_evt+n_mu_in_evt+n_pi_in_evt+n_K_in_evt)",
    )

    ma.applyEventCuts(evt_cut, path=path)

    particle_lists = charged_lists + ["gamma:final"]
    particle_types = [x.split(":")[0] for x in particle_lists]
    charged_types = [x.split(":")[0] for x in charged_lists]

    graFEI_vars = graFEI(
        "Upsilon(4S):final",
        particle_lists=particle_lists,
        store_mc_truth=store_mc_truth,
        cfg_path=args.config,
        param_file=args.weight,
        sig_side_lcas=args.lcas,
        sig_side_masses=args.masses,
        payload_config_name="graFEIConfigFile_Upsreco_example",  # If you use default payload name just remove this argument
        payload_model_name="graFEIModelFile_Upsreco_example",  # If you use default payload name just remove this argument
        path=path,
    )

    # Define signal-side B
    ma.reconstructDecay(
        "B+:Bsgn -> K+:graFEI",
        "daughter(0, extraInfo(graFEI_sigSide)) == 1",
        path=path,
    )

    # Define tag-side B
    for part in particle_types:
        ma.cutAndCopyList(
            f"{part}:Btag",
            f"{part}:graFEI",
            cut="extraInfo(graFEI_sigSide) == 0",
            writeOut=True,
            path=path,
        )
    ma.combineAllParticles([f"{part}:Btag" for part in particle_types], "B+:Btag", path=path)

    # 3) Keep only "good events" i.e. validTree + signal side matches chosen LCAS and mass hypotheses
    ma.reconstructDecay(
        "Upsilon(4S):neutral -> B+:Bsgn B-:Btag", "", path=path
    )
    ma.reconstructDecay(
        "Upsilon(4S):charged -> B+:Bsgn B+:Btag", "", allowChargeViolation=True, path=path
    )

    ma.copyLists(
            "Upsilon(4S):graFEI",
            [
                "Upsilon(4S):neutral",
                "Upsilon(4S):charged",
            ],
            path=path,
        )

    # Reject events with no signal candidates
    skimfilter = b2.register_module("SkimFilter")
    skimfilter.param("particleLists", ["Upsilon(4S):graFEI"])
    empty_path = b2.create_path()
    skimfilter.if_value("=0", empty_path, b2.AfterConditionPath.END)
    path.add_module(skimfilter)

    if store_mc_truth:
        ma.matchMCTruth("B+:Bsgn", path=path)
        ma.matchMCTruth("B-:Btag", path=path)
        ma.matchMCTruth("Upsilon(4S):graFEI", path=path)

    ################################################################################
    # Make ntuples
    ################################################################################

    # Variables
    momentum_vars = [
        "p",
        "px",
        "py",
        "pz",
        "pt",
        "E",
    ]
    default_vars = [
        "PDG",
        "M",
        "Mbc",
        "deltaE",
    ]
    default_vars += momentum_vars

    tm_vars = [
        "mcErrors",
        "genMotherPDG",
        "mcPDG",
    ] if store_mc_truth else []

    default_vars += tm_vars

    # Aliases
    for var in default_vars:
        vm.addAlias(f"Bsgn_{var}", f"daughter(0, {var})")
        vm.addAlias(f"Bsgn_d0_{var}", f"daughter(0, daughter(0, {var}))")
        vm.addAlias(f"Btag_{var}", f"daughter(1, {var})")
        vm.addAlias(f"Ups_{var}", var)
    for var in momentum_vars:
        vm.addAlias(
            f"Bsgn_d0_CMS_{var}", f"useCMSFrame(daughter(0, daughter(0, {var})))"
        )
        vm.addAlias(f"Btag_CMS_{var}", f"useCMSFrame(daughter(1, {var}))")
    for var in graFEI_vars:
        vm.addAlias(var, f"eventExtraInfo({var})")

    all_vars = (
        graFEI_vars
        + [f"Bsgn_{var}" for var in tm_vars]
        + [f"Bsgn_d0_{var}" for var in default_vars]
        + [f"Bsgn_d0_CMS_{var}" for var in momentum_vars]
        + [f"Btag_{var}" for var in default_vars]
        + [f"Btag_CMS_{var}" for var in momentum_vars]
        + [f"Ups_{var}" for var in default_vars]
    )

    ma.variablesToNtuple(
        "Upsilon(4S):graFEI",
        sorted(all_vars),
        filename="graFEI_UpsReco_example.root",
        treename="tree",
        path=path,
    )

    # Process
    b2.process(path, calculateStatistics=True)
