#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
#                                                                        #
# This script executes the bhabha timing calibration collector,          #
# which reads in the bhabha events to perform the crystal                #
# time calibration and the crate time calibration, where                 #
# the final calibration step is performed by the algorithm.              #
# This script is run directly with basf2.                                #
##########################################################################


# There are two ways you can use this script:
# 1. Provide parameters from command line:
#   basf2 run_eclBhabhaT_collector.py -i "/path/to/input/files/*.root" -o collector_output.root
#
# 2. Set parameters directly in steering file.
#   Change INPUT_LIST and OUTPUT variables.
#   (Multiple files can be easily added with glob.glob("/path/to/your/files/*.root"))
#   And then call
#   basf2 run_eclBhabhaT_collector.py

import basf2 as b2
from ROOT import Belle2
import glob
import sys
import tracking
import rawdata
import reconstruction

from basf2 import conditions as b2conditions
from reconstruction import prepare_cdst_analysis


env = Belle2.Environment.Instance()

###################################################################
# == Input/Output Parameters
###################################################################

# == List of input files
# NOTE: It is going to be sorted (alphabetic and length sorting, files with
#       shortest names are first)
INPUT_LIST = []
# = Processed data
INPUT_LIST += glob.glob("oneTestFile/*.root")

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


# Events with abs(time_ECL-time_CDC) > TIME_ABS_MAX are excluded
TIME_ABS_MAX = 250

# If true, output file will contain TTree "tree" with detailed
# event information.
SAVE_TREE = False

# First ECL CellId to calibrate
MIN_CRYSTAL = 1
# Last ECL CellId to calibrate
MAX_CRYSTAL = 8736

# Bias of CDC event t0 in bhabha vs hadronic events (in ns)
CDC_T0_BIAS_CORRECTION_OFFSET = 0   # default is 0ns

###################################################################

components = ['CDC', 'ECL']

# == Create path
main = b2.create_path()

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

prepare_cdst_analysis(main)  # for new 2020 cdst format

# == Generate time calibration matrix from ECLDigit
ECLBhabhaTCollectorInfo = main.add_module('ECLBhabhaTCollector', timeAbsMax=TIME_ABS_MAX,
                                          minCrystal=MIN_CRYSTAL, maxCrystal=MAX_CRYSTAL,
                                          saveTree=SAVE_TREE,
                                          hadronEventT0_TO_bhabhaEventT0_correction=CDC_T0_BIAS_CORRECTION_OFFSET)

ECLBhabhaTCollectorInfo.set_log_level(b2.LogLevel.INFO)  # OR: b2.LogLevel.DEBUG
ECLBhabhaTCollectorInfo.set_debug_level(36)


# == Show progress
main.add_module('Progress')

# set_log_level(LogLevel.DEBUG)
b2.set_log_level(b2.LogLevel.INFO)
b2.set_debug_level(100)

# == Configure database
# reset_database()
# use_database_chain()

b2conditions.reset()
b2conditions.override_globaltags()

# These global tags will have to be updated for your files
# Highest priority last
b2.B2INFO("Adding Local Database {} to head of chain of local databases.")
b2conditions.prepend_testing_payloads("localdb/database.txt")
b2.B2INFO("Using Global Tag {}")
b2conditions.prepend_globaltag("ECL_testingNewPayload_RefCrystalPerCrate")
b2conditions.prepend_globaltag("master_2020-05-13")
b2conditions.prepend_globaltag("online_proc11")
b2conditions.prepend_globaltag("data_reprocessing_proc11")
b2conditions.prepend_globaltag("Reco_master_patch_rel5")

# == Process events
# process(main, max_event=350000)  # reasonable stats for one crate
# process(main, max_event=600000)  # reasonable stats for crystal calibs for proc10
# process(main, max_event=3000)    # reasonable stats and speed for a quick test
b2.process(main)                   # process all events

print(b2.statistics)
