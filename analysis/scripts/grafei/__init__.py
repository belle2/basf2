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


def graFEI(
    particle_list,
    path,
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

    Applies graFEI model to a particle list in basf2.
    GraFEI information is stored as extraInfos.

    Args:
        particle_list (str): Name of particle list.
        path (basf2.Path): Module is added to this path.
        cfg_path (str): Path to config file. If `None` the config file in the global tag is used.
        param_file (str): Path to parameter file containing the model. If `None` the parameter file in the global tag is used.
        sig_side_lcas (list): List containing LCAS matrix of signal-side.
        sig_side_masses (list): List containing mass hypotheses of signal-side.
        gpu (bool): Whether to run on a GPU.
        payload_config_name (str): Name of config file payload. The default should be kept, except in basf2 examples.
        payload_model_name (str): Name of model file payload. The default should be kept, except in basf2 examples.
    """
    graFEI = GraFEIModule(
        particle_list,
        cfg_path=cfg_path,
        param_file=param_file,
        sig_side_lcas=sig_side_lcas,
        sig_side_masses=sig_side_masses,
        gpu=gpu,
        payload_config_name=payload_config_name,
        payload_model_name=payload_model_name,
    )
    path.add_module(graFEI)


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


print(r"""
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
      """)
