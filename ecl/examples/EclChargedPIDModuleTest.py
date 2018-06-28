#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# This steering file tests the eclChargedPID/ECLChargedPIDModule.
# It must run on DST files, or reduced DSTs containing
# ECLShowers, Tracks and all relevant relational containers:
#
# 'Tracks',
# 'TrackFitResults',
# 'ECLClusters',
# 'ECLShowers',
# 'TracksToECLClusters',
# 'TracksToECLShowers',
# 'ECLClustersToECLShowers',
# 'ECLConnectedRegions',
# 'ECLPidLikelihoods',
# 'PIDLikelihoods',
# 'TracksToECLPidLikelihoods',
# 'TracksToPIDLikelihoods',
#
# Author: Marco Milesi (marco.milesi@unimelb.edu.au)
# Year: 2018
#
# Usage:
#
# basf2 ecl/examples/EclChargedPIDModuleTest.py /path/to/input/DST/file.root -n N
#
########################################################

from basf2 import *
from ROOT import Belle2
from modularAnalysis import *
import os
import sys

# Choose the DB
use_local_database("localdb/database.txt")

# Register necessary modules to this path.
main_path = create_path()

if len(sys.argv) != 2:
    os.sys.exit("ERROR: input file not given. Specify it as an argument")

# Add module to read input *DST file.
inputfile = sys.argv[1]
simpleinput = register_module('RootInput')
simpleinput.param('inputFileNames', inputfile)
main_path.add_module(simpleinput)

# Add eclChargedPID/ECLChargedPIDModule.
eclid = register_module('ECLChargedPID')
main_path.add_module(eclid)
eclid.logging.log_level = LogLevel.DEBUG
eclid.logging.debug_level = 20

# Print the datamodel objects in the store.
printcolls = register_module('PrintCollections')
main_path.add_module(printcolls)

# This line allows to study events one-by-one, interactively
# main_path.add_module('Interactive')

process(main_path)
print(statistics)
