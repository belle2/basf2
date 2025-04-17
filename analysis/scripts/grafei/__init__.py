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
import basf2 as b2


def graFEI(
    list_name,
    path,
    particle_lists=None,
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
    Wrapper function to add the GraFEIModule to the path and perform other actions behind the scenes.

    Applies graFEI model to a (list of) particle list(s) in basf2.
    GraFEI information is stored as eventExtraInfos.

    .. note::
        ``list_name`` should always be provided. This is the name of the particle list to be given as input to the graFEI.
        If ``list_name`` refers to an existing particle list, it is used as input to the model.
        If also a list of final state particle lists is provided in ``particle_lists``, these are combined to form
        a new list called ``list_name`` (if ``list_name`` already exists an error is thrown).
        If ``particle_list`` is provided, the mass hypotheses of final state particles are updated to match graFEI predictions.

    Args:
        list_name (str): Name of particle list given as input to the model.
        path (basf2.Path): Module is added to this path.
        particle_lists (list): List of particle lists. If provided, these are combined to form ``list_name``.
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
    if particle_lists:
        ma.combineAllParticles(particle_lists, list_name, path=path)
    if store_mc_truth:
        ma.matchMCTruth(list_name, path=path)
        ma.fillParticleListFromMC("Upsilon(4S):MC", "", path=path)
        ma.fillParticleListFromMC("B0:MC", "", path=path)
        ma.fillParticleListFromMC("B+:MC", "", path=path)

    graFEI = GraFEIModule(
        list_name,
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
        list_name,
        dict((f"extraInfo({var})", var) for var in graFEI_vars),
        path=path,
    )

    # Update mass hypotheses
    if particle_lists:
        charged_lists = [ls for ls in particle_lists if "gamma:" not in ls]
        photon_lists = [ls for ls in particle_lists if "gamma:" in ls]

        hypotheses = {  # PDG code and graFEI class for each mass hypothesis
            "K": (321, 4),
            "pi": (211, 3),
            "mu": (13, 2),
            "e": (11, 1),
            "p": (2212, 5),
        }
        for newHyp in hypotheses:  # Loop over all possible final mass hypotheses
            newPDG = hypotheses[newHyp][0]
            newClass = hypotheses[newHyp][1]
            final_lists = []
            for oldList in charged_lists:  # Loop over all charged particle lists
                ma.cutAndCopyList(  # Retain only particles with predicted mass hypothesis equal to `newClass`
                    f"{oldList}_to_{newHyp}_",
                    oldList,
                    f"extraInfo(graFEI_massHypothesis) == {newClass}",
                    path=path,
                )
                ma.updateMassHypothesis(  # Update basf2 mass hypothesis
                    f"{oldList}_to_{newHyp}_", newPDG, path=path
                )

                label = oldList[oldList.find(':')+1:]
                oldHyp = oldList[:oldList.find(':')-1]
                final_lists.append(f"{newHyp}+:{label}_to_{newHyp}__converted_from_{oldHyp}")

            ma.copyLists(  # Merge all temporary lists into final `newHyp` list
                f"{newHyp}+:graFEI",
                final_lists,
                writeOut=True,
                path=path,
            )

        # Take care of photons
        if len(photon_lists) > 0:
            ma.cutAndCopyList(
                "gamma:graFEI",
                photon_lists[0],
                "extraInfo(graFEI_massHypothesis) == 6",
                writeOut=True,
                path=path,
            )
        elif len(photon_lists) == 0:
            b2.B2WARNING("grafei.graFEI You did not define a photon input list. Therefore you don't have any as output.")
        if len(photon_lists) > 1:
            b2.B2WARNING("grafei.graFEI You defined more than one photon input list. Using the first one.")

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
        mcparticle_list (str): Name of particle list to build LCAs from (used as root).
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
