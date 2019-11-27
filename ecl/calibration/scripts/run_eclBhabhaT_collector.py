#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# -----------------------------------------------------------
# BASF2 (Belle Analysis Framework 2)
# Copyright(C) 2018 Belle II Collaboration
#
# Author: The Belle II Collaboration
# Contributors: Ewan Hill
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
# = Processed data
INPUT_LIST += glob.glob("oneTestFile/*.root")
# INPUT_LIST += glob.glob("threeTestFiles/*.root")
# INPUT_LIST += glob.glob("specificIndividualRuns/run_784/*.root")
# INPUT_LIST += glob.glob("specificIndividualRuns/run_2727/*.root")
# INPUT_LIST += glob.glob("testingProd7/cdst.*.root")
# INPUT_LIST +=
# glob.glob("testFiles/*.root")
# # EWAN multiple files

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


# Events with abs(time_ECL-time_CDC) > TIME_ABS_MAX are excluded
TIME_ABS_MAX = 250

# If true, output file will contain TTree "tree" with detailed
# event information.
# SAVE_TREE = True
SAVE_TREE = False

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
ECLBhabhaTCollectorInfo = main.add_module('ECLBhabhaTCollector', timeAbsMax=TIME_ABS_MAX,
                                          minCrystal=MIN_CRYSTAL, maxCrystal=MAX_CRYSTAL,
                                          saveTree=SAVE_TREE)

# ECLBhabhaTCollectorInfo.set_log_level(LogLevel.DEBUG)
ECLBhabhaTCollectorInfo.set_log_level(LogLevel.INFO)
ECLBhabhaTCollectorInfo.set_debug_level(36)


# == Show progress
main.add_module('Progress')

# set_log_level(LogLevel.DEBUG)
set_log_level(LogLevel.INFO)
set_debug_level(100)

# == Configure database
reset_database()
use_database_chain()
# Required for reading the offline calibration development
# version of the database, which contains updates to the
# ECLCrystalElectronicsTime
# use_central_database("Calibration_Offline_Development")
# use_central_database("data_reprocessing_prompt_bucket6_alignment")
# use_central_database("staging_data_reprocessing")   # this did not work !


# These two below work together(?) for exp7 but failed for exp3
# use_central_database("data_reprocessing_prompt_bucket6")
# use_central_database("data_reprocessing_proc9_cdst_production")


# 3 GT required for proc9 as stated by Umberto on the jira ticket
# use_central_database("data_reprocessing_proc8")  #Fallback for exp  0 -> 3
# use_central_database("data_reprocessing_prompt_bucket6")  #Fallback for exp  5 -> 8
# use_central_database("data_reprocessing_proc9_cdst_production")  #New calibrations
# use_central_database("ECL_time_calibrations_proc9")  #validation
# use_central_database("data_reprocessing_proc9")  #New calibrations

# use_central_database("data_reprocessing_prompt_bucket7_calcdstprod")  # required for calibrating bucket 7


# use_central_database("data_reprocessing_prompt_bucket7")  # bucket 7 GT used to make valiation cdst files for bucket 7


# 2 GT required for making proc 10
use_central_database("data_reprocessing_proc10")
use_central_database("data_reprocessing_prompt_rel4_patchb")


# use_central_database("data_reprocessing_prod6")   # Use this to keep a stable tag: from Chris
#   Also see https://confluence.desy.de/x/6ThYBQ
# CHECK database with:
#      b2conditionsdb iov Calibration_Offline_Development  -f '^ECLCrateTimeOffset' -r
# and
#      b2conditionsdb iov Calibration_Offline_Development  -f '^ECLCrystalTimeOffset' -r


# CAREFUL ABOUT USING LOCAL DATABASE OF PREVIOUS CRYSTAL TIME CORRECTIONS WHEN CALCULATING
#    CRATE TIME CORRECTIONS VS CENTRAL DB VALUES.  CRYSTAL TIME CORRECTIONS SHOULD
#    USE THE LOCAL DB CRATE TIME CORRECTIONS.
use_local_database("localdb/database.txt")


# == Process events
# process(main, max_event=350000)  # reasonable stats for one crate
# process(main, max_event=600000) # reasonable stats for crystal calibs for proc10
process(main, max_event=3000)
# process(main)

print(statistics)
