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

# Usage: basf2 -i inputFile.root run_eclCosmicE_collector.py [outputFile.root globaltag]

# Run just the collector part of the eclCosmicE calibration, which calibrates the single crystal
# energy response using cosmic rays.
# Input file should be cosmic data in root format, including ecl digits

# Output histograms are written to specified output file.
# run_eclCosmicE_algorithm.py is then used to perform calibration using these histograms, or to simply copy
# them to an output file.

import sys
import basf2 as b2
from rawdata import add_unpackers


# input file
main = b2.create_path()
inputfile = '/ghi/fs01/belle2/bdata/Data/Raw/e0007/r01112/sub00/*.root'
main.add_module('RootInput', inputFileNames=[inputfile])

# output file
outputName = "eclCosmicECollectorOutput.root"
narg = len(sys.argv)
if(narg >= 2):
    outputName = sys.argv[1]
main.add_module("HistoManager", histoFileName=outputName)

main.add_module('Progress')

# ECL unpackers
add_unpackers(main, components='ECL')

b2.set_log_level(b2.LogLevel.INFO)

# CAF collector code
eclCosmicE = b2.register_module('eclCosmicECollector', logLevel=b2.LogLevel.DEBUG, debugLevel=9)
eclCosmicE.param('minCrysE', 0.01)
eclCosmicE.param('mockupL1', False)
eclCosmicE.param('trigThreshold', 0.1)
main.add_module(eclCosmicE)

# It is possible to force the job to use the specified global tag.
# Default localdb is the subdirectory of current working directory
centralGT = "data_reprocessing_prompt_bucket3b"
if(narg >= 3):
    centralGT = sys.argv[2]
b2.reset_database()
b2.use_database_chain()
b2.use_central_database(centralGT)
b2.use_local_database("localdb/database.txt")

b2.process(main)

print(b2.statistics)
