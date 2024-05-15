##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


__all__ = ["graFEI", "lcaSaver"]


from grafei.modules.GraFEIModule import GraFEIModule
from grafei.modules.LCASaverModule import LCASaverModule

import modularAnalysis as ma
from ROOT import Belle2 as b2


def graFEI(
    particle_list,
    path,
    store_mc_truth=False,
    cfg_path=None,
    param_file=None,
    sig_side_lcas=None,
    sig_side_masses=None,
    gpu=False,
    payload_config_name="graFEIConfigFile",
    payload_model_name="graFEIModelFile",
):
    """
    Wrapper function to add the GraFEIModule to the path in a single call.

    Applies graFEI model to a (list of) particle list(s) in basf2.
    GraFEI information is stored as eventExtraInfos.

    If `particle_list` is a list of particle lists, the conversion to graFEI predicted mass hypotheses is performed.

    Args:
        particle_list (str or list): Name of particle list (B reconstruction) or list of FSP lists (for Upsilon reconstruction).
        path (basf2.Path): Module is added to this path.
        store_mc_truth (bool): Whether to store MC truth information.
        cfg_path (str): Path to config file. If `None` the config file in the global tag is used.
        param_file (str): Path to parameter file containing the model. If `None` the parameter file in the global tag is used.
        sig_side_lcas (list): List containing LCAS matrix of signal-side.
        sig_side_masses (list): List containing mass hypotheses of signal-side.
        gpu (bool): Whether to run on a GPU.
        payload_config_name (str): Name of config file payload. The default should be kept, except in basf2 examples.
        payload_model_name (str): Name of model file payload. The default should be kept, except in basf2 examples.

    Returns:
        list: List of graFEI variables.
    """
    if isinstance(particle_list, list):
        input_list = "Upsilon(4S):final"
        ma.combineAllParticles(particle_list, input_list, path=path)
        if store_mc_truth:
            for ls in particle_list:
                ma.matchMCTruth(ls, path=path)
    elif isinstance(particle_list, str):
        input_list = particle_list
        if store_mc_truth:
            ma.matchMCTruth(particle_list, path=path)

    if store_mc_truth:
        ma.fillParticleListFromMC("Upsilon(4S):MC", "", path=path)
        ma.fillParticleListFromMC("B0:MC", "", path=path)

    graFEI = GraFEIModule(
        input_list,
        cfg_path=cfg_path,
        param_file=param_file,
        sig_side_lcas=sig_side_lcas,
        sig_side_masses=sig_side_masses,
        gpu=gpu,
        payload_config_name=payload_config_name,
        payload_model_name=payload_model_name,
    )
    path.add_module(graFEI)

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

    ma.variablesToEventExtraInfo(
        input_list,
        dict((f"extraInfo({var})", var) for var in graFEI_vars),
        path=path,
    )

    # Update mass hypotheses
    if isinstance(particle_list, list):
        charged_lists = [ls for ls in particle_list if "gamma:" not in ls]
        photon_lists = [ls for ls in particle_list if "gamma:" in ls]
        if len(photon_lists) != 1:
            b2.B2FATAL("grafei.graFEI You should use exactly one photon list as input to the graFEI.")
        masses = {  # PDG code and graFEI class for each mass hypothesis
            "K": (321, 4),
            "pi": (211, 3),
            "mu": (13, 2),
            "e": (11, 1),
            "p": (2212, 5),
        }
        for mass in masses:  # Loop over all possible final mass hypotheses
            for ls in charged_lists:  # Loop over all charged particle lists
                ma.cutAndCopyList(  # Retain only particles with predicted mass hypothesis equal to `mass`
                    f"{ls}_to_{mass}_",
                    ls,
                    f"extraInfo(graFEI_massHypothesis) == {masses[mass][1]}",
                    path=path,
                )
                ma.updateMassHypothesis(  # Update basf2 mass hypothesis to `mass`
                    f"{ls}_to_{mass}_", masses[mass][0], path=path
                )
            ma.copyLists(  # Merge all temporary lists into final `mass` list
                f"{mass}+:graFEI",
                [f"{mass}+:{ls[ls.find(':')+1:]}_to_{mass}__converted_from_{ls[:ls.find(':')-1]}" for ls in charged_lists],
                writeOut=True,
                path=path,
            )
        ma.cutAndCopyList(  # Take care of photons
            "gamma:graFEI",
            photon_lists[0],
            "extraInfo(graFEI_massHypothesis) == 6",
            writeOut=True,
            path=path,
        )

    return graFEI_vars


def lcaSaver(
    particle_lists,
    features,
    mcparticle_list,
    output_file,
    path,
):
    """
    Wrapper function to add the LCASaverModule to the path.

    Save Lowest Common Ancestor matrix of each MC Particle in the given list.

    Args:
        particle_lists (list): Name of particle lists to save features of.
        features (list): List of features to save for each particle.
        mcparticle_list (str): Name of particle list to build LCAs from (will use as root).
        output_file (str): Path to output file to save.
        path (basf2.Path): Module is added to this path.
    """
    root_saver_module = LCASaverModule(
        particle_lists=particle_lists,
        features=features,
        mcparticle_list=mcparticle_list,
        output_file=output_file,
    )
    path.add_module(root_saver_module)


print(
    r"""
                                                  ____  ____    _      ____  ____  _
                                                  |  _  |__|   /_\     |___  |___  |
                                                  |__|  |  \  /   \    |     |___  |

                                o
                               / \
                              /   \                             x-----x                        ＿      ＿
                             /     \            |-----\         |\   /|        |-----\        |0 1 3 3 5|
                            o       \           |----- \        | \ / |        |----- \       |1 0 3 3 5|
                           / \       \          |----- /        |  x  |        |----- /       |3 3 0 1 5|
                          /   \       \         |-----/         | / \ |        |-----/        |3 3 1 0 5|
                         /     \       \                        |/   \|                       |5 5 5 5 0|
                        o       o       \                       x-----x                        ￣      ￣
                       / \     / \       \
                      x   x   x   x       x

      Authors: Jacopo Cerasoli, Giulio Dujany, Lucas Martel, Corentin Santos. 2022 - 2024
      Model description: https://indico.cern.ch/event/1106990/papers/4996235/files/12252-ACAT_2022_proceedings.pdf
      Based on the work of Kahn et al: https://iopscience.iop.org/article/10.1088/2632-2153/ac8de0
      Please consider citing both articles.
      Code adapted from https://github.com/Helmholtz-AI-Energy/BaumBauen
      """
)
