#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
#
# Test script to produce SVD DQM plots from raw data
#
# Usage: basf2 runSVDDQM_rawData.py -i "<path/to/file.root>"
#
# the rootfiles MUST contain RAW DATA
# you need the dqm (and daq) packages
#
#############################################################

import basf2 as b2
from basf2 import conditions as b2conditions
import tracking as trk
import rawdata as raw
import daqdqm.commondqm as cdqm

# needed for some temporary issues with BKLMDisplacement payload
b2conditions.override_globaltags()
b2conditions.globaltags = ['data_reprocessing_prompt', 'online_bucket10', 'Reco_master_patch_rel5', 'online']
# b2conditions.globaltags = ['online']

# main main
main = b2.create_path()

# RAW
files = ['/group/belle2/dataprod/Data/PromptSkim/e0012/4S/r02874/skim/hadron/raw/sub00/physics.*.root']

# no SVD raw data in this file:
# files = ['/gpfs/group/belle2/fast_lane/inbox/debug.0012.01304.HLT3.f00010.root']


# read input rootfile
# -> can be overwritten with the -i option
main.add_module("RootInput", inputFileNames=files)

# register the HistoManager and specify output file
main.add_module("HistoManager", histoFileName="SVDRawDQMOutput.root")

# nee to know SVD geometry to create histograms
main.add_module('Gearbox')
main.add_module('Geometry')

# unpack
raw.add_unpackers(main, components=['PXD', 'SVD', 'CDC', 'TRG'])
# uncomment to unpack simulating the 3-samples DAQ mode!
# raw.add_unpackers(main, components=['PXD', 'CDC', 'TRG'])
# svd.add_svd_unpacker_simulate3sampleAcquisitionMode(main)

trk.add_tracking_reconstruction(main, components=['SVD', 'CDC'])

# add DQM
cdqm.add_common_dqm(main, components=['SVD'])

# == Show progress
main.add_module('Progress')

b2.print_path(main)

# Process events
b2.process(main)

print(b2.statistics)
