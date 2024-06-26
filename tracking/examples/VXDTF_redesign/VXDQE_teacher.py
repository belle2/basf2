#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#####################################################################
# VXDTF2 Example Scripts - FastBDT weightfile training for MVA QualityEstimator
#
# Trains an MVA method from a TTree set of records
#####################################################################


import sys
import subprocess
import argparse

import tracking.utilities as utilities
import tracking.root_utils as root_utils

import logging


def main():
    argument_parser = utilities.DefaultHelpArgumentParser()

    argument_parser.add_argument(
        "records_file_path",
        help="ROOT file containing the TTree of records on which to train a boosted decision tree.",
    )

    argument_parser.add_argument(
        "-r",
        "--treename",
        default="tree",
        help="Name of the input TTree in the ROOT file",
    )

    argument_parser.add_argument(
        "-i",
        "--identifier",
        default=argparse.SUPPRESS,
        help="Database identifier or name of weight file to be generated",
    )

    argument_parser.add_argument(
        "-t",
        "--truth",
        type=str,
        default="truth",
        help="Name of the column containing the truth information."
    )

    argument_parser.add_argument(
        "--variables",
        default=None,
        nargs='+',
        help="Name of the column containing the variables to be used."
    )

    argument_parser.add_argument(
        "-x",
        "--variable_excludes",
        default=None,
        nargs='+',
        help="Variables to be excluded"
    )

    argument_parser.add_argument(
        "--method",
        type=str,
        default="FastBDT",
        help="MVA Method [FastBDT], not implemented: [NeuroBayes|TMVA|XGBoost|Theano|Tensorflow|FANN|SKLearn]"
    )

    argument_parser.add_argument(
        "-e",
        "--evaluate",
        action="store_true",
        help="Evaluate the method after the training is finished"
    )

    argument_parser.add_argument(
        "-n",
        "--fillnan",
        action="store_true",
        help="Fill nan and inf values with actual numbers in evaluation"
    )

    arguments = argument_parser.parse_args()

    records_file_path = arguments.records_file_path
    treename = arguments.treename
    feature_names = arguments.variables

    excludes = arguments.variable_excludes
    if excludes is None:
        excludes = ["truth"]
    elif "truth" not in excludes:
        excludes.append("truth")

    print('excludes: ', excludes)

    # Figure out feature variables
    if feature_names is None:
        with root_utils.root_open(records_file_path) as records_tfile:
            input_tree = records_tfile.Get(treename)
            feature_names = [leave.GetName() for leave in input_tree.GetListOfLeaves()]

    truth = arguments.truth
    method = arguments.method

    identifier = vars(arguments).get("identifier", method + ".weights.xml")

    # Remove the variables that have Monte Carlo truth information unless explicitly selected
    truth_free_variable_names = [name for name
                                 in feature_names
                                 if name not in excludes]

    weight_variable = ""
    if "weight" in truth_free_variable_names:
        truth_free_variable_names.remove("weight")
        weight_variable = "weight"

    elif "__weight__" in truth_free_variable_names:
        truth_free_variable_names.remove("__weight__")
        weight_variable = "__weight__"

    else:
        weight_variable = ""

    cmd = [
        "basf2_mva_teacher",
        "--datafiles", records_file_path,
        "--treename", treename,
        "--identifier", identifier,
        "--target_variable", truth,
        "--method", method,
        "--variables", *truth_free_variable_names,
        "--weight_variable", weight_variable,
    ]

    print(cmd)
    subprocess.call(cmd)

    if arguments.evaluate:
        evaluation_pdf = identifier.rsplit(".", 1)[0] + ".pdf"
        cmd = [
            "basf2_mva_evaluate.py",
            "--identifier", identifier,
            "-d", records_file_path,
            "--treename", treename,
            "-o", evaluation_pdf
        ]
        if arguments.fillnan:
            cmd.append("-n")
        print(cmd)
        subprocess.call(cmd)


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
