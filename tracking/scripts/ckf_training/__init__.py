#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2_mva
import tracking.root_utils as root_utils
from basf2 import conditions


def my_basf2_mva_teacher(
    records_files,
    tree_name,
    weightfile_identifier,
    target_variable="truth",
    exclude_variables=None,
    fast_bdt_option=[200, 8, 3, 0.1]
):

    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    """
    Custom wrapper for basf2 mva teacher. Adapted from code in ``trackfindingcdc_teacher``.

    :param records_files: List of files with collected ("recorded") variables to use as training data for the MVA.
    :param tree_name: Name of the TTree in the ROOT file from the ``data_collection_task``
           that contains the training data for the MVA teacher.
    :param weightfile_identifier: Name of the weightfile that is created.
           Must not end in .xml nor in .root since the payload will be later downloaded to a local database.
    :param target_variable: Feature/variable to use as truth label in the quality estimator MVA classifier.
    :param exclude_variables: List of collected variables to not use in the training of the QE MVA classifier.
           In addition to variables containing the "truth" substring, which are excluded by default.
    :param fast_bdt_option: specified fast BDT options, default: [200, 8, 3, 0.1] [nTrees, nCuts, nLevels, shrinkage]
    """
    if exclude_variables is None:
        exclude_variables = []

    # extract names of all variables from one record file
    with root_utils.root_open(records_files[0]) as records_tfile:
        input_tree = records_tfile.Get(tree_name)
        feature_names = [leave.GetName() for leave in input_tree.GetListOfLeaves()]

    # get list of variables to use for training without MC truth
    truth_free_variable_names = [
        name
        for name in feature_names
        if (
            ("truth" not in name) and
            (name != target_variable) and
            (name not in exclude_variables)
        )
    ]
    if "weight" in truth_free_variable_names:
        truth_free_variable_names.remove("weight")
        weight_variable = "weight"
    elif "__weight__" in truth_free_variable_names:
        truth_free_variable_names.remove("__weight__")
        weight_variable = "__weight__"
    else:
        weight_variable = ""

    # Set options for MVA training
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector(*records_files)
    general_options.m_treename = tree_name
    general_options.m_weight_variable = weight_variable
    general_options.m_identifier = weightfile_identifier
    general_options.m_variables = basf2_mva.vector(*truth_free_variable_names)
    general_options.m_target_variable = target_variable
    fastbdt_options = basf2_mva.FastBDTOptions()

    fastbdt_options.m_nTrees = fast_bdt_option[0]
    fastbdt_options.m_nCuts = fast_bdt_option[1]
    fastbdt_options.m_nLevels = fast_bdt_option[2]
    fastbdt_options.m_shrinkage = fast_bdt_option[3]
    # Train a MVA method.
    basf2_mva.teacher(general_options, fastbdt_options)


def create_fbdt_option_string(fast_bdt_option):
    """
    Returns a readable string created by the ``fast_bdt_option`` array.

    :param fast_bdt_option: List containing the FastBDT options that should be converted to a human readable string
    """
    return f"_nTrees{fast_bdt_option[0]}_nCuts{fast_bdt_option[1]}"\
           f"_nLevels{fast_bdt_option[2]}_shrin{int(round(100*fast_bdt_option[3], 0))}"


if __name__ == "__main__":
    pass
