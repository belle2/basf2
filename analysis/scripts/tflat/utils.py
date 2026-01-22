#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from yaml import full_load
from basf2 import find_file


def get_variables(particle_list, ranked_variable, variables=None, particleNumber=1):
    """ creates variable name pattern requested by the basf2 variable getVariableByRank()
    :param particle_list:
    :param ranked_variable:
    :param variables:
    :param particleNumber:
    :return:
    """
    var_list = []
    for i_num in range(1, particleNumber + 1):
        for var in variables:
            var_list.append(f'getVariableByRank({particle_list}, {ranked_variable}, {var}, {str(i_num)})')
    return var_list


def load_config(uniqueIdentifier):
    """
    Loads the correct yaml config file for the specified TFlaT uniqueIdentifier.
    All dynamic alterations of the config are handeled in this function.
    The final config is returned as a dict.
    """
    with open(find_file(f'{uniqueIdentifier}.yaml')) as f:
        config = full_load(f)

    # Resolve placeholder names
    maskName = config["TFLAT_Mask"][0]
    config["roe_variable_list"] = [v.format(maskName=maskName) for v in config["roe_variable_list"]]
    config["trk_cut"] = config["trk_cut"].format(maskName=maskName)
    config["gamma_cut"] = config["gamma_cut"].format(maskName=maskName)

    # Calculate input list lengths
    config["parameters"]["num_trk_features"] = len(config["trk_variable_list"])
    config["parameters"]["num_ecl_features"] = len(config["ecl_variable_list"])
    config["parameters"]["num_roe_features"] = len(config["roe_variable_list"])

    return config
