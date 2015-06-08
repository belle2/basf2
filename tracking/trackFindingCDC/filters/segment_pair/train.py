#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import os.path
import basf2

import tracking.utilities as utilities

from tracking.run.event_generation import StandardEventGenerationRun
from trackfindingcdc.tmva.train import FastBDTClassifier, ClassificationOverview

import ROOT.TFile

import logging


def main():

    argument_parser = utilities.DefaultHelpArgumentParser()

    argument_parser.add_argument(
        "records_file_path",
        help="ROOT file containing the TTree of records on which to train a boosted decision tree.",
    )

    argument_parser.add_argument(
        "-t",
        "--input-tree-name",
        default="records",
        help="Name of the input TTree in the ROOT file",
    )

    arguments = argument_parser.parse_args()

    records_file_path = arguments.records_file_path
    records_folder_path, records_file_name = os.path.split(records_file_path)

    records_file_basename, ext = os.path.splitext(records_file_name)

    weights_file_name = records_file_basename + ".weights" + ext
    weights_file_path = os.path.join(records_folder_path, weights_file_name)

    overview_file_name = records_file_basename + ".overview" + ext
    overview_file_path = os.path.join(records_folder_path, overview_file_name)

    decision_tree_name = records_file_basename

    overview = ClassificationOverview(overview_file_name)
    classifer = FastBDTClassifier(
        decision_tree_name,
        weights_file_path,
        replace_nan=-999,
    )

    input_file_name = records_file_name
    input_file = ROOT.TFile(input_file_name, "READ")
    input_tree_name = arguments.input_tree_name
    input_tree = input_file.Get(input_tree_name)

    overview.train(input_tree)
    classifer.train(input_tree)


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
