#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
#
# Test script to produce SVD DQM plots from raw data
#
# Author:  Giulia Casarosa
#
# Usage: basf2 runSVDDQM_rawData.py -i "<path/to/file.root>"
#
# the rootfiles MUST contain RAW DATA
# you need the dqm (and daq) packages
#
# 2020 Belle II Collaboration
#############################################################

import os
from basf2 import *
from basf2 import conditions as b2conditions
import tracking as trk
import rawdata as raw
import daqdqm.commondqm as cdqm

# needed for some temporary issues with BKLMDisplacement payload
b2conditions.override_globaltags()
b2conditions.globaltags = ['klm_alignment_testing', 'online']
# b2conditions.globaltags = ['online']

# main main
main = create_path()

# RAW
files = [' /group/belle2/dataprod/Data/Raw/e0010/r04295/sub00/physics.0010.04295.HLT1*.root']
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

# unpack and reconstruct
raw.add_unpackers(main, components=['PXD', 'SVD', 'CDC'])
trk.add_tracking_reconstruction(main, components=['SVD', 'CDC'])

# add DQM
cdqm.add_common_dqm(main, components=['SVD'])

# == Show progress
main.add_module('Progress')

print_path(main)

# Process events
process(main)

print(statistics)
