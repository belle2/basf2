#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Check T0 calibration with Bhabha (or dimuon) events. Results in a root file.
#
# usage: basf2 cdst_checkT0calibration.py -i <cdst_file.root>
# ---------------------------------------------------------------------------------------

import basf2 as b2
from reconstruction import prepare_user_cdst_analysis

# global tags
# ******************************************************************************************************************
# note: The patching global tags and their order are bucket number and basf2 version dependent.
#       Given below is what is needed for cdst files of bucket 16 calibration and January-2023 development version.
# ******************************************************************************************************************
b2.conditions.override_globaltags()
b2.conditions.append_globaltag('patch_main_release-07_noTOP')
b2.conditions.append_globaltag('data_reprocessing_proc13')  # experiments 7 - 18
# b2.conditions.append_globaltag('data_reprocessing_prompt')  # experiments 20 - 26
b2.conditions.append_globaltag('online')

# Create path
main = b2.create_path()

# Input: cDST file(s) of Bhabha skim, use -i option
# files of bucket 16 can be found on KEKCC in /gpfs/group/belle2/dataprod/Data/PromptReco/bucket16_calib/
roinput = b2.register_module('RootInput')
main.add_module(roinput)

# run unpackers and post-tracking reconstruction
prepare_user_cdst_analysis(main)

# Calibration checker: for dimuon sample replace 'bhabha' with 'dimuon'
calibrator = b2.register_module('TOPChannelT0Calibrator')
calibrator.param('sample', 'bhabha')
calibrator.param('outputFileName', 'checkT0cal_r*.root')
main.add_module(calibrator)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
