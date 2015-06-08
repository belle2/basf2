#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import os.path
import basf2
from tracking.run.event_generation import StandardEventGenerationRun
from trackfindingcdc.tmva.train import FastBDTClassifier, ClassificationOverview

import ROOT.TFile

import logging


def main():
    overview = ClassificationOverview("CDCSegmentPairOverview.root")
    classifer = FastBDTClassifier(
        "CDCSegmentPair",
        "CDCSegmentPairWeights.root",
        replace_nan=-999,
    )

    input_file_name = "CDCSegmentPairTruthRecords.root"
    input_file = ROOT.TFile(input_file_name, "READ")
    input_tree_name = "records"
    input_tree = input_file.Get(input_tree_name)

    overview.train(input_tree)
    classifer.train(input_tree)


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
