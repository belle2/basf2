#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import os.path
import basf2
from trackfindingcdc.tmva.train import FastBDTClassifier

import ROOT.TFile

import logging


def main():
    classifer = FastBDTClassifier(
        "SegmentTrackTruth",
        "SegmentTrackTruthTruthWeight.root"
    )

    input_file_name = "SegmentTrackChooser.root"
    input_file = ROOT.TFile(input_file_name, "READ")
    input_tree_name = "records"
    input_tree = input_file.Get(input_tree_name)

    classifer.train(input_tree)

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
