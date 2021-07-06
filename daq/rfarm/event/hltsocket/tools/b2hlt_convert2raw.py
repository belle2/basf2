#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from softwaretrigger.constants import HLT_INPUT_OBJECTS

import os
from argparse import ArgumentParser


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument("input_file", help="Input file name")
    parser.add_argument("output_file", help="Output file name")
    parser.add_argument("--number", help="How many events should be converted", default=0, type=int)

    args = parser.parse_args()
    input_file = args.input_file
    output_file = args.output_file

    path = basf2.Path()
    extension = os.path.splitext(input_file)[-1]
    if extension == ".root":
        path.add_module("RootInput", inputFileName=input_file, branchNames=HLT_INPUT_OBJECTS)
    elif extension == ".sroot":
        path.add_module("SeqRootInput", inputFileName=input_file)
    else:
        raise AttributeError(f"Do not understand file format of {input_file}")

    path.add_module("Root2Raw", outputFileName=output_file)
    basf2.process(path, args.number)
