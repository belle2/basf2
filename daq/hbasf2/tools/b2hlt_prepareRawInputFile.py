#!/usr/bin/env python3

# ************************************************************************#
# BASF2 (Belle Analysis Framework 2)                                      #
# Copyright(C) 2019 - Belle II Collaboration                              #
#                                                                         #
# Author: The Belle II Collaboration                                      #
# Contributors: Nils Braun                                                #
#                                                                         #
# This software is provided "as is" without any warranty.                 #
# ************************************************************************#

import basf2
from softwaretrigger.constants import HLT_INPUT_OBJECTS, EXPRESSRECO_INPUT_OBJECTS

import os
from argparse import ArgumentParser


if __name__ == '__main__':
    parser = ArgumentParser(description="Helper script to convert a root file into an input file needed for HLT ZMQ tests.")
    parser.add_argument("input_file", help="Input file name")
    parser.add_argument("output_file", help="Output file name")
    parser.add_argument("--number", help="How many events should be converted (default 10)", default=10, type=int)
    parser.add_argument("--expressreco", help="Convert to express reco format, instead of HLT", action="store_true")

    args = parser.parse_args()
    input_file = args.input_file
    output_file = args.output_file

    basf2.conditions.override_globaltags([])
    path = basf2.Path()

    branchNames = HLT_INPUT_OBJECTS
    if args.expressreco:
        branchNames = EXPRESSRECO_INPUT_OBJECTS

    extension = os.path.splitext(input_file)[-1]
    if extension == ".root":
        path.add_module("RootInput", inputFileName=input_file, branchNames=branchNames)
    else:
        raise AttributeError(f"Do not understand file format of {input_file}")

    if not args.expressreco:
        path.add_module("Root2Raw", outputFileName=output_file + ".raw")
    path.add_module("SeqRootOutput", outputFileName=output_file + ".sroot")
    path.add_module("RootOutput", outputFileName=output_file + ".root")
    basf2.process(path, args.number)
