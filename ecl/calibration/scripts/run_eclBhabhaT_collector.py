#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# -----------------------------------------------------------
# BASF2 (Belle Analysis Framework 2)
# Copyright(C) 2018 Belle II Collaboration
#
# Author: The Belle II Collaboration
# Contributors: Mikhail Remnev
#
# This software is provided "as is" without any warranty.
# -----------------------------------------------------------

# --------------------------------------------------------------------------
# BASF2 script for the first (out of two) step of time shift calibration.
# using bhabha events.
# --------------------------------------------------------------------------
#
# There are two ways you can use it:
# 1. Provide parameters from command line:
#   basf2 EclBhabhaTCollector.py -i "/path/to/input/files/*.root" -o collector_output.root
#
# 2. Set parameters directly in steering file.
#   Change INPUT_LIST and OUTPUT variables.
#   (Multiple files can be easily added with glob.glob("/path/to/your/files/*.root"))
#   And then call
#   basf2 EclBhabhaTCollector.py

from basf2 import *
from ROOT import Belle2
import glob
import sys
import tracking
import rawdata
import reconstruction

env = Belle2.Environment.Instance()

###################################################################
# == Input/Output Parameters
###################################################################

# == List of input files
# NOTE: It is going to be sorted (alphabetic and length sorting, files with
#       shortest names are first)
INPUT_LIST = []
# = Adding data for exp3 run 529 as an example.
# = Processed data
INPUT_LIST += glob.glob("/hsm/belle2/bdata/Data/release-01-02-09/DB00000410/prod00000004/e0003/4S/r00529/all/dst/sub00/*.root")
# = Raw data
# INPUT_LIST += glob.glob("/ghi/fs01/belle2/bdata/Data/Raw/e0003/r00529/sub00/*.root")

# == Output file
OUTPUT = "eclBhabhaTCollector.root"

########################
# Input/output overrides.

# Override input if "-i file.root" argument was sent to basf2.
input_arg = env.getInputFilesOverride()
if len(input_arg) > 0:
    INPUT_LIST = [str(x) for x in input_arg]
# Sort list of input files.
INPUT_LIST.sort(key=lambda item: (len(item), item))

# Override output if "-o file.root" argument was sent to basf2.
output_arg = env.getOutputFileOverride()
if len(output_arg) > 0:
    OUTPUT = output_arg

###################################################################
# == Collector parameters
###################################################################

# Low energy cut (GeV)
MIN_EN = 0.04
# High energy cut (GeV)
MAX_EN = 10.0
# High energy cut for max total energy in event (GeV)
MAX_TOTAL_EN = 15.0
# Events with abs(time_ECL-time_CDC) > TIME_ABS_MAX are excluded
TIME_ABS_MAX = 250
# Events with ECLDigits.getEntries() > NENTRIES_MAX are excluded
# (currently unused -- set to arbitrarily high value)
NENTRIES_MAX = 9999

# If true, output file will contain TTree "tree" with detailed
# event information.
SAVE_TREE = False
# If true, fill histogram with weight min(energy**2, 1 GeV)
WEIGHTED_HIST = True

# First ECL CellId to calibrate
MIN_CRYSTAL = 1
# Last ECL CellId to calibrate
MAX_CRYSTAL = 8736

###################################################################

components = ['CDC', 'ECL']

# == Create path
main = create_path()

add_unpackers = False

# == SeqRoot/Root input
if INPUT_LIST[0].endswith('sroot'):
    main.add_module('SeqRootInput', inputFileNames=INPUT_LIST)
    add_unpackers = True
else:
    main.add_module('RootInput', inputFileNames=INPUT_LIST)

main.add_module("HistoManager", histoFileName=OUTPUT)

if 'Raw' in INPUT_LIST[0]:
    add_unpackers = True

main.add_module('Gearbox')

if add_unpackers:
    rawdata.add_unpackers(main, components=components)

    # = Get Tracks, RecoTracks, ECLClusters, add relations between them.
    tracking.add_tracking_reconstruction(main, components=components)
    reconstruction.add_ext_module(main, components)
    reconstruction.add_ecl_modules(main, components)
    reconstruction.add_ecl_track_matcher_module(main, components)
else:
    main.add_module('Geometry', useDB=True, components=components)

# == Generate time calibration matrix from ECLDigit
main.add_module('ECLBhabhaTCollector', minEn=MIN_EN,
                maxEn=MAX_EN, maxTotalEn=MAX_TOTAL_EN, timeAbsMax=TIME_ABS_MAX,
                minCrystal=MIN_CRYSTAL, maxCrystal=MAX_CRYSTAL,
                nentriesMax=NENTRIES_MAX, saveTree=SAVE_TREE,
                weightedHist=WEIGHTED_HIST)

# == Show progress
main.add_module('Progress')

set_log_level(LogLevel.INFO)

# == Configure database
reset_database()
use_database_chain()
use_central_database("development")
use_local_database("localdb/database.txt")

# == Process events
# set_nprocesses(8) # Run in multiple threads.
process(main)

print(statistics)
