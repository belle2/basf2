#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -----------------------------------------------------------
# BASF2 (Belle Analysis Framework 2)
# Copyright(C) 2018  Belle II Collaboration
#
# Author: The Belle II Collaboration
# Contributors: Christopher Hearty
#
# This software is provided "as is" without any warranty.
# -----------------------------------------------------------

# Usage: basf2 run_collector.py inputFile.sroot outputFile.root
# Or: basf2 run_eclCosmicE_collector.py inputFile.root outputFile.root

# Run just the collector part of the eclCosmicE calibration, which calibrates the single crystal
# energy response using cosmic rays.
# Input file should be cosmic data in root or sroot format, including ecl digits

# Output histograms are written to specified output file.
# run_eclCosmicE_algorithm.py is then used to perform calibration using these histograms, or to simply copy
# them to an output file.

import os
import sys
from basf2 import *
from ROOT import Belle2
from rawdata import add_unpackers

main = create_path()

# input file, root or sequential root
narg = len(sys.argv)
inputfile = "/hsm/belle2/bdata/users/kuzmin/05399/cosmic.0001.05399.HLT1.f00000.sroot"
if(narg >= 2):
    inputfile = sys.argv[1]
    if inputfile.endswith(".sroot"):
        main.add_module('SeqRootInput', inputFileName=inputfile)
    elif inputfile.endswith(".root"):
        main.add_module('RootInput', inputFileName=inputfile)
    else:
        print("Wrong input file extention")
        sys.exit()
else:
    main.add_module('SeqRootInput', inputFileName=inputfile)

# output file
outputName = "eclCosmicECollectorOutput.root"
if(narg >= 3):
    outputName = sys.argv[2]
main.add_module("HistoManager", histoFileName=outputName)

main.add_module('Progress')

# ECL unpackers
add_unpackers(main, components='ECL')

set_log_level(LogLevel.INFO)

# CAF collector code
eclCosmicE = register_module('eclCosmicECollector', logLevel=LogLevel.DEBUG, debugLevel=9)
eclCosmicE.param('minCrysE', 0.01)
eclCosmicE.param('mockupL1', False)
eclCosmicE.param('trigThreshold', 0.1)
main.add_module(eclCosmicE)

# It is possible to force the job to use the specified global tag.
# Default localdb is the subdirectory of current working directory, but can be overwritten
reset_database()
use_database_chain()
use_central_database("development")
use_local_database("localdb/database.txt")

process(main)

print(statistics)
