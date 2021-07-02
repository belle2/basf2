#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from ROOT import Belle2

import pathlib
import argparse
parser = argparse.ArgumentParser()
parser.add_argument("input_data", help=("The path to the input data directory you want to use."
                                        "It must contain a CollectorOutput.root file."))
# If the iteration is 0 then a localdb is created but no DBObjects are accessed inside
# the algorithm
# If the iteration is >0 then the previous DBObjects are read and then new ones are created
parser.add_argument("iteration",
                    type=int,
                    help="The iteration number used by the algorithm.")

parser.add_argument("--resetdb-after-execute",
                    action="store_true",
                    dest="reset",
                    help=("Should we reset the database chain each time we execute the algorithm. "
                          "Or only set the database once before all executions (default)."))


args = parser.parse_args()

b2.set_log_level(b2.LogLevel.DEBUG)
# View the framework debugging
b2.set_debug_level(100)
# For just the Algorithm debugging
# set_debug_level(29)

algo = Belle2.TestDBAccessAlgorithm()

# Can use a Python list of input files/wildcards. It will resolve the existing files
inputFileNames = [pathlib.Path(args.input_data, "CollectorOutput.root").absolute().as_posix()]
algo.setInputFileNames(inputFileNames)

if not args.reset:
    # The local db that we will both write to and read from
    b2.conditions.prepend_testing_payloads("localdb/database.txt")

# We iterate over some runs and execute separately.
# This means that we repeatedly access the DB interface after committing
for i in range(1, 5):
    if args.reset:
        # We're doing this here to test what happens when resetting in a single Python process
        # The local db that we will both write to and read from
        b2.conditions.prepend_testing_payloads("localdb/database.txt")
    print("Result of calibration =", algo.execute([(0, i)], args.iteration))
    algo.commit()
