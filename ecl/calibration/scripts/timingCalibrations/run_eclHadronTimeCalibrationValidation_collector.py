#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#
# This script executes the hadron timing calibration collector,
# which reads in the hadron events to perform the hadron
# validation, where the final validation fits is performed
# by the algorithm.
# This script is run directly with basf2.
#
# There are two ways you can use this script:
# 1. Provide parameters from command line:
#   basf2 run_eclHadronTimeCalibrationValidation_collector.py -i "/path/to/input/files/*.root" -o collector_output.root
#
# 2. Set parameters directly in steering file.
#   Change INPUT_LIST and OUTPUT variables.
#   (Multiple files can be easily added with glob.glob("/path/to/your/files/*.root"))
#   And then call
#   basf2 run_eclHadronTimeCalibrationValidation_collector.py


from basf2 import *
from ROOT import Belle2
import glob
import sys
import tracking
import rawdata
import reconstruction

from basf2 import conditions as b2conditions
from reconstruction import prepare_user_cdst_analysis


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
OUTPUT = "eclHadronTimeCalibrationValidationCollector.root"

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
TIME_ABS_MAX = 70

# If true, output file will contain TTree "tree" with detailed
# event information.
SAVE_TREE = True

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

prepare_user_cdst_analysis(main)  # for new 2020 cdst format

# == Generate time calibration matrix from ECLDigit
ECLtimeCalibValidationCollectorInfo = main.add_module('eclHadronTimeCalibrationValidationCollector', timeAbsMax=TIME_ABS_MAX,
                                                      saveTree=SAVE_TREE)

# ECLtimeCalibValidationCollectorInfo.set_log_level(LogLevel.DEBUG)
ECLtimeCalibValidationCollectorInfo.set_log_level(LogLevel.INFO)
ECLtimeCalibValidationCollectorInfo.set_debug_level(36)


# == Show progress
main.add_module('Progress')

# set_log_level(LogLevel.DEBUG)
set_log_level(LogLevel.INFO)
set_debug_level(100)

# == Configure database
# reset_database()
# use_database_chain()

b2conditions.reset()
b2conditions.override_globaltags()

B2INFO("Adding Local Database {} to head of chain of local databases.")
b2conditions.prepend_testing_payloads("localdb/database.txt")
B2INFO("Using Global Tag {}")
b2conditions.prepend_globaltag("ECL_testingNewPayload_RefCrystalPerCrate")
b2conditions.prepend_globaltag("master_2020-05-13")
b2conditions.prepend_globaltag("online_proc11")
b2conditions.prepend_globaltag("data_reprocessing_proc11")
b2conditions.prepend_globaltag("Reco_master_patch_rel5")


# == Process events
# process(main, max_event=350000)  # reasonable stats for one crate
# process(main, max_event=600000)  # reasonable stats for crystal calibs for proc10
# process(main, max_event=3000)    # reasonable stats and speed for a quick test
process(main, max_event=30)        # fast test
# process(main)                      # process all events

print(statistics)
